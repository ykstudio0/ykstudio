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

bool DeviceManager::Ready = false;

bool DeviceManager::Begin()
{
    bool ok = true;

    // 기존 시스템 초기화
    ok &= Epever::Begin();
    ok &= Scheduler::Begin();

    // 등록된 모든 Device 초기화
    for (auto* device : g_devices)
    {
        if (device == nullptr)
        {
            continue;
        }

        const bool deviceOk = device->Begin();

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

    Ready = ok;

    if (Ready)
        Logger::Info("DEVICES", "Ready");
    else
        Logger::Error("DEVICES", "Init Failed");
        
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

    if (!SVEMS::Service::TimeService::Update())
    {
        return false;
    }

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