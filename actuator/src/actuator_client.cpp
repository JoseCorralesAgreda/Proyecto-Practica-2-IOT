#include "actuator_client.h"

#include <cstring>

ActuatorClient::ActuatorClient(LedController &leds) : leds_(leds) {}

void ActuatorClient::begin() {
    leds_.turnOff();
    serverAddressValid_ = serverAddress_.fromString(config::SERVER_HOST);
    if (!serverAddressValid_) {
        Serial.println("Invalid server IPv4 address.");
    }
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
    lastWifiAttemptMs_ = millis();
}

void ActuatorClient::disconnect() {
    leds_.turnOff();
    socket_.stop();
    registered_ = false;
    lineLength_ = 0;
    discardLine_ = false;
    lastTcpAttemptMs_ = millis();
}

void ActuatorClient::applyCommand() {
    constexpr size_t PREFIX_LENGTH = sizeof("LED ") - 1;
    if (lineLength_ != PREFIX_LENGTH + config::LED_COUNT ||
        memcmp(line_, "LED ", PREFIX_LENGTH) != 0) {
        return;
    }
    for (size_t i = PREFIX_LENGTH; i < lineLength_; ++i) {
        if (line_[i] != '0' && line_[i] != 'S' && line_[i] != 'B') {
            return;
        }
    }
    lastCommandMs_ = millis();
    leds_.setPattern(line_ + PREFIX_LENGTH, lastCommandMs_);
}

void ActuatorClient::receiveCommands() {
    // Limit each pass so malformed traffic cannot starve the watchdog or LEDs.
    for (size_t i = 0; i < config::RECEIVE_BYTES_PER_LOOP && socket_.available(); ++i) {
        const int byte = socket_.read();
        if (byte < 0) {
            break;
        }
        if (byte == '\n') {
            if (!discardLine_) {
                applyCommand();
            }
            lineLength_ = 0;
            discardLine_ = false;
        } else if (!discardLine_) {
            if (lineLength_ == sizeof(line_) - 1) {
                discardLine_ = true;
            } else {
                line_[lineLength_++] = static_cast<char>(byte);
            }
        }
    }
}

void ActuatorClient::update() {
    const uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        if (wifiConnected_ || registered_) {
            disconnect();
            wifiConnected_ = false;
            Serial.println("WiFi disconnected; LEDs off.");
        }
        if (now - lastWifiAttemptMs_ >= config::RECONNECT_INTERVAL_MS) {
            lastWifiAttemptMs_ = now;
            WiFi.disconnect();
            WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
        }
        return;
    }
    if (!wifiConnected_) {
        wifiConnected_ = true;
        lastTcpAttemptMs_ = now - config::RECONNECT_INTERVAL_MS;
        Serial.println("WiFi connected.");
    }
    if (!serverAddressValid_) {
        return;
    }
    if (!socket_.connected()) {
        if (registered_) {
            disconnect();
            return;
        }
        if (now - lastTcpAttemptMs_ < config::RECONNECT_INTERVAL_MS) {
            return;
        }
        const bool connected =
            socket_.connect(serverAddress_, config::SERVER_PORT, config::CONNECT_TIMEOUT_MS);
        lastTcpAttemptMs_ = millis();
        if (!connected) {
            return;
        }
        socket_.setNoDelay(true);
        constexpr char HELLO[] = "HELLO ACTUATOR\n";
        if (socket_.write(reinterpret_cast<const uint8_t *>(HELLO), sizeof(HELLO) - 1) !=
            sizeof(HELLO) - 1) {
            disconnect();
            return;
        }
        registered_ = true;
        lastCommandMs_ = millis();
        Serial.println("Actuator registered over TCP.");
    }
    // Check before accepting queued commands so expired sessions cannot revive outputs.
    if (millis() - lastCommandMs_ >= config::COMMAND_TIMEOUT_MS) {
        Serial.println("Command timeout; LEDs off.");
        disconnect();
        return;
    }
    receiveCommands();
}
