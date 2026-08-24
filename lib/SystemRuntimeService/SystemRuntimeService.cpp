//-------------------------------------------------------------
// File : SystemRuntimeService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-24
// Project : SVEMS
// Version : 0.6.4
// Description : System runtime diagnostic service
//-------------------------------------------------------------

#include "SystemRuntimeService.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>

#include "DataManager.h"
#include "Logger.h"


namespace
{
    DataManager::ResetReason ConvertResetReason(
        esp_reset_reason_t reason)
    {
        switch (reason)
        {
            case ESP_RST_POWERON:
                return DataManager::ResetReason::PowerOn;

            case ESP_RST_EXT:
                return DataManager::ResetReason::External;

            case ESP_RST_SW:
                return DataManager::ResetReason::Software;

            case ESP_RST_PANIC:
            case ESP_RST_INT_WDT:
            case ESP_RST_TASK_WDT:
            case ESP_RST_WDT:
                return DataManager::ResetReason::Watchdog;

            case ESP_RST_DEEPSLEEP:
                return DataManager::ResetReason::DeepSleep;

            case ESP_RST_BROWNOUT:
                return DataManager::ResetReason::Brownout;

            default:
                return DataManager::ResetReason::Unknown;
        }
    }

}


namespace SVEMS::Service
{
    bool SystemRuntimeService::Begin()
    {
        //-----------------------------------------------------
        // Boot Count
        //-----------------------------------------------------

        Preferences preferences;

        if (!preferences.begin(
                "system",
                false))
        {
            Logger::Error(
                "SYSTEM",
                "NVS Open Failed"
            );

            return false;
        }

        uint32_t bootCount =
            preferences.getUInt(
                "boot_count",
                0U
            );

        ++bootCount;

        preferences.putUInt(
            "boot_count",
            bootCount
        );

        preferences.end();

        DataManager::SystemRuntime.bootCount =
            bootCount;


        //-----------------------------------------------------
        // Reset Reason
        //-----------------------------------------------------
        DataManager::SystemRuntime.resetReason =
            ConvertResetReason(
                esp_reset_reason()
            );


        //-----------------------------------------------------
        // Log
        //-----------------------------------------------------

        char message[64];

        snprintf(
            message,
            sizeof(message),
            "Boot=%lu Reset=%s",
            static_cast<unsigned long>(
                DataManager::SystemRuntime.bootCount
            ),
            DataManager::ResetReasonToString(
                DataManager::SystemRuntime.resetReason
            )
        );

        Logger::Info(
            "SYSTEM",
            message
        );

        return true;
    }
}