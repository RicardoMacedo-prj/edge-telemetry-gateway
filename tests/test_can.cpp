#include <iostream>
#include <cassert>
#include <cmath>
#include "decoder.hpp"
#include "car.hpp"

void test_decode_rpm()
{
    CanFrame frame;
    frame.data[1] = static_cast<uint8_t>(PID_standart::rpm);
    frame.data[2] = 0x27;
    frame.data[3] = 0x11;

    auto [timestamp, rpm, pid] = decode_can(frame);

    assert(rpm == 2500.25);
}

void test_decode_speed()
{
    CanFrame frame;
    frame.data[1] = static_cast<uint8_t>(PID_standart::speed);
    frame.data[2] = 85;

    auto [timestamp, speed, pid] = decode_can(frame);
    assert(speed == 85.0);
}

void test_decode_coolante_positive()
{
    CanFrame frame;
    frame.data[1] = static_cast<uint8_t>(PID_standart::coolantTemp);
    frame.data[2] = 130;

    auto [timestamp, temp, pid] = decode_can(frame);
    assert(temp == 90.0);
}

void test_decode_coolante_negative()
{
    CanFrame frame;
    frame.data[1] = static_cast<uint8_t>(PID_standart::coolantTemp);
    frame.data[2] = 30;

    auto [timestamp, temp, pid] = decode_can(frame);
    assert(temp == -10.0);
}

int main()
{
    test_decode_rpm();
    test_decode_speed();
    test_decode_coolante_positive();
    test_decode_coolante_negative();

    return 0;
}
