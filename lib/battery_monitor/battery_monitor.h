#pragma once
#include <Arduino.h>
#include "config.h"

class BatteryMonitor {
public:
    BatteryMonitor(uint8_t pin);
    void begin();
    void update();
    bool needsUpdate() const;   // true si BATTERY_READ_INTERVAL écoulé
    int getPercent() const { return _percent; }

private:
    uint8_t _pin;
    volatile int32_t _percent = 100;
    unsigned long _lastRead = 0;
};