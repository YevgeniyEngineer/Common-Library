#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <mutex>
#include <queue>

namespace common_library
{
// Bounded Shared Queue
template <typename T> class BSQueue
{
  private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable data_available_;
    std::condition_variable space_available_;
    std::size_t max_size_;
    std::atomic_bool shutdown_;

  public:
    explicit BSQueue(std::size_t max_size = std::numeric_limits<std::size_t>::max())
        : max_size_(max_size), shutdown_(false)
    {
    }

    BSQueue &operator=(const BSQueue &other) = delete;
    BSQueue &operator=(BSQueue &&other) noexcept = delete;
    BSQueue(const BSQueue &other) = delete;
    BSQueue(BSQueue &&other) noexcept = delete;

    ~BSQueue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        shutdown_.store(true, std::memory_order_release);
        data_available_.notify_all();
        space_available_.notify_all();
    }

    [[nodiscard]] bool tryPop(T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty() || shutdown_.load(std::memory_order_relaxed))
        {
            return false;
        }
        else
        {
            item = queue_.front();
            queue_.pop();
            space_available_.notify_one();
            return true;
        }
    }

    [[nodiscard]] bool tryPush(const T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() < max_size_ && !shutdown_.load(std::memory_order_relaxed))
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
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty() && !shutdown_.load(std::memory_order_relaxed))
        {
            data_available_.wait(lock);
        }

        if (shutdown_.load(std::memory_order_relaxed))
        {
            throw std::runtime_error("BSQueue shutting down");
        }

        auto item = queue_.front();
        queue_.pop();
        space_available_.notify_one();
        return item;
    }

    void push(const T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.size() >= max_size_ && !shutdown_.load(std::memory_order_relaxed))
        {
            space_available_.wait(lock);
        }

        if (shutdown_.load(std::memory_order_relaxed))
        {
            throw std::runtime_error("BSQueue is shutting down");
        }

        queue_.push(item);
        data_available_.notify_one();
    }

    std::size_t maxSize() const
    {
        return max_size_;
    }
};
} // namespace common_library