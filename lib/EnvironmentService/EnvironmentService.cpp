//-------------------------------------------------------------
// File : EnvironmentService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.3
// Description : Cabin environment service
//-------------------------------------------------------------

#include <Arduino.h>

#include "EnvironmentService.h"
#include "DataManager.h"

namespace SVEMS::Service
{
    bool EnvironmentService::Ready = false;

    SVEMS::Device::SHT40Device*
        EnvironmentService::Sensor = nullptr;

    SVEMS::Device::SHT40Data
        EnvironmentService::CurrentData;

    bool EnvironmentService::Begin(
        SVEMS::Device::SHT40Device& sensor)
    {
        if (!sensor.IsOnline())
        {
            Ready = false;
            return false;
        }

        Sensor = &sensor;
        CurrentData = Sensor->GetData();

        Ready = true;

        return true;
    }

    bool EnvironmentService::Update()
    {
        if (!Ready ||
            Sensor == nullptr)
        {
            return false;
        }

        if (!Sensor->IsOnline())
        {
            Ready = false;
            return false;
        }

        CurrentData = Sensor->GetData();

        if (!CurrentData.valid)
        {
            return false;
        }

        DataManager::Environment.temperature =
            CurrentData.temperature;

        DataManager::Environment.humidity =
            CurrentData.humidity;

        DataManager::Environment.status.updated = true;
        DataManager::Environment.status.online = true;
        DataManager::Environment.status.lastUpdate = millis();

        return true;
    }

    bool EnvironmentService::IsReady()
    {
        return Ready;
    }

    const SVEMS::Device::SHT40Data&
        EnvironmentService::GetData()
    {
        return CurrentData;
    }
}