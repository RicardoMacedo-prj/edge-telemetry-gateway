#include "producer.hpp"

void CanReceiverLoop(int sock, sockaddr_in addr, socklen_t addr_len, 
                    ThreadSafeQueue& queue, std::atomic<bool>& running)
{
    CanFrame receivedCanFrame;

    while (running)
    {
        // Receive raw 8-byte CAN frame over UDP
        ssize_t bytes_received = recvfrom(sock, receivedCanFrame.data, sizeof(receivedCanFrame.data), 0, (struct sockaddr *)&addr, &addr_len);

        // Ignore timeout or empty read and continue listening
        if (bytes_received < 0)
        {
            continue;
        }

        // Enqueue received frame for consumer processing
        queue.push(receivedCanFrame);
    }
}

