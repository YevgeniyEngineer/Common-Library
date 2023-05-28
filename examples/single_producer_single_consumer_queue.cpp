#include <common_library/concurrency/single_producer_single_consumer_queue.hpp>

#include <chrono>
#include <iostream>
#include <thread>

constexpr std::size_t QUEUE_SIZE = 100;

void producer(common_library::concurrency::SingleProducerSingleConsumerQueue<int> &queue)
{
    for (int i = 0; i < 100; ++i)
    {
        while (!queue.push(i))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void consumer(common_library::concurrency::SingleProducerSingleConsumerQueue<int> &queue)
{
    auto t1 = std::chrono::steady_clock::now();

    int value;
    for (int i = 0; i < 100; ++i)
    {
        while (!queue.pop(value))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std::cout << "Popped " << value << " from the queue.\n";
    }

    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Elapsed SPSCQueue consumer time [ns]: " << (t2 - t1).count() / 1e9 << std::endl;
}

int main()
{
    common_library::concurrency::SingleProducerSingleConsumerQueue<int> queue(QUEUE_SIZE);

    std::thread producer_thread(&producer, std::ref(queue));
    std::thread consumer_thread(&consumer, std::ref(queue));

    producer_thread.join();
    consumer_thread.join();

    return 0;
}