#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <atomic>
#include <mosquitto.h>
#include <unistd.h>
#include <thread>
#include "gatewayConfig.hpp"
#include "mqtt_client.hpp"
#include "thread_safe_queue.hpp"
#include "producer.hpp"
#include "consumer.hpp"

// Atomic flag controlling the main event loop lifecycle
std::atomic<bool> running{true};

// Runtime gateway configuration holding network and storage parameters
GatewayConfig config;

/**
 * @brief Handles the SIGINT signal (triggered by Ctrl+C) for graceful shutdown.
 *
 * Sets the global atomic running flag to false, allowing ongoing loops
 * and background threads to terminate and release resources cleanly.
 *
 * @param signal The received operating system signal number (unused).
 */
void handle_sigint(int /*signal*/)
{
    std::cout << "\n[Shutdown] Stopping cleanly..." << std::endl;
    running = false;
}

int main()
{
    // Register the signal handler for graceful shutdown on Ctrl+C
    std::signal(SIGINT, handle_sigint);

    if (!loadConfig("config/gateway.conf", config))
    {
        std::cerr << "[Gateway] Configuration failure. Exiting process." << std::endl;
        return 1;
    }

    // Initialize the Mosquitto MQTT library
    mosquitto_lib_init();

    // Create a UDP datagram socket for receiving CAN frames
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Failed to create socket!" << std::endl;
        return 1;
    }

    // Configure local UDP socket address (localhost:4000)
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config.SOCKET_PORT);
    inet_pton(AF_INET, config.SOCKET_ADDRESS.c_str(), &addr.sin_addr);
    socklen_t addr_len = sizeof(addr);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Failed to bind socket!" << std::endl;
        return 1;
    }

    // Set receive timeout so recvfrom does not block indefinitely when the vehicle is offline.
    // This allows the while(running) loop to periodically check the shutdown flag.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500 ms timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Create a new Mosquitto client instance
    struct mosquitto *mosq = mosquitto_new("can-publisher", true, NULL);
    if (!mosq)
    {
        std::cerr << "Failed to create Mosquitto instance." << std::endl;
        return 1;
    }

    // Connection state flag shared with Mosquitto callbacks
    bool is_connected = false;

    // Register connection and disconnection callbacks
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_user_data_set(mosq, &is_connected); 
    
    // Start background network loop thread to handle keepalives and QoS ACKs
    mosquitto_loop_start(mosq);
    
    // Connect to local Mosquitto MQTT broker
    int rc = mosquitto_connect(mosq, config.MOSQUITTO_BROKER_ADDRESS.c_str(), config.MOSQUITTO_BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "[Gateway] Failed to initiate connection: " << mosquitto_strerror(rc) << std::endl;
    }

    // Shared FIFO queue connecting the producer and consumer threads
    ThreadSafeQueue queue;

    // Launch producer thread (UDP frame reception) and consumer thread (decoding & storage)
    std::thread producer(CanReceiverLoop, sock, addr, addr_len, std::ref(queue), std::ref(running));
    std::thread processor(CanProcessorLoop, mosq, std::ref(is_connected), 
                        std::ref(queue), config, std::ref(running));

    // Wait for both worker threads to terminate cleanly on shutdown
    producer.join();
    processor.join();

    // Close the UDP socket
    close(sock);

    // Clean up Mosquitto client and library resources
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}