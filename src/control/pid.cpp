#include "pid.h"

float wrapTo180(float deg)
{
  while (deg > 180.0f)
    deg -= 360.0f;
  while (deg < -180.0f)
    deg += 360.0f;
  return deg;
}

float balanceRollDeg(float rollDeg)
{
  // Simple normalization helper: keep roll in [-180, 180].
  // Any mounting offset/inversion should be applied by the balance controller.
  return wrapTo180(rollDeg);
}

float PID::step(float error, float measured, float dt)
{
  if (dt <= 0.0f)
    return 0.0f;

  // Derivative on measurement (avoids the spike on setpoint changes).
  // error = setpoint - measured  ⇒  d(error)/dt = -d(measured)/dt
  float dRaw = 0.0f;
  if (hasPrev)
    dRaw = -(measured - prevMeasured) / dt;
  prevMeasured = measured;
  hasPrev = true;

  // Single-pole low-pass filter on the D term to reject sensor noise.
  const float a = (dAlpha <= 0.0f) ? 1.0f : (dAlpha > 1.0f ? 1.0f : dAlpha);
  dFiltered = a * dRaw + (1.0f - a) * dFiltered;

  const float P = kp * error;
  const float D = kd * dFiltered;

  // Only integrate when the output is not already saturated (anti-windup).
  const float uPreview = P + D + ki * i;
  if (uPreview < outMax && uPreview > outMin)
    i += error * dt;

  const float I = ki * i;
  float u = P + I + D;

  if (u > outMax)
    u = outMax;
  if (u < outMin)
    u = outMin;

  // Cache the breakdown for telemetry (web /data → diagnostics chart).
  lastP = P;
  lastI = I;
  lastD = D;
  lastOutput = u;

  return u;
}

void PID::reset()
{
  i = 0.0f;
  prevMeasured = 0.0f;
  hasPrev = false;
  dFiltered = 0.0f;
  lastP = 0.0f;
  lastI = 0.0f;
  lastD = 0.0f;
  lastOutput = 0.0f;
}