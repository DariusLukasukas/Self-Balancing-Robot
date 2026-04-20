#pragma once

#include <stdint.h>

struct Angles
{
    float pitch = 0.0f; // Not used
    float roll = 0.0f;  // Forward/backward tilt - primary balancing axis (deg)
    float yaw = 0.0f;   // Left/right tilt (deg)
};

struct BiasStore
{
    int32_t header = 0x42;
    int32_t biasGyroX = 0;
    int32_t biasGyroY = 0;
    int32_t biasGyroZ = 0;
    int32_t biasAccelX = 0;
    int32_t biasAccelY = 0;
    int32_t biasAccelZ = 0;
    int32_t biasCPassX = 0;
    int32_t biasCPassY = 0;
    int32_t biasCPassZ = 0;
    int32_t sum = 0;

    void updateSum()
    {
        sum = computeSum();
    }

    bool isValid() const
    {
        if (header != 0x42)
            return false;
        int32_t computed = computeSum();
        return sum == computed;
    }

private:
    int32_t computeSum() const
    {
        return header +
               biasGyroX + biasGyroY + biasGyroZ +
               biasAccelX + biasAccelY + biasAccelZ +
               biasCPassX + biasCPassY + biasCPassZ;
    }
};
