#include <common_library/concurrency/thread_safe_queue.hpp>

#include <chrono>
#include <iostream>
#include <thread>

void producer()
{
    auto &queue = common_library::concurrency::ThreadSafeQueue<int>::getInstance();
    for (int i = 0; i < 100; ++i)
    {
        queue.push(i);
    }
}

void consumer()
{
    auto t1 = std::chrono::steady_clock::now();

    auto &queue = common_library::concurrency::ThreadSafeQueue<int>::getInstance();
    for (int i = 0; i < 100; ++i)
    {
        std::optional<int> value;
        do
        {
            value = queue.pop();
        } while (!value);
        std::cout << "Popped " << *value << " from the queue.\n";
    }

    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Elapsed Thread Safe Queue consumer time [ns]: " << (t2 - t1).count() / 1e9 << std::endl;
}

int main()
{
    std::thread producer_thread(&producer);
    std::thread consumer_thread(&consumer);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
