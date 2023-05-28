#ifndef COMMON_LIBRARY_CONCURRENCY_BOUNDED_SHARED_QUEUE
#define COMMON_LIBRARY_CONCURRENCY_BOUNDED_SHARED_QUEUE

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

namespace common_library::concurrency
{
class BoundedSharedQueueShutdownException : public std::runtime_error
{
  public:
    explicit BoundedSharedQueueShutdownException(const std::string &message) : std::runtime_error(message)
    {
    }

    explicit BoundedSharedQueueShutdownException(const char *message) : std::runtime_error(message)
    {
    }

    BoundedSharedQueueShutdownException() : std::runtime_error("The queue is shutting down.")
    {
    }
};

template <typename T> class BoundedSharedQueue
{
  private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable data_available_;
    std::condition_variable space_available_;
    std::size_t max_size_;
    std::atomic_bool shutdown_;

  public:
    BoundedSharedQueue(std::size_t max_size = std::numeric_limits<std::size_t>::max())
        : max_size_(max_size), shutdown_(false)
    {
    }

    BoundedSharedQueue &operator=(const BoundedSharedQueue &other) = delete;
    BoundedSharedQueue &operator=(BoundedSharedQueue &&other) noexcept = delete;
    BoundedSharedQueue(const BoundedSharedQueue &other) = delete;
    BoundedSharedQueue(BoundedSharedQueue &&other) noexcept = delete;

    ~BoundedSharedQueue()
    {
        const std::lock_guard<std::mutex> lock{mutex_};

        shutdown_.store(true, std::memory_order_release);
        data_available_.notify_all();
        space_available_.notify_all();
    }

    [[nodiscard]] bool tryPop(T &item)
    {
        if (shutdown_.load(std::memory_order_relaxed))
        {
            return false;
        }

        const std::unique_lock<std::mutex> lock{mutex_};

        if (queue_.empty())
        {
            return false;
        }
        else
        {
            item = std::move(queue_.front());
            queue_.pop();
            space_available_.notify_one();
            return true;
        }
    }

    [[nodiscard]] bool tryPush(const T &item)
    {
        if (shutdown_.load(std::memory_order_relaxed))
        {
            return false;
        }

        const std::unique_lock<std::mutex> lock{mutex_};

        if ((queue_.size() < max_size_))
        {
            queue_.push(item);
            data_available_.notify_one();
            return true;
        }
        else
        {
            return false;
        }
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock{mutex_};

        data_available_.wait(lock, [this]() { return !queue_.empty() || shutdown_.load(std::memory_order_relaxed); });

        if (shutdown_.load(std::memory_order_relaxed))
        {
            throw BoundedSharedQueueShutdownException("BoundedSharedQueue shutting down");
        }

        auto item = std::move(queue_.front());
        queue_.pop();
        space_available_.notify_one();
        return item;
    }

    void push(const T &item)
    {
        std::unique_lock<std::mutex> lock{mutex_};

        space_available_.wait(
            lock, [this]() { return (queue_.size() < max_size_) || shutdown_.load(std::memory_order_relaxed); });

        if (shutdown_.load(std::memory_order_relaxed))
        {
            throw BoundedSharedQueueShutdownException("BoundedSharedQueue is shutting down");
        }

        queue_.push(item);
        data_available_.notify_one();
    }

    [[nodiscard]] std::size_t maxSize() const noexcept
    {
        return max_size_;
    }

    [[nodiscard]] bool empty() const
    {
        const std::lock_guard<std::mutex> lock{mutex_};

        return queue_.empty();
    }

    [[nodiscard]] bool full() const
    {
        const std::lock_guard<std::mutex> lock{mutex_};

        return (queue_.size() == max_size_);
    }

    [[nodiscard]] std::size_t size() const
    {
        const std::lock_guard<std::mutex> lock{mutex_};

        return queue_.size();
    }
};
} // namespace common_library::concurrency

#endif // COMMON_LIBRARY_CONCURRENCY_BOUNDED_SHARED_QUEUE