#include "motors.h"
#include "config.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
#include "soc/gpio_struct.h"
#endif

namespace
{
    struct MotorPins
    {
        int stepPin;
        int dirPin;
        bool invertDir;
    };

    constexpr MotorPins leftPins = {cfg::MOTOR_L_STEP_PIN, cfg::MOTOR_L_DIR_PIN, cfg::MOTOR_L_INVERT_DIR};
    constexpr MotorPins rightPins = {cfg::MOTOR_R_STEP_PIN, cfg::MOTOR_R_DIR_PIN, cfg::MOTOR_R_INVERT_DIR};

    MotorState state;
    bool initialized = false;

    constexpr uint32_t MICROS_PER_SEC = 1000000UL;
    constexpr float MAX_SPEED = cfg::MOTOR_MAX_STEPS_PER_SEC;

    // Timer tick rate (50kHz => 20us tick)
    constexpr uint32_t TICK_HZ = 50000;
    constexpr uint32_t TICK_US = MICROS_PER_SEC / TICK_HZ;

    // Pulse width in ticks: ensure >= 1 tick (DRV8825 only needs ~2us high).
    constexpr uint8_t PULSE_TICKS =
        (cfg::MOTOR_STEP_PULSE_WIDTH_US <= static_cast<int>(TICK_US))
            ? 1
            : static_cast<uint8_t>((cfg::MOTOR_STEP_PULSE_WIDTH_US + TICK_US - 1) / TICK_US);

    struct MotorIsrState
    {
        int stepPin;
        int dirPin;
        bool invertDir;
        volatile uint32_t phase;
        volatile uint32_t inc;
        volatile uint8_t pulseTicks;
    };

    MotorIsrState L{leftPins.stepPin, leftPins.dirPin, leftPins.invertDir, 0, 0, 0};
    MotorIsrState R{rightPins.stepPin, rightPins.dirPin, rightPins.invertDir, 0, 0, 0};

#if defined(ARDUINO_ARCH_ESP32)
    static inline void IRAM_ATTR gpioWriteFast(int pin, bool level)
    {
        if (pin < 32)
        {
            if (level)
                GPIO.out_w1ts = (1UL << pin);
            else
                GPIO.out_w1tc = (1UL << pin);
        }
        else
        {
            const uint32_t mask = (1UL << (pin - 32));
            if (level)
                GPIO.out1_w1ts.val = mask;
            else
                GPIO.out1_w1tc.val = mask;
        }
    }
#else
    static inline void IRAM_ATTR gpioWriteFast(int pin, bool level)
    {
        digitalWrite(pin, level ? HIGH : LOW);
    }
#endif

    // Shared ISR lock (protects L/R inc and any pin changes atomic with stepping)
    portMUX_TYPE stepMux = portMUX_INITIALIZER_UNLOCKED;

    hw_timer_t *stepTimer = nullptr;

    float clamp(float speed)
    {
        if (speed > MAX_SPEED)
            return MAX_SPEED;
        if (speed < -MAX_SPEED)
            return -MAX_SPEED;
        return speed;
    }

    MotorState::Direction toDir(float speed)
    {
        if (speed > cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return MotorState::Direction::FORWARD;
        if (speed < -cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return MotorState::Direction::BACKWARD;
        return MotorState::Direction::STOP;
    }

    void syncState()
    {
        state.dirLeft = toDir(state.speedLeft);
        state.dirRight = toDir(state.speedRight);

        const bool running =
            (fabsf(state.speedLeft) > cfg::MOTOR_DEADBAND_STEPS_PER_SEC) ||
            (fabsf(state.speedRight) > cfg::MOTOR_DEADBAND_STEPS_PER_SEC);

        state.status = running ? MotorState::Status::RUNNING : MotorState::Status::IDLE;
    }

    // Convert |steps/s| to phase increment per tick. (No floats in ISR)
    uint32_t speedToInc(float speedStepsPerSec)
    {
        const float a = fabsf(speedStepsPerSec);
        if (a < cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return 0;
        return static_cast<uint32_t>(a * (4294967296.0f / static_cast<float>(TICK_HZ))); // 2^32
    }

    static inline void IRAM_ATTR tickOneMotor(MotorIsrState &m)
    {
        // Finish pulse: after pulseTicks expire, set STEP low
        if (m.pulseTicks)
        {
            if (--m.pulseTicks == 0)
                gpioWriteFast(m.stepPin, false);
        }

        // No speed => no stepping
        const uint32_t inc = m.inc;
        if (inc == 0)
            return;

        // Phase accumulator
        const uint32_t prev = m.phase;
        const uint32_t next = prev + inc;
        m.phase = next;

        // Overflow => emit one step pulse
        if (next < prev)
        {
            if(m.pulseTicks == 0 ){
                gpioWriteFast(m.stepPin, true);
                m.pulseTicks = PULSE_TICKS;
            }
        }
    }

    void IRAM_ATTR onStepTimer()
    {
        portENTER_CRITICAL_ISR(&stepMux);
        tickOneMotor(L);
        tickOneMotor(R);
        portEXIT_CRITICAL_ISR(&stepMux);
    }

    // Direction level from signed speed (+) and invertDir flag
    bool dirLevel(const MotorIsrState &m, float speed)
    {
        if (fabsf(speed) < cfg::MOTOR_DEADBAND_STEPS_PER_SEC)
            return false; // arbitrary; motor not stepping
        const bool forward = (speed > 0.0f) != m.invertDir;
        return forward;
    }
}

const MotorState &motorsGetState()
{
    return state;
}

void motorsInit()
{
    pinMode(L.stepPin, OUTPUT);
    pinMode(L.dirPin, OUTPUT);
    pinMode(R.stepPin, OUTPUT);
    pinMode(R.dirPin, OUTPUT);

    digitalWrite(L.stepPin, LOW);
    digitalWrite(R.stepPin, LOW);
    digitalWrite(L.dirPin, LOW);
    digitalWrite(R.dirPin, LOW);

    stepTimer = timerBegin(0, 80, true); // 80MHz/80 => 1us tick base
    timerAttachInterrupt(stepTimer, &onStepTimer, true);
    timerAlarmWrite(stepTimer, TICK_US, true);
    timerAlarmEnable(stepTimer);

    initialized = true;
    Serial.println("[MOTORS] Init (timer ISR)");
}

void motorsStop()
{
    if (!initialized)
        return;

    state.speedLeft = 0.0f;
    state.speedRight = 0.0f;

    portENTER_CRITICAL(&stepMux);
    L.inc = 0;
    R.inc = 0;
    L.pulseTicks = 0;
    R.pulseTicks = 0;
    portEXIT_CRITICAL(&stepMux);

    digitalWrite(L.stepPin, LOW);
    digitalWrite(R.stepPin, LOW);

    syncState();
}

void motorsSetSpeed(float sl, float sr)
{
    if (!initialized)
        return;

    state.speedLeft = clamp(sl);
    state.speedRight = clamp(sr);

    const bool dirL = dirLevel(L, state.speedLeft);
    const bool dirR = dirLevel(R, state.speedRight);

    const uint32_t incL = speedToInc(state.speedLeft);
    const uint32_t incR = speedToInc(state.speedRight);

    // Make direction + speed update atomic with the ISR
    portENTER_CRITICAL(&stepMux);
    gpioWriteFast(L.dirPin, dirL);
    gpioWriteFast(R.dirPin, dirR);
    L.inc = incL;
    R.inc = incR;
    portEXIT_CRITICAL(&stepMux);

    syncState();
}