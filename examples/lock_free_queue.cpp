#include <concurrency/lock_free_queue.hpp>

#include <iostream>
#include <thread>
#include <vector>

using namespace common_library;

// Sentinel value provides indication that producer thread stopped pushing to the queue
constexpr int sentinel = -1;

void producer(LockFreeQueue<int> &queue, int num_consumers)
{
    for (int i = 0; i < 10; ++i)
    {
        queue.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work.
    }

    // Add sentinel values for each consumer.
    for (int i = 0; i < num_consumers; ++i)
    {
        queue.push(sentinel);
    }
}

void consumer(LockFreeQueue<int> &queue, int num_producers)
{
    int num_sentinels_received = 0;

    while (true)
    {
        std::unique_ptr<int> val = queue.pop();

        if (val == nullptr)
        {
            // If we received a nullptr, sleep for a bit to backoff
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (*val == sentinel)
        {
            // We received a sentinel, increment the count
            num_sentinels_received++;
            if (num_sentinels_received == num_producers)
            {
                // If we received a sentinel from each producer, we can break the loop
                break;
            }
        }
        else
        {
            std::cout << "Consumer thread: " << std::this_thread::get_id() << " Value: " << *val << "\n";
        }
    }
}

int main()
{
    LockFreeQueue<int> queue;
    const int num_consumers = 5;
    const int num_producers = 1;

    // Start a producer thread and multiple consumer threads.
    std::thread producer_thread(producer, std::ref(queue), num_consumers);
    std::vector<std::thread> consumer_threads;
    for (int i = 0; i < num_consumers; ++i)
    {
        consumer_threads.push_back(std::thread(consumer, std::ref(queue), num_producers));
    }

    // Wait for all threads to finish.
    std::cout << "Waiting for threads to join" << std::endl;
    producer_thread.join();
    for (auto &thread : consumer_threads)
    {
        std::cout << "Thread " << thread.get_id() << " is joining" << std::endl;
        thread.join();
    }

    return 0;
}
