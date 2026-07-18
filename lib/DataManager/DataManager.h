//-------------------------------------------------------------
// File : DataManger.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-18
// Project : SVEMS
// Version : 0.2.5
// Descripton : Data 관리
//-------------------------------------------------------------

#pragma once

class DataManager
{
public:
    struct SolarData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        bool updated = false;
    };

    struct BatteryData
    {
        float voltage = 0.0f;
        float current = 0.0f;

        bool updated = false;
    };

    struct LoadData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power   = 0.0f;

        bool updated = false;
    };

    struct TemperatureData
    {
        float batteryTemperature = 0.0f;
        float controllerTemperature = 0.0f;

        bool updated = false;
    };

    struct SOCData
    {
        float socPercent = 0.0f;

        bool updated = false;
    };

    static SolarData Solar;
    static BatteryData Battery;
    static LoadData Load;
    static TemperatureData Temperature;
    static SOCData Soc;

    static void ClearUpdates();
};