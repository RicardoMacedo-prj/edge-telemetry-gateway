#include "decoder.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <atomic>
#include <mosquitto.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include "car.hpp"

/**
 * @brief Decodes an 8-byte OBD-II CAN frame and returns timestamped metric.
 */
std::pair<long long, double> decode_can(CanFrame canFrame)
{
    // Generate timestamp in milliseconds since Unix epoch
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Inspect byte 1 to determine the PID sensor type
    if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::rpm))
    {
        // Reconstruct 16-bit integer from high byte (data[2]) and low byte (data[3])
        // Multiply by 0.25 (divide by 4.0) per SAE J1979 standard
        uint16_t rpmData = (static_cast<uint16_t>(canFrame.data[2]) << 8) | static_cast<uint16_t>(canFrame.data[3]);
        double rpm = static_cast<double>(rpmData) / 4.0;
        
        return {timestamp, rpm};
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::speed))
    {
        // Speed is represented as a direct 1-byte value (0 to 255 km/h)
        double speed = static_cast<double>(canFrame.data[2]);

        return {timestamp, speed};
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
    {
        // Coolant temperature uses a -40 offset to support negative temperatures (-40 C to +215 C)
        double coolantTemp = static_cast<double>(canFrame.data[2]) - 40.0;

        return {timestamp, coolantTemp};
    }

    // Default fallback if an unrecognized PID is received
    return {timestamp, 0.0};
}
