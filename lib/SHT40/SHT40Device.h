//-------------------------------------------------------------
// File : SHT40Device.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.3
// Description : SHT40 measurement data
//-------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "DeviceBase.h"
#include "SHT40Data.h"

namespace SVEMS::Device
{
    class SHT40Device : public DeviceBase
    {
    public:
        bool Begin() override;

        void Update() override;

        const SHT40Data& GetData() const;

        const char* GetName() const override{
            return "SHT40";
        }
    
    private:
        static constexpr uint8_t DEVICE_ADDRESS = 0x44;

        static constexpr uint8_t COMMAND_MEASURE_HIGH_PRECISION =
            0xFD;

        bool Probe();

        bool StartMeasurement();

        bool ReadMeasurement(
            uint8_t* buffer,
            uint8_t length);

        uint8_t CalculateCRC(
            const uint8_t* data,
            uint8_t length) const;

        bool CheckCRC(
            const uint8_t* buffer) const;

        void DecodeMeasurement(
            const uint8_t* buffer);

        SHT40Data m_data;
    };
}