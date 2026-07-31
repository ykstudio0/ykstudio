//-------------------------------------------------------------
// File : SHT40Device.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 5.0.0
// Description : SHT40
//-------------------------------------------------------------

#pragma once

#include "DeviceBase.h"

namespace SVEMS::Device
{
    class SHT40Device : public DeviceBase
    {
    public:
        bool Begin() override;

        void Update() override;

        const char* GetName() const override{
            return "SHT40";
        }
    };
}