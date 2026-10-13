#include "sensor_client.h"

#include <cstdio>
#include <cstring>

SensorClient::SensorClient(UltrasonicSensor &sensor) : sensor_(sensor) {}

void SensorClient::begin() {
    serverAddressValid_ = serverAddress_.fromString(config::SERVER_HOST);
    if (!serverAddressValid_) {
        Serial.println("Invalid server IPv4 address.");
    }
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    Serial.println("Connecting to WiFi...");
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
    lastWifiAttemptMs_ = millis();
}

void SensorClient::disconnect() {
    socket_.stop();
    registered_ = false;
    lineLength_ = 0;
    discardLine_ = false;
    lastTcpAttemptMs_ = millis();
}

bool SensorClient::sendLine(const char *line) {
    const size_t length = strlen(line);
    if (socket_.write(reinterpret_cast<const uint8_t *>(line), length) != length) {
        Serial.println("TCP send failed; reconnect scheduled.");
        disconnect();
        return false;
    }
    return true;
}

void SensorClient::applyCommand() {
    constexpr size_t PREFIX_LENGTH = sizeof("PERIOD ") - 1;
    if (lineLength_ < PREFIX_LENGTH + 3 || lineLength_ > PREFIX_LENGTH + 4 ||
        memcmp(line_, "PERIOD ", PREFIX_LENGTH) != 0) {
        return;
    }
    uint32_t value = 0;
    for (size_t i = PREFIX_LENGTH; i < lineLength_; ++i) {
        if (line_[i] < '0' || line_[i] > '9') {
            return;
        }
        value = value * 10 + (line_[i] - '0');
    }
    periodMs_ = constrain(value, config::MIN_PERIOD_MS, config::MAX_PERIOD_MS);
    Serial.printf("Measurement period: %lu ms\n", static_cast<unsigned long>(periodMs_));
}

void SensorClient::receiveCommands() {
    // Bound work per loop even if the peer continuously sends data.
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

void SensorClient::update() {
    uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        if (wifiConnected_ || registered_) {
            disconnect();
            wifiConnected_ = false;
            Serial.println("WiFi disconnected.");
        }
        if (now - lastWifiAttemptMs_ >= config::RECONNECT_INTERVAL_MS) {
            lastWifiAttemptMs_ = now;
            Serial.println("WiFi unavailable; retrying connection.");
            WiFi.disconnect();
            WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
        }
        return;
    }
    if (!wifiConnected_) {
        wifiConnected_ = true;
        lastTcpAttemptMs_ = now - config::RECONNECT_INTERVAL_MS;
        Serial.printf("WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
    }
    if (!serverAddressValid_) {
        return;
    }
    if (!socket_.connected()) {
        if (registered_) {
            Serial.println("TCP connection lost; reconnect scheduled.");
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
            Serial.println("TCP connection to server failed; retry scheduled.");
            return;
        }
        socket_.setNoDelay(true);
        periodMs_ = config::DEFAULT_PERIOD_MS;
        if (!sendLine("HELLO SENSOR\n")) {
            return;
        }
        registered_ = true;
        lastMeasurementMs_ = millis();
        Serial.println("Sensor registered over TCP.");
    }
    receiveCommands();
    now = millis();
    // Unsigned subtraction remains correct across millis() wraparound.
    if (now - lastMeasurementMs_ >= periodMs_) {
        lastMeasurementMs_ = now;
        float distanceCm = 0;
        char message[config::MAX_LINE_BYTES];
        if (sensor_.readDistance(distanceCm)) {
            snprintf(message, sizeof(message), "DIST %.1f\n", distanceCm);
            if (sendLine(message)) {
                Serial.printf("Distance: %.1f cm sent to server.\n", distanceCm);
            }
        } else {
            snprintf(message, sizeof(message), "DIST ERR\n");
            if (sendLine(message)) {
                Serial.println("Invalid distance reading; DIST ERR sent to server.");
            }
        }
    }
}
