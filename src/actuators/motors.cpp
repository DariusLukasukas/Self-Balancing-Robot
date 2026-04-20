#include "motors.h"
#include "config.h"
#include "Arduino.h"

namespace
{
    bool initialized = false;
    float targetSpeed = 0.0f;                                                // us/sec; negative = backward
    uint32_t lastStepUs = 0;                                                 // micros() timestamp of the last step pulse
    constexpr float maxSpeed = 1000000.0f / cfg::MOTOR_MIN_STEP_INVERVAL_US; // = ~5000steps/sec
}

void motorsInit()
{
    pinMode(cfg::MOTOR_L_STEP_PIN, OUTPUT);
    pinMode(cfg::MOTOR_L_DIR_PIN, OUTPUT);
    pinMode(cfg::MOTOR_R_STEP_PIN, OUTPUT);
    pinMode(cfg::MOTOR_R_DIR_PIN, OUTPUT);

    digitalWrite(cfg::MOTOR_L_STEP_PIN, LOW);
    digitalWrite(cfg::MOTOR_L_DIR_PIN, LOW);
    digitalWrite(cfg::MOTOR_R_STEP_PIN, LOW);
    digitalWrite(cfg::MOTOR_R_DIR_PIN, LOW);

    Serial.println("===MOTORS INIT===");
    initialized = true;
}

void motorsStop()
{
    targetSpeed = 0.0f;
    digitalWrite(cfg::MOTOR_L_STEP_PIN, LOW);
    digitalWrite(cfg::MOTOR_R_STEP_PIN, LOW);
}

static void setDirection(float speed)
{
    const bool forward = (speed >= 0.0f); // -speed = backward; +speed = forward

    const bool leftForward = forward != cfg::MOTOR_L_INVERT_DIR;
    const bool rightForward = forward != cfg::MOTOR_R_INVERT_DIR;

    // HIGH = forward, LOW = backward
    digitalWrite(cfg::MOTOR_L_DIR_PIN, leftForward ? HIGH : LOW);
    digitalWrite(cfg::MOTOR_R_DIR_PIN, rightForward ? HIGH : LOW);
}

void motorsSetSpeed(float speed)
{
    if (speed > maxSpeed)
        speed = maxSpeed;
    if (speed < -maxSpeed)
        speed = -maxSpeed;

    // Update direction immediately on sign change
    if ((speed >= 0.0f) != (targetSpeed >= 0.0f))
        setDirection(speed);

    targetSpeed = speed;
}

void motorsUpdate()
{
    if (!initialized)
        return;

    const float absSpeed = fabsf(targetSpeed);

    // Dead-band: ignore tiny speed commands - avoids jitter at standstill
    if (absSpeed < cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
        return;

    // How many microsteps should elapse between each step pulse?
    const uint32_t interval_us = static_cast<uint32_t>(1000000.0f / absSpeed);

    const uint32_t now = micros();

    // Not enough time has passed since last step - do nothing
    if ((now - lastStepUs) < interval_us)
        return;

    setDirection(targetSpeed);

    // Rising edge - both motors start step
    digitalWrite(cfg::MOTOR_L_STEP_PIN, HIGH);
    digitalWrite(cfg::MOTOR_R_STEP_PIN, HIGH);

    // Hold HIGH for the minimum pulse width the DRV8825 requires (≥1.9µs)
    delayMicroseconds(cfg::MOTOR_STEP_PULSE_WIDTH_US);

    // Falling edge
    digitalWrite(cfg::MOTOR_L_STEP_PIN, LOW);
    digitalWrite(cfg::MOTOR_R_STEP_PIN, LOW);

    lastStepUs = now;
}

void motorsTest()
{
    if (!initialized)
    {
        Serial.println("[MOTORS] motorsTest() called before init!");
        return;
    }

    Serial.println("[MOTORS] Test start — ramp up, hold, ramp down");

    // Ramp up forward
    for (int speed = 200; speed <= 2000; speed += 200)
    {
        motorsSetSpeed(static_cast<float>(speed));
        const uint32_t endMs = millis() + 500;
        while (millis() < endMs)
            motorsUpdate();
        Serial.printf("[MOTORS] Speed: %d usteps/sec\n", speed);
    }

    // Hold at cruise speed
    const uint32_t cruiseEnd = millis() + 2000;
    while (millis() < cruiseEnd)
        motorsUpdate();

    // Ramp back down
    for (int speed = 2000; speed >= 0; speed -= 200)
    {
        motorsSetSpeed(static_cast<float>(speed));
        const uint32_t endMs = millis() + 500;
        while (millis() < endMs)
            motorsUpdate();
    }

    motorsStop();
    Serial.println("[MOTORS] Test complete");
}