#pragma once

struct MotorState
{
    enum class Direction
    {
        FORWARD,
        BACKWARD,
        STOP
    };
    enum class Status
    {
        IDLE,
        RUNNING
    };
    float speedLeft = 0.0f;
    float speedRight = 0.0f;
    Direction dirLeft = Direction::STOP;
    Direction dirRight = Direction::STOP;
    Status status = Status::IDLE;
};

void motorsInit();
void motorsSetSpeed(float sl, float sr);
void motorsStop();

const MotorState &motorsGetState();