//-------------------------------------------------------------
// File : HttpTransport.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------

#include "HttpTransport.h"
#include "Logger.h"
#include "WiFiService.h"

namespace SVEMS::Transport
{
    bool HttpTransport::Ready = false;

    bool HttpTransport::Begin()
    {
        Ready = true;

        Logger::Info(
            "HTTP",
            "Ready");

        return true;
    }

    bool HttpTransport::IsReady()
    {
        return Ready;
    }

    bool HttpTransport::Send(
        const String& payload)
    {
        if (!Ready)
        {
            return false;
        }

        if (!SVEMS::Service::WiFiService::IsConnected())
        {
            Logger::Warning(
                "HTTP",
                "WiFi Offline");
            
            return false;
        }

        if (payload.length() == 0U)
        {
            Logger::Warning(
                "HTTP",
                "Empty Payload");

            return false;
        }

        // Actual HTTP POST will be implemented later.
        Logger::Info(
            "HTTP",
            "Payload Ready");

        return true;
    }
}