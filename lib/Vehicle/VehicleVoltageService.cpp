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
            constexpr uint8_t SAMPLE_COUNT = 16;

            uint32_t milliVoltSum = 0;
            uint32_t rawSum = 0;

            for (
                uint8_t i = 0;
                i < SAMPLE_COUNT;
                ++i
            )
            {
                rawSum +=
                    analogRead(
                        PIN_VEHICLE_BAT_ADC
                    );

                milliVoltSum +=
                    analogReadMilliVolts(
                        PIN_VEHICLE_BAT_ADC
                    );
            }

            const uint16_t raw =
                static_cast<uint16_t>(
                    rawSum /
                    SAMPLE_COUNT
                );

            const uint32_t adcMilliVolts =
                milliVoltSum /
                SAMPLE_COUNT;

            const float adcVoltage =
                static_cast<float>(
                    adcMilliVolts
                ) / 1000.0f;

            constexpr float DIVIDER_RATIO =
                22.0f /
                (100.0f + 22.0f);

            const float vehicleVoltage =
                adcVoltage /
                DIVIDER_RATIO;

            DataManager::VehicleBattery.voltage =
                vehicleVoltage;

            DataManager::VehicleBattery.status.online =
                (
                    vehicleVoltage >= 6.0f &&
                    vehicleVoltage <= 16.0f
                );

            char buffer[96];

            snprintf(
                buffer,
                sizeof(buffer),
                "RAW=%u ADC=%lumV %.3fV BAT=%.2fV ONLINE=%s",
                static_cast<unsigned>(raw),
                static_cast<unsigned long>(adcMilliVolts),
                adcVoltage,
                vehicleVoltage,
                DataManager::VehicleBattery.status.online
                    ? "YES"
                    : "NO"
            );

            Logger::Info(
                "VEH BAT",
                buffer
            );
        }
    } // namespace Vehicle
} // namespace SVEMS