//-------------------------------------------------------------
// File : WifiService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.6.0
// Description : Non-blocking WiFi connection service
//-------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>

#include "WiFiService.h"
#include "Secrets.h"
#include "Logger.h"

namespace
{
    constexpr uint32_t WIFI_RETRY_INTERVAL_MS =
        10000UL;

    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS =
        15000UL;
}

namespace SVEMS::Service
{
    bool WiFiService::Connecting = false;

    uint32_t WiFiService::LastAttemptMs = 0U;

    uint32_t WiFiService::ConnectionStartedMs = 0U;

    void WiFiService::Begin()
    {
        Connecting = false;
        LastAttemptMs = 0U;
        ConnectionStartedMs = 0U;

        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);

        StartConnection();
    }

    void WiFiService::Update()
    {
        const uint32_t now =
            millis();

        if (WiFi.status() == WL_CONNECTED)
        {
            if (Connecting)
            {
                Connecting = false;

                Logger::Info(
                    "WIFI",
                    "Connected");

                char message[48];

                snprintf(
                    message,
                    sizeof(message),
                    "IP: %s",
                    WiFi.localIP().toString().c_str());

                Logger::Info(
                    "WIFI",
                    message);

                snprintf(
                    message,
                    sizeof(message),
                    "RSSI: %ld dBm",
                    static_cast<long>(WiFi.RSSI()));

                Logger::Info(
                    "WIFI",
                    message);
            }

            return;
        }

        if (Connecting)
        {
            if ((now - ConnectionStartedMs) >=
                WIFI_CONNECT_TIMEOUT_MS)
            {
                Connecting = false;

                WiFi.disconnect(
                    false,
                    false);

                Logger::Warning(
                    "WIFI",
                    "Connect timeout");
            }

            return;
        }

        if ((now - LastAttemptMs) >=
            WIFI_RETRY_INTERVAL_MS)
        {
            StartConnection();
        }
    }

    bool WiFiService::IsConnected()
    {
        return
            WiFi.status() == WL_CONNECTED;
    }

    bool WiFiService::IsConnecting()
    {
        return Connecting;
    }

    int32_t WiFiService::GetRssi()
    {
        if (!IsConnected())
        {
            return 0;
        }

        return WiFi.RSSI();
    }

    void WiFiService::StartConnection()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Connecting = false;
            return;
        }

        LastAttemptMs =
            millis();

        ConnectionStartedMs =
            LastAttemptMs;

        Connecting = true;

        Logger::Info("WIFI", "Connecting...");
            
        WiFi.begin(
            SVEMS::Config::WIFI_SSID,
            SVEMS::Config::WIFI_PASSWORD);
    }
}