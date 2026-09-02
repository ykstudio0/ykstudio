//-------------------------------------------------------------
// File : UiAction.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Common UI action type
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::UI
{
    enum class Action : uint8_t{
        None = 0,

        PreviousPage,
        NextPage,
        SelectPage,

        NextSubPage,

        OpenMenu,
        CloseMenu,

        Confirm,
        Cancel,

        WiFiSetup,
        WiFiSetupModeCancel,

        DeviceConfig,

        DeviceMpptToggle,
        DeviceBmsToggle,
        DeviceSht40Toggle,
        DeviceRtcToggle,

        DeviceConfigSave,
        DeviceConfigCancel
    };
}