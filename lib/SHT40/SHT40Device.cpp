//-------------------------------------------------------------
// File : SHT40Device.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.0
// Description : SHT40
//-------------------------------------------------------------

#include <Wire.h>
#include "SHT40Device.h"
#include "Logger.h"

namespace SVEMS::Device
{
    const SHT40Data&
        SHT40Device::GetData() const
    {
        return m_data;
    }

    bool SHT40Device::Begin()
    {
        SetState(DeviceState::Initializing);

        if (!Probe())
        {
            SetState(DeviceState::Error);
            return false;
        }

        // 실제 SHT40 초기화는 저녁 하드웨어 테스트 때 작성
        SetState(DeviceState::Online);

        return true;
    }

    bool SHT40Device::Probe()
    {
        Wire.beginTransmission(DEVICE_ADDRESS);

        return Wire.endTransmission() == 0;
    }

    void SHT40Device::Update()
    {
        if (!IsOnline())
        {
            return;
        }

        if (!StartMeasurement())
        {
            SetState(DeviceState::Error);
            return;
        }

        delay(10);

        uint8_t buffer[6];

        if (!ReadMeasurement(buffer, sizeof(buffer)))
        {
            SetState(DeviceState::Error);
            return;
        }

        if (!CheckCRC(buffer))
        {
            SetState(DeviceState::Error);
            return;
        }

        DecodeMeasurement(buffer);

        SetState(DeviceState::Online);
    }

    bool SHT40Device::StartMeasurement()
    {
        Wire.beginTransmission(
            DEVICE_ADDRESS);

        Wire.write(
            COMMAND_MEASURE_HIGH_PRECISION);

        return
            Wire.endTransmission() == 0;
    }

    bool SHT40Device::ReadMeasurement(
        uint8_t* buffer,
        uint8_t length)
    {
        const uint8_t received =
            Wire.requestFrom(
                DEVICE_ADDRESS,
                length);

        if (received != length)
        {
            return false;
        }

        for (uint8_t i = 0; 1 < length; ++i)
        {
            buffer[i] = Wire.read();
        }

        Logger::Hex("SHT40", buffer, 6);

        return true;
    }

    uint8_t SHT40Device::CalculateCRC(
        const uint8_t* data,
        uint8_t length) const
    {
        uint8_t crc = 0xFF;

        for (uint8_t i = 0; i < length; ++i)
        {
            crc ^= data[i];

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                if (crc & 0x80)
                {
                    crc =
                        (crc << 1) ^ 0x31;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }

        return crc;
    }

    bool SHT40Device::CheckCRC(
        const uint8_t* buffer) const
    {
        // Temperature CRC
        if (CalculateCRC(buffer, 2) != buffer[2])
        {
            return false;
        }

        // Humidity CRC
        if (CalculateCRC(buffer + 3, 2) != buffer[5])
        {
            return false;
        }

        return true;
    }

    void SHT40Device::DecodeMeasurement(
        const uint8_t* buffer)
    {
        const uint16_t rawTemperature =
            (static_cast<uint16_t>(buffer[0]) << 8) |
            buffer[1];
        
        const uint16_t rawHumidity =
            (static_cast<uint16_t>(buffer[3]) << 8) |
            buffer[4];

        m_data.temperature =
            -45.0f +
            175.0f *
            static_cast<float>(rawTemperature) /
            65535.0f;

        m_data.humidity =
            -6.0f +
            125.0f *
            static_cast<float>(rawHumidity) /
            65535.0f;

        // 데이터시트 권장 범위로 보정
        if (m_data.humidity < 0.0f)
        {
            m_data.humidity = 0.0f;
        }

        if (m_data.humidity > 100.0f)
        {
            m_data.humidity = 100.0f;
        }

        m_data.valid = true;
    }
}