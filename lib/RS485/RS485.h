//-------------------------------------------------------------
// File : RS485.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-03
// Project : SVEMS
// Version : 0.1.4
// Descripton : RS485 통신을 위한 클래스 구현
//-------------------------------------------------------------

#ifndef RS485_H
#define RS485_H

#include <Arduino.h>
#include "Pins.h"
#include "Config.h"

class RS485
{
public:
    static bool Begin();
    static void TxMode();
    static void RxMode();
    static void Send(const uint8_t* data, size_t length);
    static bool Available();
    // static uint8_t Read();
    static size_t Receive(
        uint8_t* buffer,
        size_t maxLength,
        uint32_t timeout = MODBUS_TIMEOUT);
    static bool ReceiveFrame(
        uint8_t* buffer,
        size_t& length,
        size_t maxLength,
        uint32_t timeout = MODBUS_TIMEOUT);

private:

};

#endif