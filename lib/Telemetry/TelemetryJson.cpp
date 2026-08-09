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

namespace SVEMS::Telemetry
{

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

        battery["temperature"] =
            data.battery.temperature;

        battery["online"] =
            data.battery.online;

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

        communication["solarOffline"] =
            data.communication.solarOfflineCount;

        communication["chargeOffline"] =
            data.communication.chargeOfflineCount;

        communication["bmsOffline"] =
            data.communication.bmsOfflineCount;

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