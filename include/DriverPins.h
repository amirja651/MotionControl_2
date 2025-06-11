#ifndef DRIVER_PINS_H
#define DRIVER_PINS_H

#include <Arduino.h>

struct DriverPins
{
    const uint16_t DIR[4]          = {22, 4, 32, 27};
    const uint16_t STEP[4]         = {21, 16, 33, 14};
    const uint16_t EN[4]           = {17, 15, 26, 13};
    const uint16_t CS[4]           = {5, 2, 25, 12};
    const uint16_t LEDC_CHANNEL[4] = {0, 1, 2, 3};
};

#endif  // DRIVER_PINS_H