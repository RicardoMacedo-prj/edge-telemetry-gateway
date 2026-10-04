#pragma once

struct CarStatus {
    double rpm;
    double speed;
    double coolantTemp;
};

enum class CarState {
    IDLE,
    ACCELERATING,
    DECELERATING
};