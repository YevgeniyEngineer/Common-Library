#include <atomic>
#include <memory>

namespace common_library::concurrency
{
template <typename T> class LockFreeQueue
{
  private:
    // The Node structure holds the data and a pointer to the next Node in the queue.
    // Both are stored in an atomic to ensure thread-safety during modification and access.
    struct Node
    {
        explicit Node(const T &data) : data{data}, next{nullptr}
        {
        }

        Node() : data{}, next{nullptr}
        {
        }

        T data;
        std::atomic<Node *> next{nullptr};
    };

    // Head and tail pointers to Nodes in the queue are kept as atomic.
    // This ensures that multiple threads can reliably know the current state of the queue.
    std::atomic<Node *> head_{nullptr};
    std::atomic<Node *> tail_{nullptr};

  public:
    // The queue can't be copied or moved to prevent potential threading issues.
    LockFreeQueue(const LockFreeQueue &other) = delete;
    LockFreeQueue &operator=(const LockFreeQueue &other) = delete;
    LockFreeQueue(LockFreeQueue &&) noexcept = delete;
    LockFreeQueue &operator=(LockFreeQueue &&) noexcept = delete;

    // The constructor initializes an empty Node and sets both the head and tail to point to it.
    LockFreeQueue()
    {
        Node *new_node = new Node{};

        head_.store(new_node, std::memory_order_acquire);
        tail_.store(new_node, std::memory_order_acquire);
    }

    // Destructor deletes all Nodes in the queue.
    ~LockFreeQueue()
    {
        while (Node *old_head = head_.load(std::memory_order_acquire))
        {
            head_.store(old_head->next, std::memory_order_release);
            delete old_head;
        }
    }

    // Takes an element from the queue
    std::unique_ptr<T> pop()
    {
        // Save current head to old head.
        Node *old_head = head_.load(std::memory_order_acquire);

        // An infinite loop that tries to pop the head Node.
        // It only exits the loop when it successfully pops the Node.
        for (;;)
        {
            // Save the tail and next Node after the head.
            Node *tail = tail_.load(std::memory_order_acquire);
            Node *next = old_head->next.load(std::memory_order_acquire);

            // If there is no next Node, the queue is indeed empty and we return an empty pointer.
            if (next == nullptr)
            {
                return nullptr;
            }

            // If the head hasn't changed while saving tail and next.
            if (old_head == head_.load(std::memory_order_relaxed))
            {
                // If head and tail are the same, it means the queue might be empty.
                if (old_head == tail)
                {
                    // If there is a next Node, it means another thread has pushed a new Node to the queue,
                    // so we help it by moving the tail to the next Node.
                    tail_.compare_exchange_weak(old_head, next, std::memory_order_release, std::memory_order_relaxed);
                }
                // If the head and tail are not the same, it means there is at least one Node in the queue.
                // We create a unique_ptr to return the data.
                else
                {
                    T data = next->data;

                    // Try to move the head to the next Node.
                    // If successful, delete the old head Node and return the data.
                    if (head_.compare_exchange_strong(old_head, next, std::memory_order_release,
                                                      std::memory_order_relaxed))
                    {
                        delete old_head;
                        return std::make_unique<T>(data);
                    }
                }
            }
            else
            {
                old_head = head_.load(std::memory_order_acquire);
            }
        }
    }

    // Push data to the queue
    void push(const T &data)
    {
        // Create a new Node with the data.
        Node *new_node = new Node{data};

        // Infinite loop that attempts to push the new node.
        // It only exits when the new node has been successfully added to the queue.
        for (;;)
        {
            // Refresh the tail value in case it has been changed by another thread.
            Node *tail = tail_.load(std::memory_order_acquire);

            Node *expected{nullptr};

            // Try to change the next pointer of the tail Node to our new Node.
            // If this succeeds, it means no other thread has added another Node yet.
            // We have successfully added our new Node to the queue.
            if (tail->next.compare_exchange_weak(expected, new_node, std::memory_order_release,
                                                 std::memory_order_relaxed))
            {
                // Try to move the tail to our new Node.
                // If this fails, it doesn't matter because another thread will
                // do it when it adds another Node or when it pops.
                tail_.compare_exchange_weak(tail, new_node, std::memory_order_release, std::memory_order_relaxed);
                break;
            }
            else
            {
                // If we failed to set the next pointer of the tail, another thread has already
                // added a node, so we should try to help by advancing the tail
                tail_.compare_exchange_weak(tail, expected, std::memory_order_release, std::memory_order_relaxed);
            }
        }
    }

    // Check if the queue contains any elements
    bool empty()
    {
        std::atomic_thread_fence(std::memory_order_acquire); // Ensure we get the latest values
        return (head_.load(std::memory_order_relaxed)->next.load(std::memory_order_relaxed) == nullptr);
    }
};
} // namespace common_library::concurrency