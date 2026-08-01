//-------------------------------------------------------------
// File : EnvironmentService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.3
// Description : Cabin environment service
//-------------------------------------------------------------

#pragma once

#include "SHT40Device.h"
#include "SHT40Data.h"

namespace SVEMS::Service
{
    class EnvironmentService
    {
    public:
        static bool Begin(
            SVEMS::Device::SHT40Device& sensor);

        static bool Update();

        static bool IsReady();

        static const SVEMS::Device::SHT40Data&
            GetData();

    private:
        static bool Ready;

        static SVEMS::Device::SHT40Device* Sensor;

        static SVEMS::Device::SHT40Data CurrentData;
    };
}