#pragma once

#include "config.h"
#include "ultrasonic_sensor.h"
#include <WiFi.h>

/** Maintains WiFi/TCP registration and periodic distance reporting. */
class SensorClient {
  public:
    /** Use the sensor owned by the application. */
    explicit SensorClient(UltrasonicSensor &sensor);
    /** Start WiFi without waiting for association. */
    void begin();
    /** Service connections, incoming commands and one due measurement. */
    void update();

  private:
    void receiveCommands();
    void applyCommand();
    bool sendLine(const char *line);
    void disconnect();

    UltrasonicSensor &sensor_;
    WiFiClient socket_;
    IPAddress serverAddress_;
    bool serverAddressValid_ = false;
    bool wifiConnected_ = false;
    bool registered_ = false;
    uint32_t lastWifiAttemptMs_ = 0;
    uint32_t lastTcpAttemptMs_ = 0;
    uint32_t lastMeasurementMs_ = 0;
    uint32_t periodMs_ = config::DEFAULT_PERIOD_MS;
    char line_[config::MAX_LINE_BYTES] = {};
    size_t lineLength_ = 0;
    bool discardLine_ = false;
};
