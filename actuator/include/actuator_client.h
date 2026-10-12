#pragma once

#include "config.h"
#include "led_controller.h"
#include <WiFi.h>

/** Receives LED commands and turns outputs off when control is lost. */
class ActuatorClient {
  public:
    /** Use the LED controller owned by the application. */
    explicit ActuatorClient(LedController &leds);
    /** Start WiFi without waiting for association. */
    void begin();
    /** Service connections, command framing and the five-second watchdog. */
    void update();

  private:
    void receiveCommands();
    void applyCommand();
    void disconnect();

    LedController &leds_;
    WiFiClient socket_;
    IPAddress serverAddress_;
    bool serverAddressValid_ = false;
    bool wifiConnected_ = false;
    bool registered_ = false;
    uint32_t lastWifiAttemptMs_ = 0;
    uint32_t lastTcpAttemptMs_ = 0;
    uint32_t lastCommandMs_ = 0;
    char line_[config::MAX_LINE_BYTES] = {};
    size_t lineLength_ = 0;
    bool discardLine_ = false;
};
