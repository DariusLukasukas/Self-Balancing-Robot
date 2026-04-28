#pragma once

float wrapTo180(float deg);
float balanceRollDeg(float rollDeg);

struct PID
{
  float kp, ki, kd;
  float i;
  float prevMeasured; // for derivative-on-measurement
  bool hasPrev;
  float dFiltered;    // single-pole IIR state for D term
  float dAlpha;       // filter coefficient (0..1); 1.0 disables filtering
  float outMin, outMax;

  // Last-step diagnostics (read-only output of step()), in the same units as the
  // controller output (steps/s). Useful for telemetry to see which term is
  // dominating at any moment during tuning.
  float lastP;
  float lastI;
  float lastD;
  float lastOutput;

  // Compute the controller output from `error = setpoint - measured` and the current
  // `measured` value. Derivative is taken on `measured` (not `error`) to avoid the
  // "derivative kick" when the setpoint changes, then low-pass filtered.
  float step(float error, float measured, float dt);
  void reset();
};