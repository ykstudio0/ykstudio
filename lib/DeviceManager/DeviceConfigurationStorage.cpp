//-------------------------------------------------------------
// File : DeviceConfigurationStorage.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-01
// Project : SVEMS
// Version : 0.7.2
// Description : Device Configuration
//-------------------------------------------------------------

#include "DeviceConfigurationStorage.h"

#include <Preferences.h>

#include "../Logger/Logger.h"

namespace SVEMS::Device
{
    namespace
    {
        constexpr const char* NVS_NAMESPACE =
            "device_cfg";

        constexpr const char* KEY_INITIALIZED =
            "initialized";

        constexpr const char* KEY_MPPT =
            "mppt";

        constexpr const char* KEY_BMS =
            "bms";

        constexpr const char* KEY_SHT40 =
            "sht40";

        constexpr const char* KEY_RTC =
            "rtc";
    }

    bool DeviceConfigurationStorage::Load(
        DeviceConfiguration& config)
    {
        Preferences preferences;

        if (!preferences.begin(
                NVS_NAMESPACE,
                true))
        {
            Logger::Error(
                "DEV CFG",
                "NVS Open Failed");

            return false;
        }

        const bool initialized =
            preferences.getBool(
                KEY_INITIALIZED,
                false);

        if (!initialized)
        {
            preferences.end();

            Logger::Info(
                "DEV CFG",
                "Using Defaults");

            return true;
        }

        config.mppt =
            preferences.getBool(
                KEY_MPPT,
                config.mppt);

        config.bms =
            preferences.getBool(
                KEY_BMS,
                config.bms);

        config.sht40 =
            preferences.getBool(
                KEY_SHT40,
                config.sht40);

        config.rtc =
            preferences.getBool(
                KEY_RTC,
                config.rtc);

        preferences.end();

        Logger::Info(
            "DEV CFG",
            "Loaded");

        return true;
    }

    bool DeviceConfigurationStorage::Save(
        const DeviceConfiguration& config)
    {
        Preferences preferences;

        if (!preferences.begin(
                NVS_NAMESPACE,
                false))
        {
            Logger::Error(
                "DEV CFG",
                "NVS Open Failed");

            return false;
        }

        preferences.putBool(
            KEY_MPPT,
            config.mppt);

        preferences.putBool(
            KEY_BMS,
            config.bms);

        preferences.putBool(
            KEY_SHT40,
            config.sht40);

        preferences.putBool(
            KEY_RTC,
            config.rtc);

        preferences.putBool(
            KEY_INITIALIZED,
            true);

        preferences.end();

        Logger::Info(
            "DEV CFG",
            "Saved");

        return true;
    }
}