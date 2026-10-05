#pragma once

// Car status structure
struct CarStatus
{
    double rpm;
    double speed;
    double coolantTemp;
};

// Car state enumeration
enum class CarState
{
    IDLE,
    ACCELERATING,
    DECELERATING
};

// Standard OBD-II Parameter IDs (SAE J1979 standard)
enum class PID_standart
{
    rpm = 0x0C,        // Engine RPM (2 bytes response)
    speed = 0x0D,      // Vehicle Speed (1 byte response)
    coolantTemp = 0x05 // Engine Coolant Temperature (-40 C offset)
};

// Standard 8-byte CAN Bus message payload
struct CanFrame
{
    uint8_t data[8]{0};
};