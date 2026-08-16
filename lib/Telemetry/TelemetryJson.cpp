//-------------------------------------------------------------
// File : TelemetryJson.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : Telemetry JSON Serializer
//-------------------------------------------------------------

#include "TelemetryJson.h"

#include <ArduinoJson.h>

#include "EpeverStatusParser.h"

namespace
{
    const char* HttpStateToString(
        uint8_t state)
    {
        switch (state)
        {
            case 0U:
                return "Ready";

            case 1U:
                return "Sending";

            case 2U:
                return "Retry";

            default:
                return "Unknown";
        }
    }
}

namespace SVEMS::Telemetry
{
    static const char* ToString(
        TelemetryState state)
    {
        switch (state)
        {
            case TelemetryState::Online:
                return "Online";

            case TelemetryState::Stale:
                return "Stale";

            case TelemetryState::Offline:
            default:
                return "Offline";
        }
    }

    bool TelemetryJson::Serialize(
        const TelemetryData& data,
        String& output)
    {
        JsonDocument doc;

        char timestamp[24];

        snprintf(
            timestamp,
            sizeof(timestamp),
            "%04u-%02u-%02uT%02u:%02u:%02u",
            static_cast<unsigned>(
                data.timestamp.year),
            static_cast<unsigned>(
                data.timestamp.month),
            static_cast<unsigned>(
                data.timestamp.day),
            static_cast<unsigned>(
                data.timestamp.hour),
            static_cast<unsigned>(
                data.timestamp.minute),
            static_cast<unsigned>(
                data.timestamp.second));

        doc["timestamp"] =
            timestamp;

        //---------------------------------------------------------
        // Solar
        //---------------------------------------------------------

        JsonObject solar =
            doc["solar"].to<JsonObject>();

        solar["voltage"] =
            data.solar.voltage;

        solar["current"] =
            data.solar.current;

        solar["power"] =
            data.solar.power;

        doc["solar"]["dailyEnergy"] =
            data.solar.dailyEnergy;

        doc["solar"]["totalEnergy"] =
            data.solar.totalEnergy;

        solar["stage"] =
            EpeverStatusParser::ToString(
                data.solar.stage);

        solar["input"] =
            EpeverStatusParser::ToString(
                data.solar.inputVoltage);

        solar["online"] =
            data.solar.online;

        //---------------------------------------------------------
        // Battery
        //---------------------------------------------------------

        JsonObject battery =
            doc["battery"].to<JsonObject>();

        battery["voltage"] =
            data.battery.voltage;

        battery["current"] =
            data.battery.current;

        battery["power"] =
            data.battery.power;

        battery["soc"] =
            data.battery.soc;

        battery["remainingCapacity"] =
            data.battery.remainingCapacity;

        battery["totalCapacity"] =
            data.battery.totalCapacity;

        battery["temperature"] =
            data.battery.temperature;

        battery["online"] =
            data.battery.online;

        //---------------------------------------------------------
        // Load
        //---------------------------------------------------------

        JsonObject load =
            doc["load"].to<JsonObject>();

        load["voltage"] =
            data.load.voltage;

        load["current"] =
            data.load.current;

        load["power"] =
            data.load.power;

        load["online"] =
            data.load.online;

        //---------------------------------------------------------
        // Temperature
        //---------------------------------------------------------

        JsonObject temperature =
            doc["temperature"].to<JsonObject>();

        temperature["battery"] =
            data.temperature.battery;

        temperature["bms"] =
            data.temperature.bms;

        temperature["powerBankExternal"] =
            data.temperature.powerBankExternal;

        temperature["controller"] =
            data.temperature.controller;

        temperature["controllerBoard"] =
            data.temperature.controllerBoard;

        temperature["cabinState"] =
            ToString(
                data.temperature.cabinState
            );

        temperature["powerBankState"] =
            ToString(
                data.temperature.powerBankState
            );

        temperature["controllerState"] =
            ToString(
                data.temperature.controllerState
            );
            
        //---------------------------------------------------------
        // Environment
        //---------------------------------------------------------

        JsonObject environment =
            doc["environment"].to<JsonObject>();

        environment["temperature"] =
            data.environment.cabinTemperature;

        environment["humidity"] =
            data.environment.cabinHumidity;

        environment["online"] =
            data.environment.online;

        //---------------------------------------------------------
        // Communication
        //---------------------------------------------------------

        JsonObject communication =
            doc["communication"].to<JsonObject>();

        communication["rs485"] =
            data.communication.rs485Ready;

        communication["modbus"] =
            data.communication.modbusReady;

        communication["solarState"] =
            ToString(
                data.communication.solarState
            );

        communication["solarTimeout"] =
            data.communication.solarTimeoutCount;

        communication["solarOffline"] =
            data.communication.solarOfflineCount;

        communication["chargeState"] =
            ToString(
                data.communication.chargeState
            );

        communication["chargeTimeout"] =
            data.communication.chargeTimeoutCount;

        communication["chargeOffline"] =
            data.communication.chargeOfflineCount;

        communication["bmsOffline"] =
            data.communication.bmsOfflineCount;

        JsonObject http =
            communication["http"].to<JsonObject>();

        http["online"] =
            data.communication.http.online;

        http["state"] =
            HttpStateToString(
                data.communication.http.state);
        
        http["success"] =
            data.communication.http.successCount;

        http["failure"] =
            data.communication.http.failureCount;

        http["consecutiveFailures"] =
            data.communication.http.consecutiveFailures;

        //---------------------------------------------------------
        // System
        //---------------------------------------------------------

        JsonObject system =
            doc["system"].to<JsonObject>();

        system["uptime"] =
            data.system.uptimeSeconds;

        system["wifi"] =
            data.system.wifiConnected;

        system["rssi"] =
            data.system.wifiRssi;

        //---------------------------------------------------------
        // Serialize
        //---------------------------------------------------------

        output = "";

        const size_t written =
            serializeJson(
                doc,
                output);

        return written > 0U;
    }

}