//-------------------------------------------------------------
// File : DeviceManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-13
// Project : SVEMS
// Version : 0.1.9
// Description : Device 관리
//-------------------------------------------------------------

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include "DeviceConfiguration.h"

class DeviceManager
{
public:
    static bool Begin();
    static bool Update();
    static bool IsReady();

    static bool IsRTCOnline();
    static bool IsSHT40Online();

    static uint8_t GetOnlineDeviceCount();

    static const SVEMS::Device::DeviceConfiguration&
        GetConfiguration();

    static void SetConfiguration(
        const SVEMS::Device::DeviceConfiguration& config);

    static uint8_t GetExpectedDeviceCount();

private:
    static bool Ready;

    static SVEMS::Device::DeviceConfiguration
        Configuration;
};

#endif