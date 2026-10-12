#pragma once

/** Measures an ultrasonic echo without waiting indefinitely. */
class UltrasonicSensor {
  public:
    /** Configure pins only after wiring has been explicitly confirmed. */
    void begin();
    /** Return false for missing echoes, disabled wiring or out-of-range values. */
    bool readDistance(float &distanceCm);

  private:
    bool ready_ = false;
};
