#pragma once

#include "config.h"

/** Drives three active-high LEDs with a local, nonblocking blink timer. */
class LedController {
  public:
    /** Initialize confirmed pins with all LEDs off. */
    void begin();
    /** Apply three validated states; unchanged states retain their blink phase. */
    void setPattern(const char *pattern, uint32_t now);
    /** Refresh outputs without waiting for the next blink transition. */
    void update(uint32_t now);
    /** Clear the pattern and immediately switch all configured outputs off. */
    void turnOff();

  private:
    bool ready_ = false;
    char states_[config::LED_COUNT] = {'0', '0', '0'};
    uint32_t blinkStartedMs_[config::LED_COUNT] = {};
};
