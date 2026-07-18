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

bool ModbusRTU::Begin()
{
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
        Logger::Hex("TX", frame, sizeof(frame));

        RS485::Send(frame, sizeof(frame));
    
        if (!RS485::ReceiveFrame(
            response,
            len,
            responseSize))
        {
            Logger::Info("MODBUS", "Retry #" + String(retry + 1));
            
            continue;
        }

        break;
    }

    if (len == 0)
    {
        Logger::Warning("MODBUS", "NO Response");

        return false;
    }

    // 최소 프레임 확인
    if (len < 5)
    {
        Logger::Error("MODBUS", "Frame Too Short");
        return false;
    }

    uint16_t crcCalc = 
        CRC16::Calculate(response, len - 2);

    uint16_t crcRecv =
        (uint16_t)response[len - 2] |
        ((uint16_t)response[len - 1] << 8);

    if (crcCalc != crcRecv)
    {
        char msg[40];

        sprintf(
            msg,
            "CRC Error (%04X/%04X)",
            crcRecv,
            crcCalc);
        Logger::Error("MODBUS", msg);

        return false;
    }
        
    // 기대 길이 계산
    size_t expectedLength = 3 + response[2] + 2;

    if (len != expectedLength)
    {
        Logger::Error(
            "MODBUS",
            "Invalid Length (" +
            String(len) +
            "/" +
            String(expectedLength) +
            ")");
        return false;
    }

    if (len > 0)
    {
        // Serial.printf("LEN = %u\r\n", len);
        Logger::Hex("RX", response, len);
    }
    
    if (response[1] & 0x80)
    {
        char msg[32];

        sprintf(
            msg,
            "Exception %02X",
            response[2]);

        Logger::Error(
            "MODBUS",
            msg);

        return false;
    }
    
    return true;
}
