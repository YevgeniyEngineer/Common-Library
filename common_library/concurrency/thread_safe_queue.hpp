#ifndef COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_QUEUE
#define COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_QUEUE

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace common_library::concurrency
{
// Thread Safe Queue
template <typename T> class ThreadSafeQueue final
{
  private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic_bool destructing_{false};

    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;

  public:
    ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;

    [[nodiscard]] static ThreadSafeQueue<T> &getInstance()
    {
        static ThreadSafeQueue<T> instance;
        return instance;
    }

    void push(T value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (destructing_)
        {
            return;
        }
        queue_.push(std::move(value));
        cv_.notify_one();
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || destructing_; });
        if (destructing_)
        {
            return {};
        }
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lock{mutex_};
        if (destructing_)
        {
            return true;
        }
        return queue_.empty();
    }

    ~ThreadSafeQueue()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        destructing_.store(true);
        cv_.notify_all();
    }
};
} // namespace common_library::concurrency

#endif // COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_QUEUE