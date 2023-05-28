#ifndef COMMON_LIBRARY_CONTAINERS_BOUNDED_STACK_VECTOR
#define COMMON_LIBRARY_CONTAINERS_BOUNDED_STACK_VECTOR

#include <algorithm>        // std::move_backward
#include <array>            // std::array
#include <cstddef>          // std::ptrdiff_t
#include <cstdint>          // std::size_t
#include <initializer_list> // std::initializer_list
#include <iostream>         // std::cout
#include <iterator>         // std::reverse_iterator, std::distance
#include <stdexcept>        // std::overflow_error, std::underflow_error
#include <string_view>      // std::swap
#include <utility>          // std::move

namespace common_library::containers
{
class BoundedStackVectorInitializationError : public std::overflow_error
{
  public:
    BoundedStackVectorInitializationError() : std::overflow_error("Initializer list too large for BoundedStackVector")
    {
    }
};

class BoundedStackVectorOverflow : public std::runtime_error
{
  public:
    BoundedStackVectorOverflow() : std::runtime_error("BoundedStackVector is full")
    {
    }
};

class BoundedStackVectorUnderflow : public std::runtime_error
{
  public:
    BoundedStackVectorUnderflow() : std::runtime_error("BoundedStackVector is empty")
    {
    }
};

class BoundedStackVectorInvalidIteratorAccess : public std::out_of_range
{
  public:
    BoundedStackVectorInvalidIteratorAccess()
        : std::out_of_range("BoundedStackVector iterator accessing invalid memory location")
    {
    }
};

class BoundedStackVectorInvalidIndexAccess : public std::out_of_range
{
  public:
    BoundedStackVectorInvalidIndexAccess() : std::out_of_range("BoundedStackVector index access is out of range")
    {
    }
};

/// @brief BoundedStackVector is a wrapper class around std::array that implements stack allocated resizable vector
/// @tparam T Type of the values
/// @tparam N Number of elements
template <typename T, std::size_t N> class BoundedStackVector final
{
  public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /// @brief Default constructor of the BoundedStackVector class.
    BoundedStackVector() : size_(0)
    {
    }

    /// @brief Default destructor of the BoundedStackVector class.
    ~BoundedStackVector()
    {
        size_ = 0;
    }

    /// @brief Constructor of the BoundedStackVector class from the initializer list.
    /// @param initializer_list initializer list to move data from to the BoundedStackVector's data.
    /// @throws BoundedStackVectorInitializationError if initializer list contains more elements that Vector's maximum
    /// capacity
    BoundedStackVector(std::initializer_list<T> initializer_list)
    {
        if (initializer_list.size() > N)
        {
            throw BoundedStackVectorInitializationError();
        }
        size_ = initializer_list.size();
        std::move(std::make_move_iterator(initializer_list.begin()), std::make_move_iterator(initializer_list.end()),
                  data_.begin());
    }

    /// @brief Copy constructor.
    /// @param other The object to copy data from.
    BoundedStackVector(const BoundedStackVector &other) : data_(other.data_), size_(other.size_)
    {
    }

    /// @brief Copy assignment operator.
    /// @param other The object to copy data from.
    /// @return New BoundedStackVector object constructed from copying data from the other BoundedStackVector.
    BoundedStackVector &operator=(const BoundedStackVector &other)
    {
        if (this == &other)
        {
            return *this;
        }

        data_ = other.data_;
        size_ = other.size_;

        return *this;
    }

    /// @brief Move constructor.
    /// @param other Other BoundedStackVector to move data from.
    BoundedStackVector(BoundedStackVector &&other) noexcept : data_(std::move(other.data_)), size_(other.size_)
    {
        other.size_ = 0;
    }

    /// @brief Move operator.
    /// @param other Other BoundedStackVector to move data from.
    /// @return Moved BoundedStackVector.
    BoundedStackVector &operator=(BoundedStackVector &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        data_ = std::move(other.data_);
        size_ = other.size_;
        other.size_ = 0;

        return *this;
    }

    /// @brief Swap data between two BoundedStackVector objects.
    /// @param other Other BoundedStackVector to exchange data with.
    void swap(BoundedStackVector &other) noexcept
    {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
    }

    /// @brief Returns whether the BoundedStackVector is empty.
    /// @return True if empty, else False.
    [[nodiscard]] bool empty() const noexcept
    {
        return (size_ == 0UL);
    }

    /// @brief Gets the number of elements in the BoundedStackVector.
    /// @return Current data size.
    [[nodiscard]] size_type size() const noexcept
    {
        return size_;
    }

    /// @brief Get the capacity of BoundedStackVector
    /// @return Maximum number of elements that BoundedStackVector can hold
    [[nodiscard]] size_type max_size() const noexcept
    {
        return N;
    }

    /// @brief Resizes BoundedStackVector to 0
    void clear() noexcept
    {
        size_ = 0UL;
    }

    /// @brief Add a value to the end of BoundedStackVector
    /// @param value Value to be copied
    /// @throws BoundedStackVectorOverflow if the BoundedStackVector is full.
    void push_back(const T &value)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }
        data_[size_++] = value;
    }

    /// @brief Move a value to the end of the BoundedStackVector.
    /// @param value Value to be moved.
    /// @throws BoundedStackVectorOverflow if the BoundedStackVector is full.
    void push_back(T &&value)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }
        data_[size_++] = std::move(value);
    }

    /// @brief Add a value to the end of the BoundedStackVector.
    /// @tparam ...Args Argument types forwarded to construct the new element.
    /// @param ...args Argument values forwarded to construct the new element.
    /// @throws BoundedStackVectorOverflow if the BoundedStackVector is full.
    template <typename... Args> void emplace_back(Args &&...args)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }
        data_[size_++] = T(std::forward<Args>(args)...);
    }

    /// @brief Construct and insert element at the specified position of the BoundedStackVector.
    /// @tparam ...Args Argument types forwarded to construct the new element.
    /// @param pos Random access iterator position that points to the insertion position in the BoundedStackVector.
    /// @param ...args Argument values forwarded to construct the new element.
    /// @return Updated Pointer to the BoundedStackVector data.
    /// @throws BoundedStackVectorOverflow if maximum size of the BoundedStackVector was exceeded
    /// @throws BoundedStackVectorInvalidIteratorAccess if the invalid iterator position was provided.
    template <typename... Args> iterator emplace(iterator pos, Args &&...args)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }

        if (pos < begin() || pos > end())
        {
            throw BoundedStackVectorInvalidIteratorAccess();
        }

        if (pos != end())
        {
            std::move_backward(pos, end(), end() + 1);
        }
        *pos = T(std::forward<Args>(args)...);

        ++size_;
        return pos;
    }

    /// @brief Remove one element from the end of the BoundedStackVector.
    /// @throws BoundedStackVectorUnderflow if the BoundedStackVector is empty.
    void pop_back()
    {
        if (empty())
        {
            throw BoundedStackVectorUnderflow();
        }
        --size_;
    }

    /// @brief Insert an element at a specified position.
    /// @param pos Position of the BoundedStackVector where the new elements are inserted provided as a random access
    /// iterator.
    /// @param value Value to be copied to the inserted elements.
    /// @return Random access iterator that points to elements.
    /// @throws BoundedStackVectorOverflow if the BoundedStackVector is full.
    /// @throws BoundedStackVectorInvalidIteratorAccess error if the provided insert position is out of range.
    iterator insert(iterator pos, const T &value)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }
        if (pos < begin() || pos > end())
        {
            throw BoundedStackVectorInvalidIteratorAccess();
        }

        std::move_backward(pos, end(), end() + 1);
        *pos = value;
        ++size_;

        return pos;
    }

    /// @brief Insert an element at a specified position.
    /// @param pos Position of the BoundedStackVector where the new elements are inserted provided as a random access
    /// iterator.
    /// @param value Value to be moved to the inserted elements.
    /// @return Random access iterator that points to elements.
    /// @throws BoundedStackVectorOverflow if the BoundedStackVector is full.
    /// @throws BoundedStackVectorInvalidIteratorAccess error if the provided insert position is out of range.
    iterator insert(iterator pos, T &&value)
    {
        if (size_ >= N)
        {
            throw BoundedStackVectorOverflow();
        }
        if (pos < begin() || pos > end())
        {
            throw BoundedStackVectorInvalidIteratorAccess();
        }

        std::move_backward(pos, end(), end() + 1);
        *pos = std::move(value);
        ++size_;

        return pos;
    }

    /// @brief Removes from the BoundedStackVector a single element.
    /// @param pos Random access iterator position corresponding to the element to be removed.
    /// @return New access iterator with a removed element.
    /// @throws BoundedStackVectorInvalidIteratorAccess if invalid position is provided.
    [[nodiscard]] iterator erase(iterator pos)
    {
        if (pos < begin() || pos >= end())
        {
            throw BoundedStackVectorInvalidIteratorAccess();
        }

        iterator next = pos + 1;
        std::move(next, end(), pos);
        --size_;

        return pos;
    }

    /// @brief Removes from the BoundedStackVector a range of elements [first, last).
    /// @param first Random access iterator type to the first element to be removed.
    /// @param last Random access iterator type to last non-inclusive element.
    /// @return New access iterator with removed elements.
    /// @throws BoundedStackVectorInvalidIteratorAccess if invalid position is provided.
    [[nodiscard]] iterator erase(iterator first, iterator last)
    {
        if (first < begin() || first > last || last > end())
        {
            throw BoundedStackVectorInvalidIteratorAccess();
        }

        iterator new_end = std::move(last, end(), first);
        size_ -= std::distance(first, last);

        return first;
    }

    /// @brief Get a reference to the element stored at the specified position.
    /// @param index Index to the element stored in the BoundedStackVector.
    /// @return Non-const reference to the element in the data.
    [[nodiscard]] reference operator[](size_type index) noexcept
    {
        return data_[index];
    }

    /// @brief Get a constant reference to the element stored at the specified position.
    /// @param index Index to the element stored in the BoundedStackVector.
    /// @return Const reference to the element in the data.
    [[nodiscard]] const_reference operator[](size_type index) const noexcept
    {
        return data_[index];
    }

    /// @brief Get a reference to the element stored at the specified position.
    /// @param index Index to the element stored in the BoundedStackVector.
    /// @return Non-const reference to the element in the data.
    /// @throws BoundedStackVectorInvalidIndexAccess If the index is out of range.
    [[nodiscard]] reference at(size_type index) const
    {
        if (index >= size_)
        {
            throw BoundedStackVectorInvalidIndexAccess();
        }
        return data_[index];
    }

    /// @brief Returns an iterator pointing to the first element in the BoundedStackVector.
    /// @return Non-const pointer to the first position of the data stored within the BoundedStackVector.
    [[nodiscard]] iterator begin() noexcept
    {
        return data_.data();
    }

    /// @brief Returns an iterator pointing to the first element in the BoundedStackVector.
    /// @return Const pointer to the first position of the data stored within the BoundedStackVector.
    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return data_.data();
    }

    /// @brief Returns an iterator referring to the element one past the end position of the BoundedStackVector.
    /// @return Non-const pointer to the past-the-end position of the data stored within the BoundedStackVector.
    [[nodiscard]] iterator end() noexcept
    {
        return data_.data() + size_;
    }

    /// @brief Returns an iterator referring to the element one past the end position of the BoundedStackVector.
    /// @return Const pointer to the past-the-end position of the data stored within the BoundedStackVector.
    [[nodiscard]] const_iterator cend() const noexcept
    {
        return data_.data() + size_;
    }

    /// @brief Returns a reverse iterator pointing to the last element in the BoundedStackVector.
    /// @return Non-const pointer to the last element of the data stored within the BoundedStackVector.
    [[nodiscard]] reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    /// @brief Returns a const reverse iterator pointing to the last element in the BoundedStackVector.
    /// @return Const pointer to the last element of the data stored within the BoundedStackVector.
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    /// @brief Returns a reverse iterator pointing to the before the start element in the BoundedStackVector.
    /// @return Non-const pointer to the element of the data stored within the BoundedStackVector preceding the first
    /// element.
    [[nodiscard]] reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    /// @brief Returns a const reverse iterator pointing to the before the start element in the BoundedStackVector.
    /// @return Const pointer to the element of the data stored within the BoundedStackVector preceding the first
    /// element.
    [[nodiscard]] const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    /// @brief Returns the non-const reference to the first element in the BoundedStackVector.
    /// @throws BoundedStackVectorUnderflow if the BoundedStackVector is empty.
    [[nodiscard]] reference front()
    {
        if (empty())
        {
            throw BoundedStackVectorUnderflow();
        }
        return data_[0];
    }

    /// @brief Returns the const reference to the first element in the BoundedStackVector.
    /// @throws BoundedStackVectorUnderflow if the BoundedStackVector is empty.
    [[nodiscard]] const_reference front() const
    {
        if (empty())
        {
            throw BoundedStackVectorUnderflow();
        }
        return data_[0];
    }

    /// @brief Returns the non-const reference to the last element in the BoundedStackVector.
    /// @throws BoundedStackVectorUnderflow if the BoundedStackVector is empty.
    [[nodiscard]] reference back()
    {
        if (empty())
        {
            throw BoundedStackVectorUnderflow();
        }
        return data_[size_ - 1];
    }

    /// @brief Returns the const reference to the last element in the BoundedStackVector.
    /// @throws BoundedStackVectorUnderflow if the BoundedStackVector is empty.
    [[nodiscard]] const_reference back() const
    {
        if (empty())
        {
            throw BoundedStackVectorUnderflow();
        }
        return data_[size_ - 1];
    }

  private:
    std::array<T, N> data_;
    size_type size_;
};
} // namespace common_library::containers

#endif // COMMON_LIBRARY_CONTAINERS_BOUNDED_STACK_VECTOR