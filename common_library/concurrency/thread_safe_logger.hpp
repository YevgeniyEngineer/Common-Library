#ifndef COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_LOGGER
#define COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_LOGGER

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <utility>

namespace common_library::concurrency
{
class ThreadSafeLogger final
{
  private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread worker_;
    std::atomic_bool exit_;
    std::uint32_t max_log_messages_within_buffer_;
    static std::atomic_bool max_log_messages_set_;

    static std::uint32_t &getMaxLogMessages()
    {
        static std::uint32_t max_log_messages = 10'000;
        return max_log_messages;
    }

    static void setMaxLogMessagesOnce(std::uint32_t max_log_messages)
    {
        if (!max_log_messages_set_.exchange(true))
        {
            ThreadSafeLogger::getMaxLogMessages() = max_log_messages;
        }
    }

    // Private constructor to prevent instantiation
    explicit ThreadSafeLogger(std::uint32_t max_log_messages_within_buffer)
        : max_log_messages_within_buffer_(max_log_messages_within_buffer), exit_(false),
          worker_(std::thread(&ThreadSafeLogger::processLogs, this))
    {
    }

    void processLogs()
    {
        while (!exit_.load())
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return !queue_.empty() || exit_.load(); });

            if (exit_.load() && queue_.empty())
            {
                break;
            }

            std::string log_message;
            if (!queue_.empty())
            {
                log_message = queue_.front();
                queue_.pop();
            }
            lock.unlock();

            if (!log_message.empty())
            {
                std::cout << log_message << std::endl;
            }
        }
    }

  protected:
    ~ThreadSafeLogger()
    {
        exit_ = true;
        condition_.notify_one();
        worker_.join();
    }

  public:
    // Delete the copy and move constructors and assignment operators
    ThreadSafeLogger(const ThreadSafeLogger &) = delete;
    ThreadSafeLogger(ThreadSafeLogger &&) noexcept = delete;
    ThreadSafeLogger &operator=(const ThreadSafeLogger &) = delete;
    ThreadSafeLogger &operator=(ThreadSafeLogger &&) noexcept = delete;

    static ThreadSafeLogger &getInstance(std::uint32_t max_log_messages)
    {
        ThreadSafeLogger::setMaxLogMessagesOnce(max_log_messages);
        static ThreadSafeLogger instance{getMaxLogMessages()};
        return instance;
    }

    template <typename... Args> void log(Args... args)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // If there's room for a new log message, add it
        if (queue_.size() < max_log_messages_within_buffer_)
        {
            std::ostringstream msg;
            (msg << ... << args);
            queue_.push(msg.str());
            condition_.notify_one();
        }
        else
        {
            // Drop the message or print a warning/error here if you wish
            std::cout << "Dropping message as the queue is full" << std::endl;
        }
    }
};

std::atomic_bool ThreadSafeLogger::max_log_messages_set_ = false;
} // namespace common_library::concurrency

#endif // COMMON_LIBRARY_CONCURRENCY_THREAD_SAFE_LOGGER