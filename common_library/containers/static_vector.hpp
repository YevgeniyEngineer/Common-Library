#ifndef COMMON_LIBRARY_CONTAINERS_STATIC_VECTOR
#define COMMON_LIBRARY_CONTAINERS_STATIC_VECTOR

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace common_library::containers
{
template <typename T, std::size_t N> class StaticVector final
{
  public:
    StaticVector() : size_(0)
    {
    }

    StaticVector(std::initializer_list<T> init)
    {
        if (init.size() > N)
        {
            throw std::out_of_range("Initializer list is too large for StaticVector");
        }

        for (auto &&item : init)
        {
            new (data_ + size_) T(std::move(item));
            ++size_;
        }
    }

    ~StaticVector()
    {
        clear();
    }

    template <typename... Args> void emplace_back(Args &&...args)
    {
        if (size_ >= N)
        {
            throw std::out_of_range("StaticVector is full");
        }

        new (data_ + size_) T(std::forward<Args>(args)...);
        ++size_;
    }

    void push_back(const T &value)
    {
        if (size_ >= N)
        {
            throw std::out_of_range("StaticVector is full");
        }
        new (data_ + size_) T(value);
        ++size_;
    }

    void push_back(T &&value)
    {
        if (size_ >= N)
        {
            throw std::out_of_range("StaticVector is full");
        }
        new (data_ + size_) T(std::move(value));
        ++size_;
    }

    T pop_back()
    {
        if (size_ == 0)
        {
            throw std::out_of_range("StaticVector is empty");
        }
        T value = std::move((*this)[size_ - 1]);
        reinterpret_cast<T *>(data_ + size_ - 1)->~T();
        --size_;
        return value;
    }

    T *insert(T *position, const T &value)
    {
        if (size_ >= N)
        {
            throw std::out_of_range("StaticVector is full");
        }

        if (position < begin() || position > end())
        {
            throw std::out_of_range("Position is out of range");
        }

        // Shift all elements to the right of position one step to the right
        for (T *ptr = end(); ptr != position; --ptr)
        {
            new (ptr) T(std::move(*(ptr - 1))); // move construct at new location
            (ptr - 1)->~T();                    // destroy element at old location
        }

        // Copy-construct new value at position
        new (position) T(value);

        ++size_;

        return position;
    }

    T *insert(T *position, T &&value)
    {
        if (size_ >= N)
        {
            throw std::out_of_range("StaticVector is full");
        }

        if (position < begin() || position > end())
        {
            throw std::out_of_range("Position is out of range");
        }

        // Shift all elements to the right of position one step to the right
        for (T *ptr = end(); ptr != position; --ptr)
        {
            new (ptr) T(std::move(*(ptr - 1))); // move construct at new location
            (ptr - 1)->~T();                    // destroy element at old location
        }

        // Move-construct new value at position
        new (position) T(std::move(value));

        ++size_;

        return position;
    }

    T &operator[](std::size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("Index out of range");
        }
        return *reinterpret_cast<T *>(data_ + index);
    }

    const T &operator[](std::size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("Index out of range");
        }
        return *reinterpret_cast<const T *>(data_ + index);
    }

    constexpr std::size_t capacity() const noexcept
    {
        return N;
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    bool empty() const noexcept
    {
        return size_ == 0;
    }

    void clear()
    {
        for (std::size_t i = 0; i < size_; ++i)
        {
            // Correctly call destructor for object of type T
            reinterpret_cast<T *>(data_ + i)->~T();
        }
        size_ = 0;
    }

    T *begin()
    {
        return reinterpret_cast<T *>(data_);
    }

    const T *begin() const
    {
        return reinterpret_cast<const T *>(data_);
    }

    const T *cbegin() const
    {
        return reinterpret_cast<const T *>(data_);
    }

    T *end()
    {
        return reinterpret_cast<T *>(data_) + size_;
    }

    const T *end() const
    {
        return reinterpret_cast<const T *>(data_) + size_;
    }

    const T *cend() const
    {
        return reinterpret_cast<const T *>(data_) + size_;
    }

    std::reverse_iterator<T *> rbegin()
    {
        return std::reverse_iterator<T *>(end());
    }

    std::reverse_iterator<const T *> rbegin() const
    {
        return std::reverse_iterator<const T *>(end());
    }

    std::reverse_iterator<const T *> crbegin() const
    {
        return std::reverse_iterator<const T *>(cend());
    }

    std::reverse_iterator<T *> rend()
    {
        return std::reverse_iterator<T *>(begin());
    }

    std::reverse_iterator<const T *> rend() const
    {
        return std::reverse_iterator<const T *>(begin());
    }

    std::reverse_iterator<const T *> crend() const
    {
        return std::reverse_iterator<const T *>(cbegin());
    }

    T &front()
    {
        if (size_ == 0)
        {
            throw std::out_of_range("StaticVector is empty");
        }

        return *reinterpret_cast<T *>(data_);
    }

    const T &front() const
    {
        if (size_ == 0)
        {
            throw std::out_of_range("StaticVector is empty");
        }

        return *reinterpret_cast<const T *>(data_);
    }

    T &back()
    {
        if (size_ == 0)
        {
            throw std::out_of_range("StaticVector is empty");
        }

        return *reinterpret_cast<T *>(data_ + size_ - 1);
    }

    const T &back() const
    {
        if (size_ == 0)
        {
            throw std::out_of_range("StaticVector is empty");
        }

        return *reinterpret_cast<const T *>(data_ + size_ - 1);
    }

  private:
    std::aligned_storage_t<sizeof(T), alignof(T)> data_[N];
    std::size_t size_;
};
} // namespace common_library::containers

#endif // COMMON_LIBRARY_CONTAINERS_STATIC_VECTOR