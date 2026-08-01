//-------------------------------------------------------------
// File : DataManger.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-18
// Project : SVEMS
// Version : 0.2.5
// Description : Data 관리
//-------------------------------------------------------------

#pragma once
#include <stdint.h>
#include "EpeverStatusParser.h"

class DataManager
{
public:
    struct Status
    {
        bool updated = false;
        bool online = false;
        uint32_t lastUpdate = 0;
    };

    struct SolarData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        Status status;
    };

    struct BatteryData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        Status status;
    };

    struct LoadData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        Status status;
    };

    struct TemperatureData
    {
        float battery = 0.0f;
        float device = 0.0f;

        Status status;
    };

    struct EnvironmentData
    {
        float temperature = 0.0f;
        float humidity = 0.0f;

        Status status;
    };

    struct SocData
    {
        uint8_t value = 0;

        Status status;
    };

    struct ChargeData
    {
        EpeverStatusParser::ChargingStage stage =
            EpeverStatusParser::ChargingStage::NotCharging;

        EpeverStatusParser::InputVoltageStatus inputVoltage =
            EpeverStatusParser::InputVoltageStatus::Normal;

        bool running = false;
        bool fault = false;
    };

    static SolarData Solar;
    static BatteryData Battery;
    static LoadData Load;
    static TemperatureData Temperature;
    static SocData Soc;
    static ChargeData Charge;
    static EnvironmentData Environment;

    static void ClearUpdates();
    static void UpdateOnlineStatus(uint32_t now);
};

