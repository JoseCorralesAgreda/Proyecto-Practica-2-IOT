#include "led_controller.h"

#include <Arduino.h>
#include <driver/gpio.h>

void LedController::begin() {
    ready_ = false;
    if (!config::LED_WIRING_CONFIRMED) {
        Serial.println("LED outputs disabled: confirm wiring and GPIO configuration.");
        return;
    }
    for (size_t i = 0; i < config::LED_COUNT; ++i) {
        if (!GPIO_IS_VALID_OUTPUT_GPIO(config::LED_PINS[i])) {
            Serial.println("Invalid LED output GPIO.");
            return;
        }
        for (size_t j = 0; j < i; ++j) {
            if (config::LED_PINS[i] == config::LED_PINS[j]) {
                Serial.println("LED GPIO assignments must be distinct.");
                return;
            }
        }
    }
    for (size_t i = 0; i < config::LED_COUNT; ++i) {
        digitalWrite(config::LED_PINS[i], LOW);
        pinMode(config::LED_PINS[i], OUTPUT);
    }
    ready_ = true;
    turnOff();
}

void LedController::setPattern(const char *pattern, uint32_t now) {
    for (size_t i = 0; i < config::LED_COUNT; ++i) {
        if (states_[i] != pattern[i]) {
            states_[i] = pattern[i];
            blinkStartedMs_[i] = now;
        }
    }
    update(now);
}

void LedController::update(uint32_t now) {
    if (!ready_) {
        return;
    }
    for (size_t i = 0; i < config::LED_COUNT; ++i) {
        // Advance whole cycles to preserve phase through millis() wraparound.
        const uint32_t elapsed = now - blinkStartedMs_[i];
        const uint32_t phase = elapsed % config::BLINK_PERIOD_MS;
        blinkStartedMs_[i] += elapsed - phase;
        const bool on =
            states_[i] == 'S' || (states_[i] == 'B' && phase < config::BLINK_PERIOD_MS / 2);
        digitalWrite(config::LED_PINS[i], on ? HIGH : LOW);
    }
}

void LedController::turnOff() {
    for (size_t i = 0; i < config::LED_COUNT; ++i) {
        states_[i] = '0';
        if (ready_) {
            digitalWrite(config::LED_PINS[i], LOW);
        }
    }
}
