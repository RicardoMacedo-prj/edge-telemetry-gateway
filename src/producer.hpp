#pragma once

#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread_safe_queue.hpp"

/**
 * @brief Producer thread loop for receiving incoming CAN frames over UDP.
 *
 * Continuously listens on the UDP socket for raw CAN packets and enqueues
 * them into the shared ThreadSafeQueue until the running flag becomes false.
 *
 * @param sock Open UDP socket descriptor.
 * @param addr Socket address structure configured with host and port.
 * @param addr_len Length of the address structure.
 * @param queue Shared thread-safe queue where incoming frames are placed.
 * @param running Atomic flag controlling execution lifecycle.
 */
void CanReceiverLoop(int sock, sockaddr_in addr, socklen_t addr_len, 
                    ThreadSafeQueue& queue, std::atomic<bool>& running);