#include "config.h"
#include "sensor_client.h"
#include "ultrasonic_sensor.h"
#include <Arduino.h>

namespace {
UltrasonicSensor sensor;
SensorClient client(sensor);
} // namespace

/** Initialize measurement pins and start the WiFi connection. */
void setup() {
    Serial.begin(config::SERIAL_BAUD_RATE);
    Serial.println("\nSensor starting.");
    sensor.begin();
    client.begin();
}

/** Service the sensor connection and yield to the ESP32 network tasks. */
void loop() {
    client.update();
    delay(config::LOOP_DELAY_MS);
}
