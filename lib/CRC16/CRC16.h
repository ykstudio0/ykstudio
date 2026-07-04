#ifndef CRC16_H
#define CRC16_H

#include <Arduino.h>

class CRC16
{
public:
    static uint16_t Calculate(
        const uint8_t* data,
        size_t length);
};

#endif