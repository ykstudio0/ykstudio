//-------------------------------------------------------------
// File : DS3231Device.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.0
// Description : DS3231
//-------------------------------------------------------------

#include "DS3231Device.h"
#include <Wire.h>
#include "Logger.h"

namespace SVEMS::Device
{
    bool DS3231Device::Begin()
    {
        SetState(DeviceState::Initializing);

        if (!Probe())
        {
            SetState(DeviceState::Error);

            return false;
        }

        // RTCDateTime rtc;

        // rtc.year      = 2026;
        // rtc.month     = 7;
        // rtc.day       = 31;
        // rtc.dayOfWeek = 5;      // 토요일이라면 6 또는 사용하는 규칙에 맞게
        // rtc.hour      = 22;
        // rtc.minute    = 50;
        // rtc.second    = 0;

        // if (!SetTime(rtc))
        // {
        //     SetState(DeviceState::Error);
        //     return false;
        // }

        if (!ReadTime())
        {
            SetState(DeviceState::Error);
            return false;
        }

        SetState(DeviceState::Online);

        const RTCDateTime& time = GetTime();

        char message[32];

        snprintf(
            message,
            sizeof(message),
            "%04u-%02u-%02u %02u:%02u:%02u",
            time.year,
            time.month,
            time.day,
            time.hour,
            time.minute,
            time.second);

        Logger::Info("DS3231", message);

        return true;
    }

    bool DS3231Device::Probe()
    {
        Wire.beginTransmission(DEVICE_ADDRESS);

        return (Wire.endTransmission() == 0);
    }

    void DS3231Device::Update()
    {
        if (!IsOnline())
        {
            return;
        }
        
        if (!ReadTime())
        {
            SetState(DeviceState::Error);
        }

        // uint8_t buffer[7];

        // if (!ReadRegisters(0x00, buffer, sizeof(buffer)))
        // {
        //     return;
        // }
    }

    const char* DS3231Device::GetName() const
    {
        return "DS3231";
    }

    bool DS3231Device::ReadRegisters(
        uint8_t startRegister,
        uint8_t* buffer,
        uint8_t length)
    {
        Wire.beginTransmission(DEVICE_ADDRESS);
        Wire.write(startRegister);

        if (Wire.endTransmission(false) != 0)
        {
            return false;
        }

        uint8_t received =
            Wire.requestFrom(
                DEVICE_ADDRESS,
                length);

        if (received != length)
        {
            return false;
        }

        for (uint8_t i = 0; i < length; i++)
        {
            buffer[i] = Wire.read();
        }

        return true;
    }

    bool DS3231Device::WriteRegisters(
        uint8_t startRegister,
        const uint8_t* buffer,
        uint8_t length)
    {
        Wire.beginTransmission(DEVICE_ADDRESS);

        Wire.write(startRegister);

        for (uint8_t i = 0; i < length; i++)
        {
            Wire.write(buffer[i]);
        }

        return Wire.endTransmission() == 0;
    }

    uint8_t DS3231Device::BcdToDecimal(uint8_t value)
    {
        return ((value >> 4) * 10) + (value & 0x0F);
    }

    uint8_t DS3231Device::DecimalToBcd(uint8_t value)
    {
        return ((value / 10) << 4) | (value % 10);
    }

    void DS3231Device::DecodeTime(const uint8_t* buffer)
    {
        m_time.dayOfWeek = BcdToDecimal(buffer[3]);
        m_time.day       = BcdToDecimal(buffer[4]);
        m_time.month     = BcdToDecimal(buffer[5] & 0x1F);
        m_time.year      = 2000 + BcdToDecimal(buffer[6]);

        m_time.second    = BcdToDecimal(buffer[0]);
        m_time.minute    = BcdToDecimal(buffer[1]);
        m_time.hour      = BcdToDecimal(buffer[2] & 0x3F);
    }

    bool DS3231Device::ReadTime()
    {
        uint8_t buffer[7];

        if (!ReadRegisters(0x00, buffer, sizeof(buffer)))
        {
            return false;
        }

        DecodeTime(buffer);

        return true;
    }

    void DS3231Device::EncodeTime(uint8_t* buffer) const
    {
        buffer[0] = DecimalToBcd(m_time.second);
        buffer[1] = DecimalToBcd(m_time.minute);
        buffer[2] = DecimalToBcd(m_time.hour);

        buffer[3] = DecimalToBcd(m_time.dayOfWeek);
        buffer[4] = DecimalToBcd(m_time.day);
        buffer[5] = DecimalToBcd(m_time.month);
        buffer[6] = DecimalToBcd(m_time.year % 100);
    }

    bool DS3231Device::SetTime(const RTCDateTime& time)
    {
        m_time = time;

        uint8_t buffer[7];

        EncodeTime(buffer);

        return WriteRegisters(0x00, buffer, sizeof(buffer));
    }
}