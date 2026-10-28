#include "thread_safe_queue.hpp"
#include <chrono>

void ThreadSafeQueue::push(const CanFrame& frame)
{
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(frame);
    cv.notify_one();
}

CanFrame ThreadSafeQueue::pop(std::atomic<bool>& running)
{
    std::unique_lock<std::mutex> lock(mtx);

    // Wait until there are items in the queue or until running becomes false
    while (queue.empty() && running) 
    {
        cv.wait_for(lock, std::chrono::milliseconds(200));
    }

    // Return an unknown/empty frame if shutdown occurred while the queue was empty
    if (queue.empty())
    {
        CanFrame emptyFrame{};
        emptyFrame.data[1] = static_cast<uint8_t>(PID_standart::unknown);
        return emptyFrame;
    }

    CanFrame frame = queue.front();
    queue.pop();
    return frame;
}