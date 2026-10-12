#include <Arduino.h>

namespace {
constexpr unsigned long SERIAL_BAUD_RATE = 115200;
constexpr unsigned long IDLE_DELAY_MS = 10;
}

/** Initialize serial output for the actuator project scaffold. */
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Actuator project initialized.");
}

/** Yield while application behavior awaits implementation. */
void loop() {
    delay(IDLE_DELAY_MS);
}
