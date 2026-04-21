#include "motors.h"
#include "config.h"
#include "Arduino.h"

namespace
{
    struct Motor
    {
        int stepPin;
        int dirPin;
        bool invertDir;
    };

    constexpr Motor left = {cfg::MOTOR_L_STEP_PIN, cfg::MOTOR_L_DIR_PIN, cfg::MOTOR_L_INVERT_DIR};
    constexpr Motor right = {cfg::MOTOR_R_STEP_PIN, cfg::MOTOR_R_DIR_PIN, cfg::MOTOR_R_INVERT_DIR};

    // Runtime state
    MotorState state;
    bool initialized = false;

    uint32_t lastStepUsL = 0;
    uint32_t lastStepUsR = 0;

    constexpr uint32_t MICROS_PER_SEC = 1000000UL;
    constexpr float MAX_SPEED = 1000000.0f / cfg::MOTOR_MIN_STEP_INVERVAL_US;

    // Helper functions
    float clamp(float speed)
    {
        if (speed > MAX_SPEED)
            return MAX_SPEED;
        if (speed < -MAX_SPEED)
            return -MAX_SPEED;
        return speed;
    };

    void applyDir(const Motor &motor, float speed)
    {
        if (fabsf(speed) < cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return;

        bool forward = (speed > 0.0f) != motor.invertDir;
        digitalWrite(motor.dirPin, forward ? HIGH : LOW);
    };

    MotorState::Direction toDir(float speed)
    {
        if (speed > cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return MotorState::Direction::FORWARD;
        if (speed < -cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return MotorState::Direction::BACKWARD;
        return MotorState::Direction::STOP;
    };

    void syncState()
    {
        state.dirLeft = toDir(state.speedLeft);
        state.dirRight = toDir(state.speedRight);

        const bool running =
            (fabsf(state.speedLeft) > cfg::MOTOR_DEADBAND_STEPS_PER_SEC ||
             fabsf(state.speedRight) > cfg::MOTOR_DEADBAND_STEPS_PER_SEC);

        state.status = running ? MotorState::Status::RUNNING
                               : MotorState::Status::IDLE;
    };

    void stepMotor(const Motor &motor, float speed, uint32_t &lastStepUs)
    {
        float absSpeed = fabsf(speed);

        if (absSpeed < cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return;

        // Convert steps/sec to microseconds between pulses
        const uint32_t interval = static_cast<uint32_t>(MICROS_PER_SEC / absSpeed);
        uint32_t now = micros();

        // Not enough time has passed since last pulse — wait
        if ((now - lastStepUs) < interval)
            return;

        // Fire one step pulse — HIGH → hold → LOW
        digitalWrite(motor.stepPin, HIGH);
        delayMicroseconds(cfg::MOTOR_STEP_PULSE_WIDTH_US);
        digitalWrite(motor.stepPin, LOW);

        // Records when this pulse fired so next call knows when to fire again
        lastStepUs = now;
    }
};

const MotorState &motorsGetState()
{
    return state;
};

void motorsInit()
{
    pinMode(left.stepPin, OUTPUT);
    pinMode(left.dirPin, OUTPUT);
    pinMode(right.stepPin, OUTPUT);
    pinMode(right.dirPin, OUTPUT);

    digitalWrite(left.stepPin, LOW);
    digitalWrite(left.dirPin, LOW);
    digitalWrite(right.stepPin, LOW);
    digitalWrite(right.dirPin, LOW);

    initialized = true;
    Serial.println("[MOTORS] Init");
};

void motorsStop()
{
    if (!initialized)
        return;

    state.speedLeft = 0.0f;
    state.speedRight = 0.0f;
    lastStepUsL = 0;
    lastStepUsR = 0;

    digitalWrite(left.stepPin, LOW);
    digitalWrite(right.stepPin, LOW);

    syncState();
};

void motorsSetSpeed(float sl, float sr)
{
    if (!initialized)
        return;

    state.speedLeft = clamp(sl);
    state.speedRight = clamp(sr);

    applyDir(left, state.speedLeft);
    applyDir(right, state.speedRight);
    syncState();
}

void motorsUpdate()
{
    if (!initialized)
        return;

    stepMotor(left, state.speedLeft, lastStepUsL);
    stepMotor(right, state.speedRight, lastStepUsR);
}