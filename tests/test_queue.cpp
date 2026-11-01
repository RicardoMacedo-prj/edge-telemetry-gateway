#include <cassert>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "thread_safe_queue.hpp"

void test_push_and_pop_single_thread()
{
    ThreadSafeQueue queue;
    std::atomic<bool> running{true};

    CanFrame frame{};
    frame.data[0] = 0x01;
    frame.data[1] = static_cast<uint8_t>(PID_standart::rpm);

    queue.push(frame);

    CanFrame popped = queue.pop(running);
    assert(popped.data[0] == 0x01);
    assert(popped.data[1] == static_cast<uint8_t>(PID_standart::rpm));
}

void test_pop_timeout_when_empty()
{
    ThreadSafeQueue queue;
    std::atomic<bool> running{false};

    // When running is false and queue is empty, pop() should return an unknown PID frame
    CanFrame popped = queue.pop(running);
    assert(popped.data[1] == static_cast<uint8_t>(PID_standart::unknown));
}

void test_concurrent_producer_consumer()
{
    ThreadSafeQueue queue;
    std::atomic<bool> running{true};
    const int total_items = 1000;
    std::vector<int> received_ids;

    std::thread producer([&]() {
        for (int i = 0; i < total_items; ++i)
        {
            CanFrame frame{};
            // Store integer i inside data[0]..data[3]
            frame.data[0] = static_cast<uint8_t>((i >> 24) & 0xFF);
            frame.data[1] = static_cast<uint8_t>((i >> 16) & 0xFF);
            frame.data[2] = static_cast<uint8_t>((i >> 8) & 0xFF);
            frame.data[3] = static_cast<uint8_t>(i & 0xFF);
            queue.push(frame);
        }
    });

    std::thread consumer([&]() {
        while (static_cast<int>(received_ids.size()) < total_items)
        {
            CanFrame frame = queue.pop(running);
            int id = (static_cast<int>(frame.data[0]) << 24) |
                     (static_cast<int>(frame.data[1]) << 16) |
                     (static_cast<int>(frame.data[2]) << 8) |
                     static_cast<int>(frame.data[3]);
            received_ids.push_back(id);
        }
    });

    producer.join();
    consumer.join();

    assert(static_cast<int>(received_ids.size()) == total_items);
    for (int i = 0; i < total_items; ++i)
    {
        assert(received_ids[i] == i);
    }
}

int main()
{
    test_push_and_pop_single_thread();
    test_pop_timeout_when_empty();
    test_concurrent_producer_consumer();
    return 0;
}
