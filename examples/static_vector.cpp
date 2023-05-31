#include "common_library/containers/static_vector.hpp"

#include <iostream>

class NonTrivialType
{
  public:
    NonTrivialType(int value) : value_(new int(value))
    {
    }

    NonTrivialType(const NonTrivialType &other) : value_(new int(*other.value_))
    {
    }

    NonTrivialType(NonTrivialType &&other) noexcept : value_(other.value_)
    {
        other.value_ = nullptr;
    }

    ~NonTrivialType()
    {
        delete value_;
    }

    NonTrivialType &operator=(const NonTrivialType &other)
    {
        if (this != &other)
        {
            delete value_;
            value_ = new int(*other.value_);
        }
        return *this;
    }

    NonTrivialType &operator=(NonTrivialType &&other) noexcept
    {
        if (this != &other)
        {
            delete value_;
            value_ = other.value_;
            other.value_ = nullptr;
        }
        return *this;
    }

    int value() const
    {
        return *value_;
    }

  private:
    int *value_;
};

int main()
{
    // Create a StaticVector of NonTrivialType
    common_library::containers::StaticVector<NonTrivialType, 5> vec;

    // Add elements to the vector
    vec.push_back(NonTrivialType(1));
    vec.push_back(NonTrivialType(2));
    vec.push_back(NonTrivialType(3));

    // Print the size of the vector
    std::cout << "Size: " << vec.size() << std::endl;

    // Print the elements in the vector
    for (size_t i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i].value() << ' ';
    }
    std::cout << '\n';

    // Add more elements using emplace_back
    vec.emplace_back(4);
    vec.emplace_back(5);

    // Attempting to add more elements will throw an exception
    try
    {
        vec.push_back(NonTrivialType(6));
    }
    catch (const std::out_of_range &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    // Test forward iterator
    std::cout << "Testing forward iterator " << std::endl;
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        std::cout << it->value() << std::endl;
    }

    std::cout << "Testing reverse iterator " << std::endl;
    for (auto it = vec.rbegin(); it != vec.rend(); ++it)
    {
        std::cout << it->value() << std::endl;
    }

    // Pop elements from the vector
    while (!vec.empty())
    {
        std::cout << "Popping" << std::endl;
        vec.pop_back();
    }

    return 0;
}
