#include <iostream>
#include <chrono>
#include <thread>

struct CarStatus
{
    double rpm;
    double speed;
    double coolantTemp;
};

enum class CarState
{
    IDLE,
    ACCELERATING,
    DECELERATING
};

int main()
{
    std::cout << "=== Virtual Car Simulator ===" << std::endl;

    CarStatus carStatus = {800.0, 0.0, 20.0}; // Initial values

    CarState carState = CarState::IDLE;
    int counter = 0;
    while (true)
    {
        counter++;

        switch (carState)
        {
        case CarState::IDLE:
            carStatus.rpm = 800.0 + (counter % 23);
            carStatus.speed = 0.0;

            if (counter % 25 == 0)
            {
                carState = CarState::ACCELERATING;
            }

            break;

        case CarState::ACCELERATING:
            carStatus.rpm += 8.0 * (counter % 15) + 10.0;
            carStatus.speed += 0.7 * (counter % 7) + 0.5;
            carStatus.coolantTemp += 0.2;

            if (carStatus.coolantTemp > 90.0)
            {
                carStatus.coolantTemp = 90.0;
            }

            if (carStatus.rpm > 4200.0)
            {
                carState = CarState::DECELERATING;
            }

            break;

        case CarState::DECELERATING:
            carStatus.rpm -= 25.0 * (counter % 11) + 20.0;
            carStatus.speed -= 0.8 * (counter % 5) + 0.5;

            if (carStatus.rpm <= 800.0 || carStatus.speed <= 0.0)
            {
                carStatus.rpm = 800.0;
                carStatus.speed = 0.0;
                carState = CarState::IDLE;
            }

            break;
        }

        std::cout << "\r[Vehicle Running] "
                  << "RPM: " << static_cast<int>(carStatus.rpm) << "   "
                  << " | Speed: " << static_cast<int>(carStatus.speed) << " km/h   "
                  << " | Temp: " << static_cast<int>(carStatus.coolantTemp) << " C   "
                  << std::flush;

        // 10 hz update rate
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}