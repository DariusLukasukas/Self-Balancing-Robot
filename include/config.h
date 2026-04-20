#pragma once

#include <Arduino.h>
#include <Wire.h>

#define IMU_WIRE_PORT Wire

#ifndef WIFI_SSID_BUILD
#define WIFI_SSID_BUILD ""
#endif
#ifndef WIFI_PASS_BUILD
#define WIFI_PASS_BUILD ""
#endif

namespace cfg
{
    // === Serial ===
    constexpr uint32_t SERIAL_BAUD = 115200;
    constexpr uint32_t SERIAL_BOOT_DELAY = 3000; // ms - 3s

    // === WiFi Credentials ===
    inline constexpr const char *WIFI_SSID = WIFI_SSID_BUILD;
    inline constexpr const char *WIFI_PASS = WIFI_PASS_BUILD;

    // === Web Server ===
    constexpr int WEB_SERVER_PORT = 8000;

    // === IMU (ICM-20948) ===
    // I2C pins
    constexpr int IMU_SDA_PIN = 21;
    constexpr int IMU_SCL_PIN = 22;

    /*
     * I2C Serial Clock Speed (SCL)
     * Options: 100 kHz (standard), 400kHz (fast mode),
     */
    constexpr uint32_t IMU_I2C_CLOCK = 400000;

    // Initialisation retries
    constexpr uint32_t IMU_INIT_MAX_TRIES = 5;
    constexpr uint16_t IMU_INIT_RETRY_MS = 500;

    /*
     * I2C address:
     * AD0 floating = 0x69 (AD0_VAL 1)
     * AD0 to GND   = 0x68 (AD0_VAL 0)
     */
    constexpr uint8_t IMU_AD0_VAL = 1;
    // DMP output rate: 0 = maximum (~55 Hz)
    constexpr uint8_t IMU_DMP_ODR = 0;

    // Mounting orientation correction
    constexpr bool IMU_MOUNTED_UPSIDE_DOWN = true;

    // === BIAS ===
    constexpr uint32_t IMU_BIAS_CALIBRATION_MS = 120000; // 2 minutes

    // === EEPROM ===
    constexpr uint16_t IMU_EEPROM_SIZE = 128; // bytes
    constexpr uint8_t IMU_EEPROM_ADDR = 0;

    // === Drive module (DRV8825) ===
    constexpr int MOTOR_L_STEP_PIN = 19;
    constexpr int MOTOR_L_DIR_PIN = 18;
    constexpr int MOTOR_R_STEP_PIN = 32;
    constexpr int MOTOR_R_DIR_PIN = 33;

    /*
     * Invert motor direction
     * Options: true = reverse, false = normal;
     */
    constexpr bool MOTOR_L_INVERT_DIR = false;
    constexpr bool MOTOR_R_INVERT_DIR = false;

    /*
     * Microstepping
     * Options: 1 (full step), 2 (1/2), 4 (1/4), 8 (1/8), 16 (1/16), 32 (1/32)
     */
    constexpr int MOTOR_MICROSTEPS_MODE = 8;

    /*
     * Motor Pulse Timing
     */
    constexpr int MOTOR_STEP_PULSE_WIDTH_US = 2;    // microseconds (min 1.9us for DRV8825)
    constexpr int MOTOR_MIN_STEP_INVERVAL_US = 200; // microseconds (max ~5000 steps/sec)

    // === Stepper motors (NEMA 17HE15-1504S) ===
    constexpr int MOTOR_STEPS_PER_REV = 200;                                          // Total steps per revolution
    constexpr int MOTOR_USTEPS_PER_REV = MOTOR_STEPS_PER_REV * MOTOR_MICROSTEPS_MODE; // Total microsteps per motor shaft revolution
    constexpr float MOTOR_DEG_PER_USTEP = 360.0f / MOTOR_USTEPS_PER_REV;              // Degrees moved per one microstep at the motor shaft

    // === PID ===
    constexpr float PID_TARGET_ANGLE_DEG = 0.0f;

    constexpr float PID_KP = 25.0f;
    constexpr float PID_KI = 0.0f;
    constexpr float PID_KD = 1.2f;

    // Minimum speed command. Safety guard against division by zero and prevents jitter at near-standstill. Tunable alongside PID
    constexpr float MOTOR_DEADBAND_STEPS_PER_SEC = 2.0f;
}
