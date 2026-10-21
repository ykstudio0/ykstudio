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

    //---------------------------------------------------------
    // Asynchronous Modbus State
    //---------------------------------------------------------
    enum class AsyncState : uint8_t
    {
        Idle,
        Waiting,
        Success,
        Failed
    };

    //---------------------------------------------------------
    // Asynchronous Modbus Transaction
    //---------------------------------------------------------
    struct AsyncTransaction
    {
        AsyncState state = AsyncState::Idle;

        uint8_t slave = 0U;
        uint16_t address = 0U;
        uint16_t count = 0U;

        uint32_t startMillis = 0U;

        uint8_t retryCount = 0U;
        bool recoveryAttempted = false;

        uint8_t response[64] = {};
        size_t responseLength = 0U;
        size_t expectedLength = 0U;
    };

    static bool Begin();
    static bool IsReady();

    static bool ReadInputRegisters(
        uint8_t slave,
        uint16_t address,
        uint16_t count,
        uint8_t* response,
        size_t responseSize);

    //---------------------------------------------------------
    // Asynchronous Modbus Interface
    //---------------------------------------------------------
    static bool StartReadInputRegisters(
        uint8_t slave,
        uint16_t address,
        uint16_t count);

    static void UpdateAsync();

    static AsyncState GetAsyncState();

    static bool CopyAsyncResponse(
        uint8_t* buffer,
        size_t bufferSize,
        size_t& length);

    static void ResetAsync();

    static ErrorReason GetLastErrorReason();

    static bool IsCommunicationError();

    static uint32_t
        GetConsecutiveFrameErrorCount();

    static void ResetCommunicationState();

private:
    static bool Ready;

    static ErrorReason LastErrorReason;

    static uint32_t ConsecutiveFrameErrorCount;

    //---------------------------------------------------------
    // Asynchronous Modbus Transaction
    //---------------------------------------------------------
    static AsyncTransaction Transaction;

    static constexpr uint32_t
        FRAME_ERROR_THRESHOLD =
            3U;

    static void RecordFrameError();

    //---------------------------------------------------------
    // Asynchronous Modbus - Request Transmission
    //---------------------------------------------------------
    static void SendAsyncRequest();
};

#endif