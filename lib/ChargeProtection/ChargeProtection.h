//-------------------------------------------------------------
// File : ChargeProtection.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-09
// Project : SVEMS
// Version : 0.8.6
// Description : ChargeProtection
//-------------------------------------------------------------

#pragma once

class ChargeProtection
{
public:
    static void Begin();
    static void Update();

private:
    static bool s_inhibit;
};