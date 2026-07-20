//-------------------------------------------------------------
// File : Display.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Service
//-------------------------------------------------------------

#pragma once

class Display
{
public:
    static bool Begin();
    static void Update();

private:
    static bool InitializeLCD();    
    static void DrawMainScreen();
    
    static void DrawHeader();
    static void DrawSolarBlock();
    static void DrawBatteryBlock();
    static void DrawLoadBlock();
    static void DrawSocBlock();
    static void DrawTemperatureBlock();
    static void DrawFooter();
};
