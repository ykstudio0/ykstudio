//-------------------------------------------------------------
// File : VehicleVoltageService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-28
// Project : SVEMS
// Version : 0.7.0
// Description : Vehicle Battery Charge Control
//-------------------------------------------------------------

#include "VehicleVoltageService.h"

#include <Arduino.h>

#include "Pins.h"
#include "DataManager.h"
#include "Logger.h"

namespace SVEMS
{
    namespace Vehicle
    {

        void VehicleVoltageService::Begin()
        {
            pinMode(PIN_VEHICLE_BAT_ADC, INPUT);
        }

        void VehicleVoltageService::Update()
        {
            const uint16_t raw = analogRead(PIN_VEHICLE_BAT_ADC);

            const float adcVoltage =
                static_cast<float>(raw) * 3.3f / 4095.0f;

            constexpr float DIVIDER_RATIO =
                22.0f / (100.0f + 22.0f);

            const float vehicleVoltage =
                adcVoltage / DIVIDER_RATIO;

            DataManager::VehicleBattery.voltage =
                vehicleVoltage;

            char buffer[64];

            snprintf(
                buffer,
                sizeof(buffer),
                "RAW=%u ADC=%.3fV BAT=%.2fV",
                static_cast<unsigned>(raw),
                adcVoltage,
                vehicleVoltage);

            Logger::Info(
                "VEH BAT",
                buffer);
        }

    } // namespace Vehicle
} // namespace SVEMS