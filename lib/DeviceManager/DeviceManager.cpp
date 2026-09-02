//-------------------------------------------------------------
// File : DeviceManager.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-13
// Project : SVEMS
// Version : 0.1.9
// Description : Device 관리
//-------------------------------------------------------------

#include "DeviceManager.h"
#include "Logger.h"
#include "Epever.h"
#include "Scheduler.h"

#include "DeviceBase.h"
#include "DS3231Device.h"
#include "SHT40Device.h"
#include "TimeService.h"
#include "EnvironmentService.h"
#include "DataManager.h"
#include "DeviceConfigurationStorage.h"
#include "RS485.h"
#include "ModbusRTU.h"
#include "BMSService.h"

namespace
{
    // 실제 Device 객체
    SVEMS::Device::DS3231Device g_ds3231;
    SVEMS::Device::SHT40Device  g_sht40;

    // DeviceManager가 공통 인터페이스로 관리할 Device 목록
    SVEMS::Device::DeviceBase* g_devices[] =
    {
        &g_ds3231,
        &g_sht40
    };
}

SVEMS::Device::DeviceConfiguration
    DeviceManager::Configuration;

const SVEMS::Device::DeviceConfiguration&
    DeviceManager::GetConfiguration()
{
    return Configuration;
}

void DeviceManager::SetConfiguration(
    const SVEMS::Device::DeviceConfiguration& config)
{
    const bool mpptChanged =
        Configuration.mppt !=
        config.mppt;

    Configuration =
        config;

    if (mpptChanged)
    {
        RS485::ResetCommunicationState();

        ModbusRTU::ResetCommunicationState();

        Logger::Info(
            "DEV CFG",
            "RS485/MOD State Reset");
    }
}

bool DeviceManager::Ready = false;

bool DeviceManager::Begin()
{
    if (!SVEMS::Device::DeviceConfigurationStorage::Load(
            Configuration))
    {
        Logger::Warning(
            "DEV CFG",
            "Load Failed - Using Defaults");
    }

    bool ok = true;

    //-------------------------------------------------
    // MPPT
    //-------------------------------------------------

    if (Configuration.mppt)
    {
        ok &= Epever::Begin();
    }
    else
    {
        Logger::Info(
            "EPEVER",
            "Not Used");
    }

    //-------------------------------------------------
    // BMS
    //-------------------------------------------------

    if constexpr (ENABLE_BMS_SERVICE)
    {
        if (Configuration.bms)
        {
            const bool bmsOk =
                SVEMS::Service::BMSService::Begin();

            if (!bmsOk)
            {
                Logger::Error(
                    "BMS",
                    "Init Failed");
            }

            ok &= bmsOk;
        }
        else
        {
            Logger::Info(
                "BMS",
                "Not Used");
        }
    }

    //-------------------------------------------------
    // Scheduler
    //-------------------------------------------------

    ok &= Scheduler::Begin();

    //-------------------------------------------------
    // Registered Devices
    //-------------------------------------------------

    for (auto* device : g_devices)
    {
        if (device == nullptr)
        {
            continue;
        }

        bool enabled = true;

        if (device == &g_ds3231)
        {
            enabled = Configuration.rtc;
        }
        else if (device == &g_sht40)
        {
            enabled = Configuration.sht40;
        }

        if (!enabled)
        {
            Logger::Info(
                device->GetName(),
                "Not Used");

            continue;
        }

        const bool deviceOk =
            device->Begin();

        if (deviceOk)
        {
            Logger::Info(
                device->GetName(),
                "Ready");
        }
        else
        {
            Logger::Error(
                device->GetName(),
                "Init Failed");
        }

        ok &= deviceOk;
    }

    //-------------------------------------------------
    // Time Service
    //-------------------------------------------------

    if (Configuration.rtc)
    {
        if (g_ds3231.IsOnline())
        {
            const bool timeServiceOk =
                SVEMS::Service::TimeService::Begin(
                    g_ds3231);

            ok &= timeServiceOk;
        }
        else
        {
            ok = false;
        }
    }

    //-------------------------------------------------
    // Environment Service
    //-------------------------------------------------

    if (Configuration.sht40)
    {
        if (g_sht40.IsOnline())
        {
            const bool environmentServiceOk =
                SVEMS::Service::EnvironmentService::Begin(
                    g_sht40);

            ok &= environmentServiceOk;
        }
        else
        {
            ok = false;
        }
    }

    Ready = ok;

    if (Ready)
    {
        Logger::Info(
            "DEVICES",
            "Ready");
    }
    else
    {
        Logger::Error(
            "DEVICES",
            "Init Failed");
    }

    return Ready;
}

bool DeviceManager::Update()
{
    if (!Ready)
    {
        return false;
    }

    // 등록된 모든 Device 갱신
    for (auto* device : g_devices)
    {
        if (device != nullptr)
        {
            device->Update();
        }
    }

    // if (!SVEMS::Service::TimeService::Update())
    // {
    //     return false;
    // }

    if (!SVEMS::Service::EnvironmentService::Update())
    {
        // 첫 측정 전 valid=false 일 수 있으므로
        // 전체 DeviceManager를 실패 처리하지는 않는다.
    }

    return true;
}

bool DeviceManager::IsReady()
{
    return Ready;
}

uint8_t DeviceManager::GetExpectedDeviceCount()
{
    uint8_t count = 0U;

    if (Configuration.mppt)
    {
        ++count;
    }

    if (Configuration.bms)
    {
        ++count;
    }

    if (Configuration.sht40)
    {
        ++count;
    }

    if (Configuration.rtc)
    {
        ++count;
    }

    return count;
}

bool DeviceManager::IsRs485Required()
{
    return Configuration.mppt;
}

bool DeviceManager::IsRTCOnline()
{
    return g_ds3231.IsOnline();
}

bool DeviceManager::IsSHT40Online()
{
    return g_sht40.IsOnline();
}

uint8_t DeviceManager::GetOnlineDeviceCount()
{
    uint8_t count = 0U;

    if (
        Configuration.mppt &&
        DataManager::Solar.status.online)
    {
        ++count;
    }

    if (
        Configuration.bms &&
        DataManager::Battery.status.online)
    {
        ++count;
    }

    if (
        Configuration.sht40 &&
        DeviceManager::IsSHT40Online())
    {
        ++count;
    }

    if (
        Configuration.rtc &&
        DeviceManager::IsRTCOnline())
    {
        ++count;
    }

    return count;
}