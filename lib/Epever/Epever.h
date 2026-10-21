//-------------------------------------------------------------
// File : Epever.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Description : Epever 장치와의 통신을 위한 클래스 구현
//-------------------------------------------------------------

#ifndef EPEVER_H
#define EPEVER_H

#include <Arduino.h>
#include "EpeverMaps.h"

struct EpeverUpdateFlags
{
    bool solar = false;
    bool battery = false;
    bool load = false;
    bool temperature = false;
    bool soc = false;
};

struct EpeverData
{
    // Solar
    float pvVoltage = 0.0f;
    float pvCurrent = 0.0f;
    float pvPower = 0.0f;

    // Battery
    float batteryVoltage = 0.0f;
    float batteryCurrent = 0.0f;
    float batteryPower = 0.0f;

    uint8_t batterySOC = 0;

    // Load
    float loadVoltage = 0.0f;
    float loadCurrent = 0.0f;
    float loadPower = 0.0f;

    // Temperature
    float batteryTemperature = 0.0f;
    float controllerTemperature = 0.0f;

    // Status
    bool online = false;
    uint32_t lastUpdate = 0;

    // Update Flags
    EpeverUpdateFlags updated;
};

class Epever
{

public:
    static EpeverData Data;

    static bool Begin();
    static bool Update();
    static bool ReadSolar();
    static bool ReadBattery();
    static bool ReadLoad();
    static bool ReadTemperature();
    static bool ReadSOC();
    static void ClearUpdates();
    static bool ReadChargingStatus();
    static bool ReadEnergy();

    //---------------------------------------------------------
    // Process completed asynchronous Solar response
    //---------------------------------------------------------
    static bool ProcessAsyncSolarResponse();

    //---------------------------------------------------------
    // Start asynchronous Solar polling
    //---------------------------------------------------------
    static bool StartAsyncSolar();

    //---------------------------------------------------------
    // Handle asynchronous Solar communication failure
    //---------------------------------------------------------
    static void HandleAsyncSolarFailure();

    static bool StartAsyncBattery();
    static bool ProcessAsyncBatteryResponse();
    static void HandleAsyncBatteryFailure();

    static bool StartAsyncLoad();
    static bool ProcessAsyncLoadResponse();
    static void HandleAsyncLoadFailure();

    static bool StartAsyncChargingStatus();
    static bool ProcessAsyncChargingStatusResponse();
    static void HandleAsyncChargingStatusFailure();

    static bool StartAsyncTemperature();
    static bool ProcessAsyncTemperatureResponse();
    static void HandleAsyncTemperatureFailure();

    static bool StartAsyncSoc();
    static bool ProcessAsyncSocResponse();
    static void HandleAsyncSocFailure();

    static bool StartAsyncEnergy();
    static bool ProcessAsyncEnergyResponse();
    static void HandleAsyncEnergyFailure();

private:
    static bool ReadRegisters(
        uint16_t address,
        uint16_t count,
        uint16_t* values);

    //---------------------------------------------------------
    // Apply decoded Solar registers to DataManager
    //---------------------------------------------------------
    static void ApplySolarData(
        const EpeverMap::Solar& solar);

    static void ApplyBatteryData(
        const EpeverMap::Battery& battery);

    static void ApplyLoadData(
        const EpeverMap::Load& load);

    static void ApplyChargingStatus(
        const EpeverMap::ChargingStatus& status);

    static void ApplyTemperatureData(
        const EpeverMap::Temperature& temp);

    static void ApplySocData(const EpeverMap::Soc& soc);

    static void ApplyEnergyData(const EpeverMap::Energy& energy);

    //---------------------------------------------------------
    // Decode asynchronous Solar response
    //---------------------------------------------------------
    static bool DecodeSolarResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Solar& solar);

    static bool DecodeBatteryResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Battery& battery);

    static bool DecodeLoadResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Load& load);

    static bool DecodeChargingStatusResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::ChargingStatus& status);

    static bool DecodeTemperatureResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Temperature& temp);

    static bool DecodeSocResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Soc& soc);
    
    static bool DecodeEnergyResponse(
        const uint8_t* response,
        size_t length,
        EpeverMap::Energy& energy);
};

#endif