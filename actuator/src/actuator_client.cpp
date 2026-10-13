#include "actuator_client.h"

#include <cstring>

namespace {
const char *ledStateName(char state) {
    switch (state) {
    case 'S':
        return "on";
    case 'B':
        return "blinking";
    default:
        return "off";
    }
}
} // namespace

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
    Serial.println("Connecting to WiFi...");
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
    lastWifiAttemptMs_ = millis();
}

void ActuatorClient::disconnect() {
    leds_.turnOff();
    socket_.stop();
    registered_ = false;
    lineLength_ = 0;
    discardLine_ = false;
    hasCommand_ = false;
    lastTcpAttemptMs_ = millis();
}

void ActuatorClient::applyCommand() {
    constexpr size_t PREFIX_LENGTH = sizeof("LED ") - 1;
    if (lineLength_ != PREFIX_LENGTH + config::LED_COUNT ||
        memcmp(line_, "LED ", PREFIX_LENGTH) != 0) {
        Serial.println("Ignored invalid LED command.");
        return;
    }
    for (size_t i = PREFIX_LENGTH; i < lineLength_; ++i) {
        if (line_[i] != '0' && line_[i] != 'S' && line_[i] != 'B') {
            Serial.println("Ignored invalid LED command.");
            return;
        }
    }
    lastCommandMs_ = millis();
    leds_.setPattern(line_ + PREFIX_LENGTH, lastCommandMs_);
    if (!hasCommand_ || memcmp(lastPattern_, line_ + PREFIX_LENGTH, config::LED_COUNT) != 0) {
        memcpy(lastPattern_, line_ + PREFIX_LENGTH, config::LED_COUNT);
        lastPattern_[config::LED_COUNT] = '\0';
        hasCommand_ = true;
        Serial.printf("LED pattern received: red=%s, orange=%s, green=%s.\n",
                      ledStateName(lastPattern_[0]), ledStateName(lastPattern_[1]),
                      ledStateName(lastPattern_[2]));
    }
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
