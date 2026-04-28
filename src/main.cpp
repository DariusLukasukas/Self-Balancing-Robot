#include <Arduino.h>
#include "config.h"
#include "imu.h"
#include "motors.h"
#include "balance.h"
#include "web/web_server.h"

namespace
{
    Angles angles;
    uint32_t lastImuUs = 0;   // timestamp of the last good IMU sample (microseconds)
    uint32_t lastImuMs = 0;   // same in millis for stale-detection

    // Effective control-loop frequency (IMU samples per second), updated once per second.
    volatile float imuHz = 0.0f;
    uint32_t imuTickCount = 0;
    uint32_t imuHzWindowMs = 0;
}

float mainGetImuHz()
{
    return imuHz;
}

void setup()
{
    Serial.begin(cfg::SERIAL_BAUD);
    delay(cfg::SERIAL_BOOT_DELAY);

    Serial.println("=== Self-Balancing Robot ===");

    if (!imuInit())
    {
        Serial.println("[MAIN] IMU init failed. Halting.");
        while (true)
            delay(1000);
    }

    if (imuBiasesRestored())
        Serial.println("[MAIN] Saved calibration restored.");
    else
    {
        Serial.println("[MAIN] No saved calibration found.");
        Serial.println("[MAIN] Move sensor for ~2 minutes so bias can be saved.");
    }

    motorsInit();
    balanceInit();
    webServerInit();

    xTaskCreatePinnedToCore(
        [](void*) {
            for (;;) {
                webServerUpdate();
                vTaskDelay(pdMS_TO_TICKS(1)); // yield 1ms between polls
            }
        },
        "webTask",
        4096,           // stack size
        nullptr,
        1,              // priority 
        nullptr,
        0               // Core 0 
    );

    lastImuUs = micros();
    lastImuMs = millis();
    imuHzWindowMs = millis();
}

void loop()
{
    const bool hasNewImu = imuRead(angles);

    imuUpdateBias();

    if (hasNewImu)
    {
        // dt is the time since the previous IMU sample
        const uint32_t now = micros();
        float dt = (now - lastImuUs) * 1e-6f;
        lastImuUs = now;
        lastImuMs = millis();

        // Guard against very first sample and timer overflows.
        if (dt <= 0.0f || dt > 0.5f)
            dt = 0.02f; // safe fallback ~50 Hz

        balanceUpdate(angles, dt);

        imuTickCount++;
        const uint32_t nowMs = millis();
        const uint32_t windowMs = nowMs - imuHzWindowMs;
        if (windowMs >= 1000)
        {
            imuHz = (imuTickCount * 1000.0f) / static_cast<float>(windowMs);
            imuTickCount = 0;
            imuHzWindowMs = nowMs;
        }
    }
    else
    {
        // No new IMU packet this loop, MANUAL still needs to apply commands.
        if (balanceGetMode() == ControlMode::MANUAL)
            balanceUpdate(angles, 0.0f);

        // Freeze balance output if IMU has been truly stale for too long.
        if (balanceGetMode() == ControlMode::BALANCE)
        {
            if ((millis() - lastImuMs) > 200)
                balanceFreezeOutput();
        }
    }
}