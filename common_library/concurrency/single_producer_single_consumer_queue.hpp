#ifndef COMMON_LIBRARY_CONCURRENCY_SINGLE_PRODUCER_SINGLE_CONSUMER_QUEUE
#define COMMON_LIBRARY_CONCURRENCY_SINGLE_PRODUCER_SINGLE_CONSUMER_QUEUE

#include <atomic>
#include <cstdint>
#include <vector>

namespace common_library::concurrency
{
// Single Producer Single Consumer Queue
template <typename T> class SingleProducerSingleConsumerQueue final
{
  private:
    std::vector<T> buffer_;
    std::atomic_size_t head_, tail_;
    const std::size_t capacity_;

  public:
    explicit SingleProducerSingleConsumerQueue(std::size_t size) : buffer_(size), head_(0), tail_(0), capacity_(size)
    {
    }

    SingleProducerSingleConsumerQueue() = delete;
    SingleProducerSingleConsumerQueue(const SingleProducerSingleConsumerQueue &other) = delete;
    SingleProducerSingleConsumerQueue &operator=(const SingleProducerSingleConsumerQueue &other) = delete;

    [[nodiscard]] bool push(const T &value) noexcept
    {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = increment(current_tail);
        if (next_tail != head_.load(std::memory_order_acquire))
        {
            buffer_[current_tail] = value;
            tail_.store(next_tail, std::memory_order_release);
            return true;
        }
        return false;
    }

    [[nodiscard]] bool pop(T &value) noexcept
    {
        const auto current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_.load(std::memory_order_acquire))
        {
            return false;
        }
        value = buffer_[current_head];
        head_.store(increment(current_head), std::memory_order_release);
        return true;
    }

  private:
    [[nodiscard]] std::size_t increment(std::size_t idx) const noexcept
    {
        return (idx + 1) % capacity_;
    }
};
} // namespace common_library::concurrency

#endif // COMMON_LIBRARY_CONCURRENCY_SINGLE_PRODUCER_SINGLE_CONSUMER_QUEUE