#include <iostream>
#include <chrono>
#include <thread>
#include "car.hpp"

enum class PID_standart
{
    rpm = 0x0C,
    speed = 0x0D,
    coolantTemp = 0x05
};

struct CanFrame
{
    uint8_t data[8]{0};
};

PID_standart sensors[] = {PID_standart::rpm, PID_standart::speed, PID_standart::coolantTemp};

CanFrame encode_can(PID_standart currentSensor, CarStatus carStatus)
{
    CanFrame canFrame;
    uint8_t lenght = 0;
    uint8_t highByte = 0;
    uint8_t lowByte = 0;

    if (currentSensor == PID_standart::rpm)
    {
        lenght = 4;
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
        highByte = static_cast<uint8_t>(carStatus.coolantTemp + 40);
    }

    canFrame.data[0] = lenght;
    canFrame.data[1] = static_cast<uint8_t>(currentSensor);
    canFrame.data[2] = static_cast<uint8_t>(highByte);
    canFrame.data[3] = static_cast<uint8_t>(lowByte);

    return canFrame;
}

void decode_can(CanFrame canFrame)
{
    if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::rpm))
    {
        uint16_t rpmData = (static_cast<uint16_t>(canFrame.data[2]) << 8) | static_cast<uint16_t>(canFrame.data[3]);
        double rpm = static_cast<double>(rpmData) / 4.0;
        std::cout << "Decoded RPM: " << rpm << std::endl;
    }

    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::speed))
    {
        double speed = static_cast<double>(canFrame.data[2]);
        std::cout << "Decoded Speed: " << speed << std::endl;
    }

    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
    {
        double coolantTemp = static_cast<double>(canFrame.data[2]) - 40.0;
        std::cout << "Decoded Coolant Temperature: " << coolantTemp << std::endl;
    }
}

int main()
{
    std::cout << "=== Testing CAN Encoder & Decoder ===" << std::endl;

    CarStatus car{2500.0, 85.0, 88.0}; // RPM: 2500, Speed: 85, Temp: 88

    // 1. Testa RPM
    CanFrame f1 = encode_can(PID_standart::rpm, car);
    decode_can(f1);

    // 2. Testa Velocidade
    CanFrame f2 = encode_can(PID_standart::speed, car);
    decode_can(f2);

    // 3. Testa Temperatura
    CanFrame f3 = encode_can(PID_standart::coolantTemp, car);
    decode_can(f3);

    return 0;
}