#ifndef SPI_H
#define SPI_H

#include <Arduino.h>

struct SPIPins
{
    const uint16_t MOSI = 23;
    const uint16_t MISO = 19;
    const uint16_t SCK  = 18;
};

#endif  // SPI_H