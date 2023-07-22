#include <chrono>
#include <common_library/containers/bounded_dynamic_array.hpp>
#include <iostream>
#include <vector>

int main()
{
    common_library::containers::BoundedDynamicArray<float, 1'000'000, false>
        array;

    std::cout << "size: " << array.size() << std::endl;
    std::cout << "capacity: " << array.capacity() << std::endl;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < array.capacity(); ++i)
    {
        array.push_back(i);
    }

    for (std::size_t i = 0; i < array.capacity(); ++i)
    {
        array.pop_back();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time (s): " << (t2 - t1).count() / 1e9 << std::endl;

    std::vector<float> vector;
    vector.reserve(1'000'000);

    auto t3 = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < array.capacity(); ++i)
    {
        vector.push_back(i);
    }

    for (std::size_t i = 0; i < array.capacity(); ++i)
    {
        vector.pop_back();
    }

    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time (s): " << (t4 - t3).count() / 1e9 << std::endl;

    return 0;
}