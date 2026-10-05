#include <iostream>
#include <chrono>
#include <thread>
#include "car.hpp"

int main()
{
    std::cout << "=== Virtual Car Simulator ===" << std::endl;

    // Initial vehicle telemetry state
    CarStatus carStatus = {800.0, 0.0, 20.0}; 

    // Initial state machine state: starting at idle
    CarState carState = CarState::IDLE;
    int counter = 0;

    // Main vehicle physics loop
    while (true)
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

        // Live dashboard telemetry printout overwriting current line (\r)
        std::cout << "\r[Vehicle Running] "
                  << "RPM: " << static_cast<int>(carStatus.rpm) << "   "
                  << " | Speed: " << static_cast<int>(carStatus.speed) << " km/h   "
                  << " | Temp: " << static_cast<int>(carStatus.coolantTemp) << " C   "
                  << std::flush;

        // 10 Hz physics update rate (100 milliseconds)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}