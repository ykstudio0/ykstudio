//-------------------------------------------------------------
// File : DS3231Device.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.1
// Description : DS3231
//-------------------------------------------------------------

#pragma once

#include <Wire.h>

#include "DeviceBase.h"
#include "RTCDateTime.h"

namespace SVEMS::Device
{

    class DS3231Device : public DeviceBase
    {
    public:

        bool Begin() override;

        void Update() override;

        const char* GetName() const override;

        const RTCDateTime& GetTime() const
        {
            return m_time;
        }

    private:

        bool Probe();

        bool ReadRegisters(
            uint8_t startRegister,
            uint8_t* buffer,
            uint8_t length);

        bool WriteRegisters(
            uint8_t startRegister,
            const uint8_t* buffer,
            uint8_t length);

        bool ReadTime();

        bool SetTime(
            const RTCDateTime& time);

        void DecodeTime(
            const uint8_t* buffer);

        void EncodeTime(
            uint8_t* buffer) const;

        static uint8_t BcdToDecimal(uint8_t value);

        static uint8_t DecimalToBcd(uint8_t value);

    private:

        static constexpr uint8_t DEVICE_ADDRESS = 0x68;

        RTCDateTime m_time;

    };
}