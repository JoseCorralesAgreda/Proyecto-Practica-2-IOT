#include "actuator_client.h"
#include "config.h"
#include "led_controller.h"
#include <Arduino.h>

namespace {
LedController leds;
ActuatorClient client(leds);
} // namespace

/** Initialize outputs off and start the WiFi connection. */
void setup() {
    Serial.begin(config::SERIAL_BAUD_RATE);
    Serial.println("\nActuator starting.");
    leds.begin();
    client.begin();
}

/** Service commands and local blinking, then yield to network tasks. */
void loop() {
    client.update();
    leds.update(millis());
    delay(config::LOOP_DELAY_MS);
}
