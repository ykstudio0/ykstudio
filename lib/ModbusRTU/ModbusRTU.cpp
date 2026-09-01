//-----------------------------------------------------
// File    : ModbusRTU.cpp
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-06
//-----------------------------------------------------

#include "ModbusRTU.h"
#include "Modbus.h"
#include "Config.h"
#include "CRC16.h"
#include "RS485.h"
#include "Logger.h"

bool ModbusRTU::Ready = false;

uint32_t ModbusRTU::ConsecutiveFrameErrorCount =
    0U;

ModbusRTU::ErrorReason
    ModbusRTU::LastErrorReason =
        ModbusRTU::ErrorReason::None;

void ModbusRTU::RecordFrameError()
{
    ++ConsecutiveFrameErrorCount;
}

bool ModbusRTU::Begin()
{   
    Ready = false;

    ConsecutiveFrameErrorCount =
        0U;

    if (!RS485::IsReady())
    {
        Logger::Error(
            "MODBUS",
            "RS485 Not Ready");

        return false;
    }

    Ready = true;

    Logger::Info("MODBUS", "Ready (" + String(MODBUS_BAUDRATE) + "bps)");
    
    return true;
}

bool ModbusRTU::ReadInputRegisters(
    uint8_t slave,
    uint16_t address,
    uint16_t count,
    uint8_t* response,
    size_t responseSize)
{
    static uint32_t seq = 0;

    LastErrorReason =
        ErrorReason::None;

    seq++;
    Logger::Info("MODBUS", "Read #" + String(seq));

    uint8_t frame[8];

    frame[0] = slave;
    frame[1] = MODBUS_READ_INPUT_REGISTERS;
    
    frame[2] = address >> 8;
    frame[3] = address & 0xFF;

    frame[4] = count >> 8;
    frame[5] = count & 0xFF;

    uint16_t crc = CRC16::Calculate(frame, 6);

    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;

    size_t len = 0;

    for (uint8_t retry = 0;
        retry <= MODBUS_RETRY;
        retry++)
    {
        len = 0;
        memset(response, 0, responseSize);

        Logger::Hex("TX", frame, sizeof(frame));

        RS485::Send(frame, sizeof(frame));

        bool received =
            RS485::ReceiveFrame(
                response,
                len,
                responseSize,
                MODBUS_TIMEOUT);

        //-------------------------------------------------
        // Partial Frame Recovery
        //
        // 일부 byte가 수신되었지만 완전한 Frame을
        // 받지 못한 경우 동일 요청을 1회 재전송한다.
        //
        // 구형 EPEVER Tracer-BP 통신 대응
        //-------------------------------------------------

        if (!received && len > 0)
        {
            Logger::Warning(
                "MODBUS",
                "Partial Frame - Recovery Retry");

            len = 0;

            memset(
                response,
                0,
                responseSize);

            Logger::Hex(
                "TX RECOVERY",
                frame,
                sizeof(frame));

            RS485::Send(
                frame,
                sizeof(frame));

            received =
                RS485::ReceiveFrame(
                    response,
                    len,
                    responseSize,
                    MODBUS_TIMEOUT);

            if (received)
            {
                Logger::Info(
                    "MODBUS",
                    "Partial Recovery OK");
            }
            else
            {
                Logger::Warning(
                    "MODBUS",
                    "Partial Recovery Failed");
            }
        }

        //-------------------------------------------------
        // 수신 실패
        //-------------------------------------------------

        if (!received)
        {
            if (retry < MODBUS_RETRY)
            {
                Logger::Info(
                    "MODBUS",
                    "Retry #" +
                    String(retry + 1));
            }

            continue;
        }

        break;
    }

    if (len == 0)
    {
        LastErrorReason =
            ErrorReason::NoResponse;

        ConsecutiveFrameErrorCount =
            0U;

        Logger::Warning(
            "MODBUS",
            "NO Response");

        return false;
    }

    Logger::Hex("RX", response, len);

    #if defined(SVEMS_MODBUS_TEST_ERROR)

        LastErrorReason =
            ErrorReason::InvalidSlave;

        RecordFrameError();

        Logger::Warning(
            "MODBUS",
            "TEST Frame Error");

        return false;

        #endif

    // 최소 프레임 확인
    if (len < 5)
    {
        LastErrorReason =
            ErrorReason::FrameTooShort;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Frame Too Short");

        return false;
    }

    // Slave 확인
    if (response[0] != slave)
    {
        LastErrorReason =
            ErrorReason::InvalidSlave;

        RecordFrameError();

        Logger::Error("MODBUS", "Invalid Slave");
        return false;
    }

    // Exception인지 먼저 구분
    const bool isException = 
        (response[1] & 0x80) != 0;

    size_t expectedLength = 0;

    if (isException)
    {
        expectedLength = 5;
    }
    else
    {
        if (response[1] != MODBUS_READ_INPUT_REGISTERS)
        {
            LastErrorReason =
                ErrorReason::InvalidFunction;

            RecordFrameError();

            Logger::Error("MODBUS", "Invalid Function");
            return false;
        }

        expectedLength =
            5U + static_cast<size_t>(response[2]);
    }

    if (len != expectedLength)
    {
        LastErrorReason =
            ErrorReason::InvalidLength;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Invalid Length (" +
            String(len) + "/" +
            String(expectedLength) + ")");
        
        return false;
    }

    // CRC 검사
    uint16_t crcCalc = 
        CRC16::Calculate(response, len - 2);

    uint16_t crcRecv =
        (uint16_t)response[len - 2] |
        ((uint16_t)response[len - 1] << 8);

    if (crcCalc != crcRecv)
    {
        LastErrorReason =
            ErrorReason::CrcError;

        RecordFrameError();

        char msg[48];

        sprintf(
            msg,
            "CRC Error (Recv=%04X Calc=%04X)",
            crcRecv,
            crcCalc);

        Logger::Error("MODBUS", msg);

        return false;
    }

    if (isException)
    {
        LastErrorReason =
            ErrorReason::Exception;

        RecordFrameError();

        char msg[32];

        sprintf(
            msg,
            "Exception %02X",
            response[2]);

        Logger::Error("MODBUS", msg);
        return false;
    }

    const uint8_t requestedBytes =
        static_cast<uint8_t>(count * 2U);
    
    if (response[2] != requestedBytes)
    {
        LastErrorReason =
            ErrorReason::InvalidByteCount;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Invalid Byte Count (" +
            String(response[2]) + "/" +
            String(requestedBytes) + ")");

        return false;
    }
    
    // if (response[1] & 0x80)
    // {
    //     char msg[32];

    //     sprintf(
    //         msg,
    //         "Exception %02X",
    //         response[2]);

    //     Logger::Error(
    //         "MODBUS",
    //         msg);

    //     return false;
    // }

    ConsecutiveFrameErrorCount =
        0U;
    
    LastErrorReason =
        ErrorReason::None;

    return true;
}

ModbusRTU::ErrorReason
    ModbusRTU::GetLastErrorReason()
{
    return LastErrorReason;
}

bool ModbusRTU::IsReady()
{
    return Ready;
}

bool ModbusRTU::IsCommunicationError()
{
    if (!Ready)
    {
        return false;
    }

    return
        ConsecutiveFrameErrorCount >=
        FRAME_ERROR_THRESHOLD;
}

uint32_t ModbusRTU::GetConsecutiveFrameErrorCount()
{
    return ConsecutiveFrameErrorCount;
}