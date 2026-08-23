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

private:
    static bool InitializeLCD();    
};
