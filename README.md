# Self Balancing Robot

A self-balancing two-wheel robot built with PlatformIO. It reads an IMU, runs a balance controller, and drives stepper motors. Includes a web UI for telemetry + PID tuning.

## Features
- IMU angle estimation
- Balance control loop (PID)
- Stepper motor drive (timer/ISR stepping)
- Web dashboard for:
  - Live telemetry (angle/target, motor commands)
  - PID tuning (Kp/Ki/Kd + target)
  - Quick tuning metrics

## Hardware
- **MCU**: ESP32 DevKit v1 (30 pin) Board
- **IMU**: SparkFun ICM-20948 9DoF
- **Motors/Drivers**: NEMA 17HE15-1504S Stepper motors + DRV8825
- **Power**: DC 24V, 5A Power Supply

## Wiring
List the important connections here.

- **IMU**
  - SDA: 21
  - SCL: 22
  - VCC/GND

- **Motor drivers**
  - Left STEP/DIR: 19, 18
  - Right STEP/DIR: 32, 33
  - Microstepping: 8

## Project structure
- `src/main.cpp`: main loop and task setup
- `src/control/`: balance + PID control
- `src/sensors/`: IMU reading + filtering
- `src/actuators/`: motor driver
- `src/web/`: web UI (served by the firmware)
- `include/`: public headers and configuration

## Build & upload
### Prerequisites
- [PlatformIO](https://platformio.org/) (VS Code extension recommended)

### Build
```bash
pio run