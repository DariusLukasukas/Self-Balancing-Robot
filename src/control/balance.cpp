#include "balance.h"

#include <Arduino.h>

#include "config.h"
#include "motors.h"

namespace
{
    // Spinlock guarding everything below that is touched by both the main loop
    // (Core 1, balanceUpdate / balanceFreezeOutput) and the web task
    // (Core 0, balanceSet*/balanceGet*/balanceHardStop).
    portMUX_TYPE balanceMux = portMUX_INITIALIZER_UNLOCKED;

    PID pid;
    ControlMode mode = ControlMode::MANUAL;
    float manualLeft = 0.0f;
    float manualRight = 0.0f;
    float cmdLeft = 0.0f;
    float cmdRight = 0.0f;
    float lastMeasured = 0.0f;
    float targetAngle = 0.0f;
    bool initialized = false;

    // Real elapsed time tracker for the manual-mode slew rate (independent of IMU
    // sample timing so the rate-limit is correct even when the IMU is silent).
    uint32_t manualLastUs = 0;

    float clampf(float v, float lo, float hi)
    {
        if (v < lo)
            return lo;
        if (v > hi)
            return hi;
        return v;
    }

    float slew(float current, float target, float maxDelta)
    {
        const float d = target - current;
        if (d > maxDelta)
            return current + maxDelta;
        if (d < -maxDelta)
            return current - maxDelta;
        return target;
    }
}

void balanceInit()
{
    portENTER_CRITICAL(&balanceMux);
    pid.kp = cfg::PID_KP;
    pid.ki = cfg::PID_KI;
    pid.kd = cfg::PID_KD;
    targetAngle = cfg::PID_TARGET_ANGLE_DEG;
    pid.i = 0.0f;
    pid.prevMeasured = 0.0f;
    pid.hasPrev = false;
    pid.dFiltered = 0.0f;
    pid.dAlpha = cfg::PID_D_FILTER_ALPHA;
    pid.outMin = -cfg::MOTOR_MAX_STEPS_PER_SEC;
    pid.outMax = cfg::MOTOR_MAX_STEPS_PER_SEC;
    pid.lastP = 0.0f;
    pid.lastI = 0.0f;
    pid.lastD = 0.0f;
    pid.lastOutput = 0.0f;

    manualLeft = 0.0f;
    manualRight = 0.0f;
    cmdLeft = 0.0f;
    cmdRight = 0.0f;
    mode = cfg::CONTROL_START_IN_BALANCE ? ControlMode::BALANCE : ControlMode::MANUAL;
    initialized = true;
    portEXIT_CRITICAL(&balanceMux);

    if (mode == ControlMode::MANUAL)
        motorsStop();
}

void balanceUpdate(const Angles &a, float dt)
{
    if (!initialized)
        balanceInit();

    // Snapshot all shared state once at the top so the math below operates on a
    // consistent view, even if the web task mutates state mid-update.
    ControlMode m;
    float ml, mr, tgt;
    PID p;
    portENTER_CRITICAL(&balanceMux);
    m = mode;
    ml = manualLeft;
    mr = manualRight;
    tgt = targetAngle;
    p = pid;
    portEXIT_CRITICAL(&balanceMux);

    const float roll = cfg::PID_ROLL_INVERT ? -a.roll : a.roll;
    const float measured = wrapTo180(roll + cfg::PID_ROLL_OFFSET_DEG);

    // Real elapsed time since the last balanceUpdate call (used for manual slew
    // and as a fallback when the IMU dt parameter is zero).
    const uint32_t now = micros();
    float realDt = 0.0f;
    if (manualLastUs != 0)
        realDt = (now - manualLastUs) * 1e-6f;
    manualLastUs = now;
    if (realDt <= 0.0f || realDt > 0.5f)
        realDt = 0.02f; // safe fallback ~50 Hz

    if (m == ControlMode::MANUAL)
    {
        const float maxDelta = cfg::MOTOR_SLEW_RATE_STEPS_PER_SEC2 * realDt;
        cmdLeft = slew(cmdLeft, ml, maxDelta);
        cmdRight = slew(cmdRight, mr, maxDelta);
        motorsSetSpeed(cmdLeft, cmdRight);

        portENTER_CRITICAL(&balanceMux);
        lastMeasured = measured;
        portEXIT_CRITICAL(&balanceMux);
        return;
    }

    // BALANCE — use the IMU-supplied dt for PID math (correct inter-sample interval).
    const float error = tgt - measured;

    // Safety cut: clearly fallen over → stop motors and clear integrator.
    if (fabsf(measured) > cfg::PID_MAX_TILT_DEG)
    {
        motorsStop();
        cmdLeft = 0.0f;
        cmdRight = 0.0f;

        portENTER_CRITICAL(&balanceMux);
        pid.reset();
        lastMeasured = measured;
        portEXIT_CRITICAL(&balanceMux);
        return;
    }

    const float u = p.step(error, measured, dt);
    cmdLeft = u;
    cmdRight = u;
    motorsSetSpeed(u, u);

    // Persist updated PID internal state and the latest measured angle.
    portENTER_CRITICAL(&balanceMux);
    pid.i = p.i;
    pid.prevMeasured = p.prevMeasured;
    pid.dFiltered = p.dFiltered;
    pid.hasPrev = p.hasPrev;
    pid.lastP = p.lastP;
    pid.lastI = p.lastI;
    pid.lastD = p.lastD;
    pid.lastOutput = p.lastOutput;
    lastMeasured = measured;
    portEXIT_CRITICAL(&balanceMux);
}

