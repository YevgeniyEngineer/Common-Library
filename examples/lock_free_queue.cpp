#include <concurrency/lock_free_queue.hpp>
#include <concurrency/ts_logger.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

using LockFreeQueueSharedPtr = std::shared_ptr<common_library::LockFreeQueue<int>>;

constexpr int TERMINATION_TOKEN = std::numeric_limits<int>::lowest();
constexpr int NUM_CONSUMERS = 5;
constexpr int NUM_PRODUCERS = 5;

std::atomic<int> number_of_active_producers = 0;

auto &logger = common_library::TSLogger::getInstance(10'000);

void produce(LockFreeQueueSharedPtr queue)
{
    ++number_of_active_producers;

    for (int i = 0; i < 100; ++i)
    {
        queue->push(i);

        logger.log("Producer thread ", std::this_thread::get_id(), " Value: ", i);

        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    --number_of_active_producers;

    if (number_of_active_producers.load() == 0)
    {
        for (int i = 0; i < NUM_CONSUMERS; ++i)
        {
            queue->push(TERMINATION_TOKEN);

            logger.log("Producer thread ", std::this_thread::get_id(), " Termination token: ", TERMINATION_TOKEN);
        }
    }
}

void consume(LockFreeQueueSharedPtr queue)
{
    while (true)
    {
        auto val = queue->pop();

        if (val == nullptr)
        {
            // If we received a nullptr, sleep for a bit to backoff
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // Check for termination token.
        if (*val == TERMINATION_TOKEN)
        {
            logger.log("Consumer thread: ", std::this_thread::get_id(), " terminating");
            break;
        }

        logger.log("Consumer thread: ", std::this_thread::get_id(), " Value: ", *val);
    }
}

int main()
{
    auto queue = std::make_shared<common_library::LockFreeQueue<int>>();

    // Start a producer thread and multiple consumer threads.
    std::vector<std::thread> producer_threads;
    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        producer_threads.push_back(std::thread(produce, queue));
    }

    std::vector<std::thread> consumer_threads;
    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        consumer_threads.push_back(std::thread(consume, queue));
    }

    // Wait for all threads to finish.
    logger.log("Waiting for threads to join");
    for (auto &producer_thread : producer_threads)
    {
        logger.log("Producer thread ", producer_thread.get_id(), " is joining");
        producer_thread.join();
    }

    for (auto &consumer_thread : consumer_threads)
    {
        logger.log("Consumer thread ", consumer_thread.get_id(), " is joining");
        consumer_thread.join();
    }

    return 0;
}

// #include <concurrency/lock_free_queue.hpp>
// // Sentinel value provides indication that producer thread stopped pushing to the queue
// constexpr int sentinel = -1;

// void producer(common_library::LockFreeQueue<int> &queue, int num_consumers)
// {
//     for (int i = 0; i < 10; ++i)
//     {
//         queue.push(i);
//         std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work.
//     }

//     // Add sentinel values for each consumer.
//     for (int i = 0; i < num_consumers; ++i)
//     {
//         queue.push(sentinel);
//     }
// }

// void consumer(common_library::LockFreeQueue<int> &queue, int num_producers)
// {
//     int num_sentinels_received = 0;

//     while (true)
//     {
//         std::unique_ptr<int> val = queue.pop();

//         if (val == nullptr)
//         {
//             // If we received a nullptr, sleep for a bit to backoff
//             std::this_thread::sleep_for(std::chrono::milliseconds(1));
//             continue;
//         }

//         if (*val == sentinel)
//         {
//             // We received a sentinel, increment the count
//             num_sentinels_received++;
//             if (num_sentinels_received == num_producers)
//             {
//                 // If we received a sentinel from each producer, we can break the loop
//                 break;
//             }
//         }
//         else
//         {
//             std::cout << "Consumer thread: " << std::this_thread::get_id() << " Value: " << *val << "\n";
//         }
//     }
// }

// int main()
// {
//     common_library::LockFreeQueue<int> queue;
//     const int num_consumers = 5;
//     const int num_producers = 1;

//     // Start a producer thread and multiple consumer threads.
//     std::thread producer_thread(producer, std::ref(queue), num_consumers);
//     std::vector<std::thread> consumer_threads;
//     for (int i = 0; i < num_consumers; ++i)
//     {
//         consumer_threads.push_back(std::thread(consumer, std::ref(queue), num_producers));
//     }

//     // Wait for all threads to finish.
//     std::cout << "Waiting for threads to join" << std::endl;
//     producer_thread.join();
//     for (auto &thread : consumer_threads)
//     {
//         std::cout << "Thread " << thread.get_id() << " is joining" << std::endl;
//         thread.join();
//     }

//     return 0;
// }
