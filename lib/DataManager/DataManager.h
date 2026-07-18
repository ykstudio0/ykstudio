//-------------------------------------------------------------
// File : DataManger.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-18
// Project : SVEMS
// Version : 0.2.5
// Descripton : Data 관리
//-------------------------------------------------------------

#pragma once
#include <stdint.h>

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

        // bool updated = false;
        Status status;
    };

    struct BatteryData
    {
        float voltage = 0.0f;
        float current = 0.0f;

        // bool updated = false;
        Status status;
    };

    struct LoadData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        // bool updated = false;
        Status status;
    };

    struct TemperatureData
    {
        float batteryTemperature = 0.0f;
        float controllerTemperature = 0.0f;

        // bool updated = false;
        Status status;
    };

    struct SOCData
    {
        float socPercent = 0.0f;

        // bool updated = false;
        Status status;
    };

    static SolarData Solar;
    static BatteryData Battery;
    static LoadData Load;
    static TemperatureData Temperature;
    static SOCData Soc;

    static void ClearUpdates();
    static void UpdateOnlineStatus(uint32_t now);
};

