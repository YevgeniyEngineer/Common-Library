#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace common_library::containers
{
class StaticVectorIndexOutOfRangeException : public std::out_of_range
{
  public:
    StaticVectorIndexOutOfRangeException()
        : std::out_of_range("Attempting to access StackContainer at an invalid position.")
    {
    }

    ~StaticVectorIndexOutOfRangeException() = default;
};

class StaticVectorReachedMaximumCapacityException : public std::length_error
{
  public:
    StaticVectorReachedMaximumCapacityException()
        : std::length_error("StackContainer reached maximum capacity. No more elements are allowed.")
    {
    }
};

template <typename T, std::size_t N> class StaticContainer
{
    static_assert((N > 0), "Size of the StaticContainer must be greater than 0!");

  public:
    class Iterator
    {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::random_access_iterator_tag;

        explicit Iterator(T *data_ptr) noexcept : data_ptr_(data_ptr)
        {
        }

        Iterator() = delete;

        ~Iterator()
        {
            data_ptr_ = nullptr;
        }

        Iterator &operator++() noexcept
        {
            ++data_ptr_;
            return (*this);
        }

        Iterator operator++(int) noexcept
        {
            Iterator data_ref_tmp = (*this);
            ++(*this);
            return data_ref_tmp;
        }

        Iterator &operator--() noexcept
        {
            --data_ptr_;
            return (*this);
        }

        Iterator operator--(int) noexcept
        {
            Iterator data_ref_tmp = (*this);
            --(*this);
            return data_ref_tmp;
        }

        Iterator &operator+=(const difference_type data_ptr_diff) noexcept
        {
            data_ptr_ += data_ptr_diff;
            return (*this);
        }

        Iterator &operator-=(const difference_type data_ptr_diff) noexcept
        {
            data_ptr_ -= data_ptr_diff;
            return (*this);
        }

        difference_type operator-(const Iterator &other) noexcept
        {
            return (data_ptr_ - other.data_ptr_);
        }

        Iterator operator+(const difference_type data_ptr_diff) noexcept
        {
            return Iterator(data_ptr_ + data_ptr_diff);
        }

        Iterator operator-(const difference_type data_ptr_diff) noexcept
        {
            return Iterator(data_ptr_ - data_ptr_diff);
        }

        T &operator[](const difference_type data_ptr_diff) noexcept
        {
            return (*(data_ptr_ + data_ptr_diff));
        }

        const T &operator[](const difference_type data_ptr_diff) const noexcept
        {
            return (*(data_ptr_ + data_ptr_diff));
        }

        bool operator==(const Iterator &other) const noexcept
        {
            return (data_ptr_ == other.data_ptr_);
        }

        bool operator!=(const Iterator &other) const noexcept
        {
            return !((*this) == (other));
        }

        T &operator*() const noexcept
        {
            return (*data_ptr_);
        }

      private:
        T *data_ptr_;
    };

    // Default constructor
    StaticContainer() : data_{}, size_{0U}
    {
    }

    // Copy constructor
    StaticContainer(const StaticContainer &other) : size_{other.size_}
    {
        std::copy(other.data_, other.data_ + other.size_, data_);
    }

    // Copy assignment operator
    StaticContainer &operator=(const StaticContainer &other)
    {
        if (this != (&other))
        {
            size_ = other.size_;
            std::copy(other.data_, other.data_ + other.size_, data_);
        }
        return (*this);
    }

    // Move constructor
    StaticContainer(StaticContainer &&other) noexcept : size_{other.size_}
    {
        std::move(other.data_, other.data_ + other.size_, data_);
        other.size_ = 0;
    }

    // Move assignment operator
    StaticContainer &operator=(StaticContainer &&other) noexcept
    {
        if (this != (&other))
        {
            size_ = other.size_;
            std::move(other.data_, other.data_ + other.size_, data_);
            other.size_ = 0;
        }
        return (*this);
    }

    Iterator begin() noexcept
    {
        return Iterator(data_);
    }

    Iterator end() noexcept
    {
        return Iterator(data_ + size_);
    }

    void pop_back() noexcept
    {
        if (size_ == 0U)
        {
            return;
        }
        --size_;
    }

    void push_back(const T &value)
    {
        if (size_ >= N)
        {
            throw StaticVectorReachedMaximumCapacityException();
        }
        data_[size_++] = value;
    }

    void resize(const std::size_t new_size)
    {
        if (new_size > N)
        {
            throw StaticVectorReachedMaximumCapacityException();
        }
        size_ = new_size;
    }

    void reset() noexcept
    {
        size_ = 0U;
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    constexpr std::size_t max_size() const noexcept
    {
        return N;
    }

    T &operator[](const std::size_t index) noexcept
    {
        return data_[index];
    }

    const T &operator[](const std::size_t index) const noexcept
    {
        return data_[index];
    }

    T &at(const std::size_t index)
    {
        if ((index > N) || (size_ == 0))
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[index];
    }

    const T &at(const std::size_t index) const
    {
        if ((index > N) || (size_ == 0))
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[index];
    }

    T &front()
    {
        if (size_ == 0U)
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[0U];
    }

    const T &front() const
    {
        if (size_ == 0U)
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[0U];
    }

    T &back()
    {
        if (size_ == 0U)
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[size_ - 1];
    }

    const T &back() const
    {
        if (size_ == 0U)
        {
            throw StaticVectorIndexOutOfRangeException();
        }
        return data_[size_ - 1];
    }

  private:
    T data_[N];
    std::size_t size_;
};

} // namespace common_library::containers