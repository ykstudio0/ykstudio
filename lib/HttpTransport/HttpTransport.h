//-------------------------------------------------------------
// File : HttpTransport.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Transport
{
    class HttpTransport
    {
    public:
        static bool Begin();

        static bool IsReady();

        static bool Send(
            const String& payload);

    private:
            static bool Ready;
    };
}