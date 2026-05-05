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
    // inline constexpr const char *WIFI_SSID = WIFI_SSID_BUILD;
    // inline constexpr const char *WIFI_PASS = WIFI_PASS_BUILD;

    // === WiFi Credentials ===
    // TODO: Replace temporary WiFi credentials with university sensor network credentials when received
    inline constexpr const char *WIFI_SSID = "DiPhone";
    inline constexpr const char *WIFI_PASS = "duxxyw-Bubcas-5tuxfo";

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
    // DMP output rate: 0 = maximum (~225 Hz)
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
    constexpr int MOTOR_MICROSTEPS_MODE = 16;

    /*
     * Motor Pulse Timing
     */
    constexpr int MOTOR_STEP_PULSE_WIDTH_US = 2;    // microseconds (min 1.9us for DRV8825)
    constexpr int MOTOR_MIN_STEP_INVERVAL_US = 200; // microseconds (max ~5000 steps/sec)

    // Maximum signed motor command (steps/s). Single source of truth shared by motor
    // clamping AND the PID output bounds so they cannot drift apart silently.
    constexpr float MOTOR_MAX_STEPS_PER_SEC =
        1.0e6f / static_cast<float>(MOTOR_MIN_STEP_INVERVAL_US);

    // === Stepper motors (NEMA 17HE15-1504S) ===
    constexpr int MOTOR_STEPS_PER_REV = 200; // Total steps per revolution

    // === PID ===
    constexpr float PID_TARGET_ANGLE_DEG = 2.0f;

    constexpr float PID_KP = 150.0f;
    constexpr float PID_KI = 0.5f;
    constexpr float PID_KD = 1.0f;

    // D-term low-pass filter coefficient (single-pole IIR, 0..1).
    // Lower = more smoothing (and more lag); 1.0 disables filtering.
    constexpr float PID_D_FILTER_ALPHA = 0.25f;

    // === Control / Safety ===
    // Start in MANUAL (motors stopped) and require user to engage BALANCE via web UI.
    constexpr bool CONTROL_START_IN_BALANCE = false;

    // Roll angle normalization: measured_roll = wrapTo180((invert ? -roll : roll) + offset)
    // DMP roll is often ~±180° when upright; offset -180 maps upright to ~0° for balancing.
    constexpr float PID_ROLL_OFFSET_DEG = -180.0f;
    constexpr bool PID_ROLL_INVERT = false;

    // If robot is beyond this tilt, cut motor output and reset integrator.
    // Past ~40° the wheels can't recover anyway; any larger value just spins motors
    // while the robot is on the floor.
    constexpr float PID_MAX_TILT_DEG = 40.0f;

    // Manual + PID output slew-rate limit (helps prevent stepper stalls/grinding).
    // Units: (steps/s) per second.
    constexpr float MOTOR_SLEW_RATE_STEPS_PER_SEC2 = 8000.0f;

    // Minimum speed command. Safety guard against division by zero and prevents jitter at near-standstill. Tunable alongside PID
    constexpr float MOTOR_DEADBAND_STEPS_PER_SEC = 2.0f;
}
