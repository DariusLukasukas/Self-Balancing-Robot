#pragma once

#include "types.h"
#include "pid.h"

enum class ControlMode : uint8_t
{
    BALANCE = 0,
    MANUAL = 1,
};

// Initializes PID values (defaults from config) and starts in BALANCE mode.
void balanceInit();

// Update control output. Call once per loop with dt seconds.
void balanceUpdate(const Angles &a, float dt);

// Control mode switching.
void balanceSetMode(ControlMode m);
ControlMode balanceGetMode();

// Manual override speeds (used when mode == MANUAL).
void balanceSetManualSpeeds(float sl, float sr);

// Immediate stop: zero controller commands and stop motors now.
void balanceHardStop();

// Stop motors and clear ramp/PID state without leaving BALANCE mode (e.g. IMU sample gap).
void balanceFreezeOutput();

// PID accessors for web tuning.
void balanceGetPidGains(float &kp, float &ki, float &kd);
void balanceSetPidGains(float kp, float ki, float kd);
void balanceResetPid();

// Last-step PID contributions (P, I, D in motor units) — for the tuning UI to
// visualize which term is dominating. Cleared by balanceResetPid().
void balanceGetPidComponents(float &p, float &i, float &d);

// Target angle accessors (balance setpoint, degrees).
float balanceGetTargetAngle();
void balanceSetTargetAngle(float deg);

// Returns the last normalized roll angle seen by the PID (same value as `measured` in balanceUpdate).
float balanceGetMeasuredAngle();

