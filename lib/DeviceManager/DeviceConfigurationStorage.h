//-------------------------------------------------------------
// File : DeviceConfigurationStorage.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-01
// Project : SVEMS
// Version : 0.7.2
// Description : Device Configuration
//-------------------------------------------------------------

#pragma once

#include "DeviceConfiguration.h"

namespace SVEMS::Device
{
    class DeviceConfigurationStorage
    {
    public:
        static bool Load(
            DeviceConfiguration& config);

        static bool Save(
            const DeviceConfiguration& config);
    };
}