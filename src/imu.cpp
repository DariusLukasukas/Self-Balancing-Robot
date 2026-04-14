#include "config.h"
#include "types.h"
#include "ICM_20948.h"
#include <EEPROM.h>

static ICM_20948_I2C myICM;
static bool biasRestored = false;
static bool biasSaved = false;
static bool eepromReady = false;
static uint32_t startTimeMs = 0;

static bool connectSensor()
{
    for (uint8_t attempt = 1; attempt <= cfg::IMU_INIT_MAX_TRIES; attempt++)
    {
        myICM.begin(IMU_WIRE_PORT, cfg::IMU_AD0_VAL);
        Serial.printf("[IMU] Connect attempt %d/%d: %s\n",
                      attempt, cfg::IMU_INIT_MAX_TRIES, myICM.statusString());

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

static void printBias(const BiasStore &store)
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
        Serial.println("[IMU] WARNING: No valid bias data in EEPROM - START UNCALIBRATED");
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
        printBias(store);
    }
    else
    {
        Serial.println("[IMU] ERROR: Bias restore failed!");
    }

    return success;
}

static bool saveBias()
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
        printBias(verify);
        return true;
    }

    Serial.println("[IMU] ERROR: Bias save verification failed.");
    return false;
}

static void quaternionToAngles(const icm_20948_DMP_data_t &data, Angles &out)
{
    constexpr double inv2pow30 = 1.0 / 1073741824.0; // 1 / 2^30
    constexpr double radToDeg = 180.0 / PI;

    const double q1 = data.Quat6.Data.Q1 * inv2pow30;
    const double q2 = data.Quat6.Data.Q2 * inv2pow30;
    const double q3 = data.Quat6.Data.Q3 * inv2pow30;

    // Guard sqrt against tiny negative values from floating-point roundoff
    const double q0sq = 1.0 - (q1 * q1 + q2 * q2 + q3 * q3);
    const double q0 = sqrt(q0sq > 0.0 ? q0sq : 0.0);

    // Remap ICM-20948 chip axes → aircraft convention (x-forward, y-right, z-down)
    const double qw = q0;
    const double qx = q2;
    const double qy = q1;
    const double qz = -q3;

    // Roll (x-axis rotation)
    const double t0 = 2.0 * (qw * qx + qy * qz);
    const double t1 = 1.0 - 2.0 * (qx * qx + qy * qy);
    const double roll = atan2(t0, t1) * radToDeg;

    // Pitch (y-axis rotation)
    double t2 = 2.0 * (qw * qy - qx * qz);
    t2 = t2 > 1.0 ? 1.0 : t2;
    t2 = t2 < -1.0 ? -1.0 : t2;
    const double pitch = asin(t2) * radToDeg;

    // Yaw (z-axis rotation)
    const double t3 = 2.0 * (qw * qz + qx * qy);
    const double t4 = 1.0 - 2.0 * (qy * qy + qz * qz);
    const double yaw = atan2(t3, t4) * radToDeg;

    out.roll = static_cast<float>(roll);
    out.pitch = static_cast<float>(pitch);
    out.yaw = static_cast<float>(yaw);

    if (cfg::IMU_MOUNTED_UPSIDE_DOWN)
    {
        out.pitch = -out.pitch;
        out.roll = -out.roll;
    }
}

// === Public APIs ===
bool imuInit()
{
    IMU_WIRE_PORT.begin(cfg::IMU_SDA_PIN, cfg::IMU_SCL_PIN);
    IMU_WIRE_PORT.setClock(cfg::IMU_I2C_CLOCK);

    if (!connectSensor())
    {
        Serial.println("[IMU] ERROR: Sensor not found. Check wiring!");
        return false;
    }
    Serial.println("[IMU] SUCCESS: Sensor connected");

    if (!configureDMP())
    {
        Serial.println("[IMU] ERROR: DMP setup failed");
        return false;
    }
    Serial.println("[IMU] SUCCESS: DMP ready - 6DoF mode");

    eepromReady = EEPROM.begin(cfg::IMU_EEPROM_SIZE);

    if (!eepromReady)
    {
        Serial.println("[IMU] WARNING: EEPROM init failed - bias will not be saved");
    }
    else
    {
        biasRestored = restoreBias();
    }

    startTimeMs = millis();
    return true;
}

void imuUpdateBias()
{
    if (biasSaved)
        return;
    if (!eepromReady)
        return;

    if ((millis() - startTimeMs) >= cfg::IMU_BIAS_CALIBRATION_MS)
    {
        Serial.println("[IMU] Calibration period complete, saving bias");
        biasSaved = saveBias();
    }
}

bool imuRead(Angles &out)
{
    icm_20948_DMP_data_t data;
    myICM.readDMPdataFromFIFO(&data);

    const bool hasData = (myICM.status == ICM_20948_Stat_Ok ||
                          myICM.status == ICM_20948_Stat_FIFOMoreDataAvail);

    if (!hasData)
        return false;
    if (!(data.header & DMP_header_bitmap_Quat6))
        return false;

    quaternionToAngles(data, out);

    // Drain remaining FIFO frames to prevent lag buildup
    if (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)
        imuRead(out);

    return true;
}

bool imuBiasesRestored()
{
    return biasRestored;
}

void imuPrintAngles(const Angles &a)
{
    Serial.printf("Pitch: %6.2f  Roll: %6.2f  Yaw: %6.2f  (deg)\n",
                  a.pitch, a.roll, a.yaw);
}