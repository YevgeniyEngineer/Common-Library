#include <common_library/concurrency/lock_free_queue.hpp>
#include <common_library/concurrency/thread_safe_logger.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

using LockFreeQueueSharedPtr = std::shared_ptr<common_library::concurrency::LockFreeQueue<int>>;

constexpr int TERMINATION_TOKEN = std::numeric_limits<int>::lowest();
constexpr int NUM_CONSUMERS = 5;
constexpr int NUM_PRODUCERS = 1;

std::atomic<int> number_of_active_producers = 0;

auto &logger = common_library::concurrency::ThreadSafeLogger::getInstance(10'000);

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

    number_of_active_producers.fetch_sub(1);
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
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Check for termination token.
        if (*val == TERMINATION_TOKEN)
        {
            break;
        }

        // Note that due to thread scheduling, the logging might happen with some offset
        // causing values to be printed out-of-order, despite logger being thread safe
        logger.log("Consumer thread: ", std::this_thread::get_id(), " Value: ", *val);
    }
}

int main()
{
    auto queue = std::make_shared<common_library::concurrency::LockFreeQueue<int>>();

    // Start a producer thread and multiple consumer threads.
    std::vector<std::thread> producer_threads;
    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        producer_threads.push_back(std::thread(produce, queue));
    }

    // Introduce a small sleep time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
