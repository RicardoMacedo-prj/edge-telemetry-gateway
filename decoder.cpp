#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "car.hpp"

std::atomic<bool> running{true};

// Signal handler for graceful shutdown on Ctrl+C
void handle_sigint(int signal)
{
    std::cout << "\n[Shutdown] Stopping cleanly..." << std::endl;
    running = false;
}

/**
 * @brief Decodes an 8-byte OBD-II CAN frame and prints reconstructed metric.
 */
void decode_can(CanFrame canFrame)
{
    // Inspect byte 1 to determine the PID sensor type
    if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::rpm))
    {
        uint16_t rpmData = (static_cast<uint16_t>(canFrame.data[2]) << 8) | static_cast<uint16_t>(canFrame.data[3]);
        double rpm = static_cast<double>(rpmData) / 4.0;
        std::cout << "Decoded RPM: " << rpm << std::endl;
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::speed))
    {
        double speed = static_cast<double>(canFrame.data[2]);
        std::cout << "Decoded Speed: " << speed << " km/h" << std::endl;
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
    {
        double coolantTemp = static_cast<double>(canFrame.data[2]) - 40.0;
        std::cout << "Decoded Coolant Temperature: " << coolantTemp << std::endl;
    }
}

int main()
{
    // Register the signal handler for Ctrl+C
    std::signal(SIGINT, handle_sigint);

    // Create a UDP socket for sending CAN frames
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Failed to create socket!" << std::endl;
        return 1;
    }

    // UDP socket address configuration for localhost:4000
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    socklen_t addr_len = sizeof(addr);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Failed to bind socket!" << std::endl;
        return 1;
    }

    std::cout << "=== Testing CAN Decoder ===" << std::endl;

    // Buffer to receive CAN frames
    uint8_t receiveBuffer[8];
    CanFrame receivedFrame;

    // Continuously listen for incoming CAN frames and decode them
    while (running)
    {
        recvfrom(sock, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&addr, &addr_len);
        for (int i = 0; i < 8; ++i)
        {
            receivedFrame.data[i] = receiveBuffer[i];
            std::cout << "Received CAN Byte " << i << ": " << static_cast<int>(receiveBuffer[i]) << std::endl;
        }

        decode_can(receivedFrame);
    }

    close(sock);

    return 0;
}