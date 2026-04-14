#pragma once

#include "types.h"

/*
=== Public Interface for the IMU module ===
Hardware: SparkFun ICM-20948 9DoF
DMP Mode: 6DoF (accel + gyro only - magnetometer disabled)
Bias: DMP biases saved once to EEPROM after calibration,
then restored automatically on every subsequent boot.
*/

// Initialise I2C, connect ICM-20948, configure DMP, restore biases if saved.
// Returns true if fully ready, false on any failure (Serial output).
bool imuInit();

// Save biases to EEPROM once after the calibration period has elapsed.
// Call every loop() — returns immediately if not time yet or already saved.
void imuUpdateBias();

// Read latest orientation from DMP FIFO.
// Returns true and populates out when new data is available.
bool imuRead(Angles &out);

// Returns true if valid biases were restored from EEPROM at boot.
bool imuBiasesRestored();

// Print formatted angles to Serial. Debug helper.
void imuPrintAngles(const Angles &a);