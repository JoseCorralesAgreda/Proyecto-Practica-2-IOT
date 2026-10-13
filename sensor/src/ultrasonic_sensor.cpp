#include "ultrasonic_sensor.h"

#include "config.h"
#include <Arduino.h>
#include <driver/gpio.h>

void UltrasonicSensor::begin() {
    ready_ = config::SENSOR_WIRING_CONFIRMED && GPIO_IS_VALID_OUTPUT_GPIO(config::TRIGGER_PIN) &&
             GPIO_IS_VALID_GPIO(config::ECHO_PIN) && config::TRIGGER_PIN != config::ECHO_PIN;
    if (!ready_) {
        Serial.println("Sensor disabled: confirm wiring and GPIO configuration.");
        return;
    }
    pinMode(config::TRIGGER_PIN, OUTPUT);
    digitalWrite(config::TRIGGER_PIN, LOW);
    pinMode(config::ECHO_PIN, INPUT);
    Serial.printf("Ultrasonic sensor ready (TRIG=%d, ECHO=%d).\n", config::TRIGGER_PIN,
                  config::ECHO_PIN);
}

bool UltrasonicSensor::readDistance(float &distanceCm) {
    if (!ready_) {
        return false;
    }
    digitalWrite(config::TRIGGER_PIN, LOW);
    delayMicroseconds(config::TRIGGER_LOW_US);
    digitalWrite(config::TRIGGER_PIN, HIGH);
    delayMicroseconds(config::TRIGGER_HIGH_US);
    digitalWrite(config::TRIGGER_PIN, LOW);
    const unsigned long echoUs = pulseIn(config::ECHO_PIN, HIGH, config::ECHO_TIMEOUT_US);
    // The echo travels from the sensor to the target and back.
    distanceCm = echoUs * config::SOUND_SPEED_CM_PER_US / 2.0f;
    return echoUs != 0 && distanceCm >= config::MIN_DISTANCE_CM &&
           distanceCm <= config::MAX_DISTANCE_CM;
}
