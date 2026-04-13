#include <Arduino.h>
#include <config.h>

void setup()
{
    Serial.begin(cfg::SERIAL_BAUD);
    delay(cfg::SERIAL_BOOT_DELAY);
}

void loop()
{
}