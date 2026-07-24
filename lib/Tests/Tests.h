//-------------------------------------------------------------
// File : Tests.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-21
// Project : SVEMS
// Version : 0.3.0
// Description : SVEMS test entry points
//-------------------------------------------------------------

#pragma once

class IRenderTarget;

namespace Tests
{
    void RunDisplayTests();
    
    void RunDisplayThemeTests();

    void RunDisplayModelTests();

    void RunDisplayRendererTests(
        IRenderTarget& target);
        
    // void RunDisplayRendererTests();
}