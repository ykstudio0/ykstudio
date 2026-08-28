//-------------------------------------------------------------
// File : VehicleVoltageService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-28
// Project : SVEMS
// Version : 0.7.0
// Description : Vehicle Battery Charge Control
//-------------------------------------------------------------

#pragma once

namespace SVEMS
{
    namespace Vehicle
    {

        class VehicleVoltageService
        {
        public:
            static void Begin();
            static void Update();
        };

    } // namespace Vehicle
} // namespace SVEMS