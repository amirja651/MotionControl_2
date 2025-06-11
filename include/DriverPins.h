#ifndef DRIVER_PINS_H
#define DRIVER_PINS_H

#include <Arduino.h>

struct DriverPins
{
    static const uint16_t DIR[4];
    static const uint16_t STEP[4];
    static const uint16_t EN[4];
    static const uint16_t CS[4];
    static const uint16_t LEDC_CHANNEL[4];
};

#endif  // DRIVER_PINS_H