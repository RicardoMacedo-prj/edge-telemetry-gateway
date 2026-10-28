#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "vehicle.hpp"

/**
 * @brief Thread-safe FIFO queue for CanFrame messages.
 *
 * Synchronizes producer (CAN receiver) and consumer (processor) threads
 * using a mutex and condition variable to coordinate data access.
 */
class ThreadSafeQueue {
private:
    std::queue<CanFrame> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    /**
     * @brief Pushes a CanFrame to the back of the queue and notifies one waiting consumer.
     * @param frame The CAN frame to enqueue.
     */
    void push(const CanFrame& frame);

    /**
     * @brief Retrieves and removes a CanFrame from the front of the queue.
     *
     * Waits until a frame arrives or the 200 ms timeout expires,
     * allowing the consumer thread to exit cleanly during shutdown.
     *
     * @param running Atomic flag controlling execution lifecycle.
     * @return The next CanFrame, or an empty frame with PID_standart::unknown if timed out on shutdown.
     */
    CanFrame pop(std::atomic<bool>& running);
};