//-------------------------------------------------------------
// File : Display.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : Display Service
//-------------------------------------------------------------

#pragma once

#include "DisplayPages.h"
#include "DeviceManager.h"
#include "DeviceConfigurationStorage.h"

class Display
{
public:
    static bool Begin();
    static void Update();

    static void NextPage();
    static void PreviousPage();

    static void NextSubPage();

    static DisplayPages::Page CurrentPage();

    static void SetWiFiSetupConfirm(
        bool visible);

    static bool IsWiFiSetupConfirm();

    static void SetWiFiSetupMode(
        bool active);

    static void SetDeviceConfigMode(
        bool visible);

    static bool IsDeviceConfigMode();

    static void ToggleDeviceMppt();
    static void ToggleDeviceBms();
    static void ToggleDeviceSht40();
    static void ToggleDeviceRtc();

    static bool SaveDeviceConfig();

    static void CancelDeviceConfig();

private:
    static bool InitializeLCD();    
};
