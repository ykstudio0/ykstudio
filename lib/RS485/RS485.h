#ifndef RS485_H
#define RS485_H

#include <Arduino.h>
#include "Pins.h"

class RS485
{
public:
    static bool Begin();
    static void TxMode();
    static void RxMode();
    static void Send(const uint8_t* data, size_t length);
    static bool Available();
    static uint8_t Read();

private:

};

#endif