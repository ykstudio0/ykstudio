//-------------------------------------------------------------
// File : RS485.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-03
// Project : SVEMS
// Version : 0.1.4
// Description : RS485 통신을 위한 클래스 구현
//-------------------------------------------------------------
#pragma once

#include <Arduino.h>
#include "Config.h"

class RS485
{
public:
    static bool Begin();
    static bool IsReady();
    static void TxMode();
    static void RxMode();
    static void Send(
        const uint8_t* data,
        size_t length);

    static bool ReceiveFrame(
        uint8_t* buffer,
        size_t& length,
        size_t maxLength,
        uint32_t timeout = MODBUS_TIMEOUT);

    static void ClearReceiveBuffer();

    static bool IsCommunicationError();

    static uint32_t GetConsecutiveNoRxCount();

    static void ResetCommunicationState();

private:
    static bool Ready;
    
    static bool DetermineFrameLength(
        const uint8_t* buffer,
        size_t receivedLength,
        size_t& expectedLength);

    static bool IsTimeOut(
        uint32_t startTime,
        uint32_t timeout);

    static uint32_t ConsecutiveNoRxCount;

    static constexpr uint32_t
        NO_RX_ERROR_THRESHOLD =
            3U;
};
