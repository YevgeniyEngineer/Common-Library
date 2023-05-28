#include <common_library/containers/bounded_stack_vector.hpp>

int main()
{
    common_library::containers::BoundedStackVector<int, 10> vec;

    // Add elements to the StackVector
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    // Access elements
    std::cout << "vec[2] = " << vec[2] << std::endl;

    // Iterate over elements and print them
    for (const auto &val : vec)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Remove the last element
    vec.pop_back();

    // Check the size of the StackVector
    std::cout << "Size: " << vec.size() << std::endl;

    // for (auto it = vec.cbegin(); it != vec.cend(); ++it)
    // {
    //     std::cout << *it << std::endl;
    // }

    // for (auto it = vec.rbegin(); it != vec.rend(); ++it)
    // {
    //     std::cout << *it << std::endl;
    // }

    // Insert elements
    vec.insert(vec.begin() + 2, 42);
    vec.insert(vec.begin() + 4, 99);
    vec.emplace_back(5);

    // Iterate over elements and print them
    for (const auto &val : vec)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Create a StackVector using an initializer list
    common_library::containers::BoundedStackVector<int, 10> vec2 = {1, 2, 3, 4, 5};

    // Iterate over elements and print them
    for (const auto &val : vec2)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    vec2.emplace(vec2.begin() + 1, 101);

    // Iterate over elements and print them
    for (const auto &val : vec2)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << vec2.front() << std::endl;
    std::cout << vec2.back() << std::endl;

    common_library::containers::BoundedStackVector<int, 1'000'000'000> vec3;
    std::cout << "Size of the vector: " << vec3.max_size() << std::endl;

    return 0;
}