void balanceSetMode(ControlMode m)
{
    if (!initialized)
        balanceInit();

    bool entering;
    {
        portENTER_CRITICAL(&balanceMux);
        if (m == mode)
        {
            portEXIT_CRITICAL(&balanceMux);
            return;
        }
        mode = m;
        cmdLeft = 0.0f;
        cmdRight = 0.0f;
        if (m == ControlMode::BALANCE)
            pid.reset();
        entering = (m == ControlMode::MANUAL);
        portEXIT_CRITICAL(&balanceMux);
    }

    if (entering)
    {
        // Do not clear manualLeft/Right here — user may switch back to BALANCE without pressing Run.
        motorsStop();
    }
}

ControlMode balanceGetMode()
{
    portENTER_CRITICAL(&balanceMux);
    const ControlMode m = mode;
    portEXIT_CRITICAL(&balanceMux);
    return m;
}

void balanceSetManualSpeeds(float sl, float sr)
{
    const float lo = -cfg::MOTOR_MAX_STEPS_PER_SEC;
    const float hi = cfg::MOTOR_MAX_STEPS_PER_SEC;
    const float ll = clampf(sl, lo, hi);
    const float rr = clampf(sr, lo, hi);
    portENTER_CRITICAL(&balanceMux);
    manualLeft = ll;
    manualRight = rr;
    portEXIT_CRITICAL(&balanceMux);
}

void balanceHardStop()
{
    if (!initialized)
        balanceInit();

    portENTER_CRITICAL(&balanceMux);
    mode = ControlMode::MANUAL;
    manualLeft = 0.0f;
    manualRight = 0.0f;
    cmdLeft = 0.0f;
    cmdRight = 0.0f;
    pid.reset();
    portEXIT_CRITICAL(&balanceMux);

    motorsStop();
}

void balanceFreezeOutput()
{
    if (!initialized)
        balanceInit();

    // Stop motors and clear ramp state but do NOT reset PID integrator —
    // resetting the integrator mid-balance causes a jerk on recovery.
    portENTER_CRITICAL(&balanceMux);
    cmdLeft = 0.0f;
    cmdRight = 0.0f;
    portEXIT_CRITICAL(&balanceMux);

    motorsStop();
}

void balanceGetPidGains(float &kp, float &ki, float &kd)
{
    portENTER_CRITICAL(&balanceMux);
    kp = pid.kp;
    ki = pid.ki;
    kd = pid.kd;
    portEXIT_CRITICAL(&balanceMux);
}

void balanceSetPidGains(float kp, float ki, float kd)
{
    portENTER_CRITICAL(&balanceMux);
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;
    portEXIT_CRITICAL(&balanceMux);
}

void balanceResetPid()
{
    portENTER_CRITICAL(&balanceMux);
    pid.reset();
    portEXIT_CRITICAL(&balanceMux);
}

void balanceGetPidComponents(float &p, float &i, float &d)
{
    portENTER_CRITICAL(&balanceMux);
    p = pid.lastP;
    i = pid.lastI;
    d = pid.lastD;
    portEXIT_CRITICAL(&balanceMux);
}

float balanceGetMeasuredAngle()
{
    portENTER_CRITICAL(&balanceMux);
    const float v = lastMeasured;
    portEXIT_CRITICAL(&balanceMux);
    return v;
}

float balanceGetTargetAngle()
{
    portENTER_CRITICAL(&balanceMux);
    const float v = targetAngle;
    portEXIT_CRITICAL(&balanceMux);
    return v;
}

void balanceSetTargetAngle(float deg)
{
    portENTER_CRITICAL(&balanceMux);
    targetAngle = deg;
    portEXIT_CRITICAL(&balanceMux);
}
