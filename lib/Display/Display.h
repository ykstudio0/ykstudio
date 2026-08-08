//-------------------------------------------------------------
// File : Display.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : Display Service
//-------------------------------------------------------------

#pragma once

class Display
{
public:
    static bool Begin();
    static void Update();

    static void NextPage();
    static void PreviousPage();

    static void NextSubPage();

private:
    static bool InitializeLCD();    
};
