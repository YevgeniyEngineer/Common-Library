#include <atomic>
#include <cstdint>
#include <vector>

namespace common_library
{
// Single Producer Single Consumer Queue
template <typename T> class SPSCQueue final
{
  private:
    std::vector<T> buffer_;
    std::atomic_size_t head_, tail_;
    const std::size_t capacity_;

  public:
    explicit SPSCQueue(std::size_t size) : buffer_(size), head_(0), tail_(0), capacity_(size)
    {
    }

    SPSCQueue() = delete;
    SPSCQueue(const SPSCQueue &other) = delete;
    SPSCQueue &operator=(const SPSCQueue &other) = delete;

    bool push(const T &value) noexcept
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

    bool pop(T &value) noexcept
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
    std::size_t increment(std::size_t idx) const noexcept
    {
        return (idx + 1) % capacity_;
    }
};
} // namespace common_library