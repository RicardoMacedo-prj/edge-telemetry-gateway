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

// Available vehicle sensors list
PID_standart sensors[] = {PID_standart::rpm, PID_standart::speed, PID_standart::coolantTemp};

/**
 * @brief Encodes vehicle telemetry into an 8-byte OBD-II CAN frame.
 */
CanFrame encode_can(PID_standart currentSensor, CarStatus carStatus)
{
    CanFrame canFrame;
    uint8_t lenght = 0;
    uint8_t highByte = 0;
    uint8_t lowByte = 0;

    if (currentSensor == PID_standart::rpm)
    {
        lenght = 4; // Total OBD-II data length
        // In the CAN bytes: multiply RPM by 4 to give 0.25 precision
        uint16_t rpmData = static_cast<uint16_t>(carStatus.rpm * 4);
        lowByte = static_cast<uint8_t>(rpmData);
        highByte = static_cast<uint8_t>(rpmData >> 8);
    }
    else if (currentSensor == PID_standart::speed)
    {
        lenght = 3;
        highByte = static_cast<uint8_t>(carStatus.speed);
    }
    else if (currentSensor == PID_standart::coolantTemp)
    {
        lenght = 3;
        // In the CAN bytes: add 40 offset to support negative temperatures
        highByte = static_cast<uint8_t>(carStatus.coolantTemp + 40);
    }

    // Populate standard OBD-II CAN payload
    canFrame.data[0] = lenght;
    canFrame.data[1] = static_cast<uint8_t>(currentSensor);
    canFrame.data[2] = static_cast<uint8_t>(highByte);
    canFrame.data[3] = static_cast<uint8_t>(lowByte);

    return canFrame;
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

    std::cout << "=== Virtual Car Simulator ===" << std::endl;

    // Initial vehicle telemetry state
    CarStatus carStatus = {800.0, 0.0, 20.0};

    // Initial state machine state: starting at idle
    CarState carState = CarState::IDLE;
    int counter = 0;

    // Main vehicle physics loop
    while (running)
    {
        counter++;

        switch (carState)
        {
        case CarState::IDLE:
            // Natural engine vibration at idle (around 800-822 RPM)
            carStatus.rpm = 800.0 + (counter % 23);
            carStatus.speed = 0.0;

            // Transition to accelerating after 25 ticks (approx 2.5 seconds)
            if (counter % 25 == 0)
            {
                carState = CarState::ACCELERATING;
            }
            break;

        case CarState::ACCELERATING:
            // Non-linear acceleration dynamic using non-matching modulo factors
            carStatus.rpm += 8.0 * (counter % 15) + 10.0;
            carStatus.speed += 0.7 * (counter % 7) + 0.5;
            carStatus.coolantTemp += 0.2;

            // Engine thermostat regulation: cap coolant temperature at 90 C
            if (carStatus.coolantTemp > 90.0)
            {
                carStatus.coolantTemp = 90.0;
            }

            // Redline limit: trigger deceleration when exceeding 4200 RPM
            if (carStatus.rpm > 4200.0)
            {
                carState = CarState::DECELERATING;
            }
            break;

        case CarState::DECELERATING:
            // Deceleration physics: asymmetric braking rate
            carStatus.rpm -= 25.0 * (counter % 11) + 20.0;
            carStatus.speed -= 0.8 * (counter % 5) + 0.5;

            // Stop condition: when wheels stop or engine reaches idle, settle to IDLE
            if (carStatus.rpm <= 800.0 || carStatus.speed <= 0.0)
            {
                carStatus.rpm = 800.0;
                carStatus.speed = 0.0;
                carState = CarState::IDLE;
            }
            break;
        }

        // Encode and send telemetry for each sensor in the list
        for (PID_standart sensor : sensors)
        {
            CanFrame sensorFrame = encode_can(sensor, carStatus);
            sendto(sock, sensorFrame.data, sizeof(sensorFrame.data), 0, (struct sockaddr *)&addr, sizeof(addr));
        }

        // Live dashboard telemetry printout overwriting current line (\r)
        std::cout << "\r[Vehicle Running] "
                  << "RPM: " << static_cast<int>(carStatus.rpm) << "   "
                  << " | Speed: " << static_cast<int>(carStatus.speed) << " km/h   "
                  << " | Temp: " << static_cast<int>(carStatus.coolantTemp) << " C   "
                  << std::flush;

        // 10 Hz physics update rate (100 milliseconds)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sock);

    return 0;
}