#pragma once

#include <atomic>
#include <mosquitto.h>
#include "gatewayConfig.hpp"
#include "thread_safe_queue.hpp"

/**
 * @brief Consumer thread loop for processing CAN frames from the queue.
 *
 * Continuously dequeues CAN frames from the ThreadSafeQueue, decodes them,
 * applies edge alert rules, buffers records into local storage, and publishes
 * batches to the MQTT broker while connected.
 *
 * @param mosq Pointer to the initialized Mosquitto client instance.
 * @param is_connected Reference to the MQTT connection status flag.
 * @param queue Shared thread-safe queue containing incoming CAN frames.
 * @param config Runtime configuration parameters.
 * @param running Atomic flag controlling execution lifecycle.
 */
void CanProcessorLoop(mosquitto* mosq, bool& is_connected, ThreadSafeQueue& queue, 
                    GatewayConfig config, std::atomic<bool>& running);