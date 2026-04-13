#pragma once

#include <Arduino.h>
#include <Wire.h>

#define IMU_WIRE_PORT Wire

namespace cfg
{
    // --- Serial ---
    constexpr uint32_t SERIAL_BAUD = 115200;
    constexpr uint32_t SERIAL_BOOT_DELAY = 3000; // ms - 3s

    // --- IMU (ICM-20948) ---
    // I2C pins
    constexpr int IMU_SDA_PIN = 21;
    constexpr int IMU_SCL_PIN = 22;

    constexpr uint32_t IMU_I2C_CLOCK = 400000; // Hz - 400kHz (fast mode)

    // Initialisation retries
    constexpr uint32_t IMU_INIT_MAX_TRIES = 5;
    constexpr uint16_t IMU_INIT_RETRY_MS = 500;

    // I2C address: AD0 floating = 0x69 (AD0_VAL 1)
    //              AD0 to GND   = 0x68 (AD0_VAL 0)
    constexpr uint8_t IMU_AD0_VAL = 1;
    // DMP output rate: 0 = maximum (~55 Hz)
    constexpr uint8_t IMU_DMP_ODR = 0;

    // Mounting orientation correction
    constexpr bool IMU_MOUNTED_UPSIDE_DOWN = true;

    // BIAS
    constexpr uint32_t IMU_BIAS_SAVE_INTERVAL_MS = 120000;

    constexpr uint16_t IMU_EEPROM_SIZE = 128; // bytes
    constexpr uint8_t IMU_EEPROM_ADDR = 0;

    // --- Stepper motors (DRV8825) ---
    constexpr int MOTOR_L_STEP_PIN = 25;
    constexpr int MOTOR_L_DIR_PIN = 26;
    constexpr int MOTOR_R_STEP_PIN = 27;
    constexpr int MOTOR_R_DIR_PIN = 14;
}
