#pragma once

#include "config.h"
#include "types.h"
#include "ICM_20948.h"
#include <EEPROM.h>

static ICM_20948_I2C myICM;
static bool biasRestores = false;
static bool biasSaved = false;
static uint32_t startTimeMs = 0;

static bool ConnectSensor()
{
    for (int32_t attempt = 1; attempt <= cfg::IMU_INIT_MAX_TRIES; attempt++)
    {
        myICM.begin(IMU_WIRE_PORT, cfg::IMU_AD0_VAL);
        Serial.printf("[IMU] Connect attempt:" + attempt);

        if (myICM.status == ICM_20948_Stat_Ok)
            return true;
        delay(cfg::IMU_INIT_RETRY_MS);
    }
    return false;
}

static bool configureDMP()
{
    bool success = true;

    success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);

    // Enable the DMP 6DoF Game Rotation Vector - accel + gyro, NO magnetometer.
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);

    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, cfg::IMU_DMP_ODR) == ICM_20948_Stat_Ok);

    // Enable the FIFO
    success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
    // Enable the DMP
    success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
    // Reset DMP
    success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
    // Reset FIFO
    success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

    return success;
}

static void printBiases(const BiasStore &store)
{
    Serial.printf("Gyro  X:%d Y:%d Z:%d\n",
                  store.biasGyroX, store.biasGyroY, store.biasGyroZ);
    Serial.printf("Accel X:%d Y:%d Z:%d\n",
                  store.biasAccelX, store.biasAccelY, store.biasAccelZ);
}

static bool restoreBias()
{
    BiasStore store;

    EEPROM.get(cfg::IMU_EEPROM_ADDR, store);

    if (!store.isValid())
    {
        Serial.println("[IMU] WARNING: No valid bias data in EEPROM");
        return false;
    }

    bool success = true;

    success &= (myICM.setBiasGyroX(store.biasGyroX) == ICM_20948_Stat_Ok);
    success &= (myICM.setBiasGyroY(store.biasGyroY) == ICM_20948_Stat_Ok);
    success &= (myICM.setBiasGyroZ(store.biasGyroZ) == ICM_20948_Stat_Ok);
    success &= (myICM.setBiasAccelX(store.biasAccelX) == ICM_20948_Stat_Ok);
    success &= (myICM.setBiasAccelY(store.biasAccelY) == ICM_20948_Stat_Ok);
    success &= (myICM.setBiasAccelZ(store.biasAccelZ) == ICM_20948_Stat_Ok);

    if (success)
    {
        Serial.println("[IMU] SUCCESS: Bias restored");
        printBiases(store);
    }
    else
    {
        Serial.println("[IMU] ERROR: Bias restore failed!");
    }

    return success;
}

static bool saveBiases()
{
    BiasStore store;

    bool success = true;
    success &= (myICM.getBiasGyroX(&store.biasGyroX) == ICM_20948_Stat_Ok);
    success &= (myICM.getBiasGyroY(&store.biasGyroY) == ICM_20948_Stat_Ok);
    success &= (myICM.getBiasGyroZ(&store.biasGyroZ) == ICM_20948_Stat_Ok);
    success &= (myICM.getBiasAccelX(&store.biasAccelX) == ICM_20948_Stat_Ok);
    success &= (myICM.getBiasAccelY(&store.biasAccelY) == ICM_20948_Stat_Ok);
    success &= (myICM.getBiasAccelZ(&store.biasAccelZ) == ICM_20948_Stat_Ok);

    if (!success)
    {
        Serial.println("[IMU] ERROR: Saving bias failed!");
        return false;
    }

    store.updateSum();
    EEPROM.put(cfg::IMU_EEPROM_ADDR, store);
    EEPROM.commit();

    BiasStore verify;
    EEPROM.get(cfg::IMU_EEPROM_ADDR, verify);

    if (verify.isValid())
    {
        Serial.println("[IMU] SUCCESS: Bias saved to EEPROM");
        printBiases(verify);
        return true;
    }

    Serial.println("[IMU] ERROR: Bias save verification failed.");
    return false;
}

static void quaternionToAngles(const icm_20948_DMP_data_t &data, Angles &out)
{
    constexpr double inv2pow30 = 1.0 / 1073741824.0; // 1 / 2^30

    const double q1 = data.Quat6.Data.Q1 * inv2pow30;
    const double q2 = data.Quat6.Data.Q2 * inv2pow30;
    const double q3 = data.Quat6.Data.Q3 * inv2pow30;
    const double q0 = sqrt(1.0 - (q1 * q1 + q2 * q2 + q3 * q3));

    // Remap ICM-20948 chip axes → aircraft convention (x-forward, y-right, z-down)
    const double qw = q0;
    const double qx = q2;
    const double qy = q1;
    const double qz = -q3;

    // Roll (+/- 180°)
    out.roll = static_cast<float>(
        atan2(2.0 * (qw * qx + qy * qz), 1.0 - 2.0 * (qx * qx + qy * qy)) * 180.0 / PI);

    // Pitch (+/- 90°) — constrain prevents asin() domain error on noisy input
    const double sinPitch = constrain(2.0 * (qw * qy - qx * qz), -1.0, 1.0);
    out.pitch = static_cast<float>(asin(sinPitch) * 180.0 / PI);

    // Yaw (+/- 180°)
    out.yaw = static_cast<float>(
        atan2(2.0 * (qw * qz + qx * qy), 1.0 - 2.0 * (qy * qy + qz * qz)) * 180.0 / PI);

    if (cfg::IMU_MOUNTED_UPSIDE_DOWN)
    {
        out.pitch = -out.pitch;
        out.roll = -out.roll;
    }
}