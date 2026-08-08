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

        float remainingCapacity = 0.0f;

        float cellVoltage[4] =
        {
            0.0f,
            0.0f,
            0.0f,
            0.0f
        };

        uint8_t cellCount = 0U;

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
        // Quantumcat battery temperature
        float battery = 0.0f;

        // Quantumcat BMS temperature
        float bms = 0.0f;

        // Quantumcat external sensor temperature
        float powerBankExternal = 0.0f;

        // EPEVER controller temperature sensor
        float controller = 0.0f;

        // EPEVER controller board temperature
        float controllerBoard = 0.0f;

        // SHT40 source status
        Status cabinStatus;

        // Quantumcat BMS source status
        Status powerBankStatus;

        // EPEVER temperature source status
        Status controllerStatus;
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

        Status status;
    };

    struct ControllerBatteryData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        uint8_t soc = 0U;

        Status status;
    };

    static SolarData Solar;
    static BatteryData Battery;
    static ControllerBatteryData ControllerBattery;
    static LoadData Load;
    static TemperatureData Temperature;
    static SocData Soc;
    static ChargeData Charge;
    static EnvironmentData Environment;
    
    static void ClearUpdates();
    static void UpdateOnlineStatus(uint32_t now);
};