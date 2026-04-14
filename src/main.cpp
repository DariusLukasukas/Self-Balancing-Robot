#include <Arduino.h>
#include "config.h"
#include "imu.h"

static Angles angles;

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

    Serial.println();
}

void loop()
{
    if (imuRead(angles))
    {
        imuPrintAngles(angles);

        // Later:
        // float pidOutput = pidCompute(angles.pitch);
        // motorsSetSpeed(pidOutput);
    }

    imuUpdateBias();
    delay(10);
}