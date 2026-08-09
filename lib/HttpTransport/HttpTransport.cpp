//-------------------------------------------------------------
// File : HttpTransport.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------

#include <HTTPClient.h>

#include "HttpTransport.h"
#include "Logger.h"
#include "WiFiService.h"
#include <HTTPClient.h>

namespace SVEMS::Transport
{
    constexpr const char* TELEMETRY_URL =
        "http://192.168.0.78:8080/telemetry";

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

        HTTPClient http;

        if (!http.begin(
                TELEMETRY_URL))
        {
            Logger::Warning(
                "HTTP",
                "Begin Failed");

            return false;
        }

        http.addHeader(
            "Content-Type",
            "application/json");

        const int httpCode =
                http.POST(payload);

        if (httpCode > 0)
        {
            char message[48];

            snprintf(
                message,
                sizeof(message),
                "Response = %d",
                httpCode);

            Logger::Info(
                "HTTP",
                message);
        }
        else
        {
            Logger::Warning(
                "HTTP",
                "POST Failed");
        }

        http.end();

        return
            httpCode >= 200 &&
            httpCode < 300;
    }
}