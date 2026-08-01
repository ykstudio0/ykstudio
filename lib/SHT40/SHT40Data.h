//-------------------------------------------------------------
// File : SHT40Data.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.3
// Description : SHT40 measurement data
//-------------------------------------------------------------

#pragma once

namespace SVEMS::Device
{
    struct SHT40Data
    {
        float temperature = 0.0f;
        float humidity = 0.0f;

        bool valid = false;
    };
}