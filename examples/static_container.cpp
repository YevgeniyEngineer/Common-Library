#include <common_library/containers/static_container.hpp>

#include <iostream>
#include <ostream>
#include <random>

struct Point
{
    float x;
    float y;
    float z;

    friend std::ostream &operator<<(std::ostream &os, const Point &other)
    {
        os << "(" << other.x << ", " << other.y << ", " << other.z << ")";
        return os;
    }
};

struct Plane
{
    Point points[3];

    friend std::ostream &operator<<(std::ostream &os, const Plane &other)
    {
        os << "[" << other.points[0] << ", " << other.points[1] << ", " << other.points[2] << "]";
        return os;
    }
};

int main()
{
    using namespace common_library::containers;

    StaticContainer<Plane, 100> planes;
    std::cout << *(planes.begin() + 10) << std::endl;

    planes.resize(10);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-100.0f, 100.0f);

    for (auto &plane : planes)
    {
        for (auto &point : plane.points)
        {
            point.x = distribution(generator);
            point.y = distribution(generator);
            point.z = distribution(generator);
        }
    }

    // Forward iterators
    for (auto it = planes.begin(); it != planes.end(); ++it)
    {
        // Iterator dereference
        std::cout << *it << std::endl;
    }

    std::cout << std::endl << std::endl << std::endl;

    // Remove two element
    planes.pop_back();
    planes.pop_back();

    // Iteration
    for (std::size_t i = 0; i < planes.size(); ++i)
    {
        // Access without exception checking
        const auto &plane = planes[i];

        // Random index access
        try
        {
            std::cout << planes.at(i) << std::endl;
        }
        catch (const StaticVectorIndexOutOfRangeException &ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    // Try to resize beyond capacity
    try
    {
        planes.resize(1000);
    }
    catch (const StaticVectorReachedMaximumCapacityException &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}