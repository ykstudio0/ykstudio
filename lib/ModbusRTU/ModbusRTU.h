//---------------------------------------------------------
// File     : ModbusRTU.h
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-05
//---------------------------------------------------------

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>

class ModbusRTU
{
public:
    enum class ErrorReason : uint8_t
    {
        None = 0,
        NoResponse,
        FrameTooShort,
        InvalidSlave,
        InvalidFunction,
        InvalidLength,
        CrcError,
        Exception,
        InvalidByteCount
    };

    static bool Begin();
    static bool IsReady();

    static bool ReadInputRegisters(
        uint8_t slave,
        uint16_t address,
        uint16_t count,
        uint8_t* response,
        size_t responseSize);

    static ErrorReason GetLastErrorReason();

    static bool IsCommunicationError();

    static uint32_t
        GetConsecutiveFrameErrorCount();

    static void ResetCommunicationState();

private:
    static bool Ready;

    static ErrorReason LastErrorReason;

    static uint32_t ConsecutiveFrameErrorCount;

    static constexpr uint32_t
        FRAME_ERROR_THRESHOLD =
            3U;

    static void RecordFrameError();
};

#endif