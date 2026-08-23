//-------------------------------------------------------------
// File : WifiService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.5.5
// Description : Non-blocking WiFi connection service
//-------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>

#include "WiFiService.h"
#include "Secrets.h"
#include "Logger.h"
#include <Preferences.h>
#include "WebServer.h"

namespace
{
    constexpr uint32_t WIFI_RETRY_INTERVAL_MS =
        10000UL;

    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS =
        15000UL;

    WebServer SetupServer(80);
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

        if (WiFi.getMode() == WIFI_AP)
        {
            SetupServer.handleClient();
            return;
        }

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

        Logger::Info(
            "WIFI",
            "Connecting..."
        );

        Preferences preferences;

        preferences.begin(
            "wifi",
            true
        );

        String ssid =
            preferences.getString(
                "ssid",
                ""
            );

        String password =
            preferences.getString(
                "password",
                ""
            );

        preferences.end();

        //---------------------------------------------------------
        // Use default configuration when no saved SSID exists
        //---------------------------------------------------------

        if (ssid.length() == 0U)
        {
            ssid =
                SVEMS::Config::WIFI_SSID;

            password =
                SVEMS::Config::WIFI_PASSWORD;

            Logger::Info(
                "WIFI",
                "Using default config"
            );
        }
        else
        {
            Logger::Info(
                "WIFI",
                "Using saved config"
            );
        }

        Logger::Info(
            "WIFI",
            ssid.c_str()
        );

        char buffer[64];

        snprintf(
            buffer,
            sizeof(buffer),
            "SSID='%s' PW_LEN=%u",
            ssid.c_str(),
            static_cast<unsigned>(
                password.length()
            )
        );

        Logger::Info(
            "WIFI",
            buffer
        );

        WiFi.begin(
            ssid.c_str(),
            password.c_str()
        );
    }

    bool WiFiService::SaveConfig(
        const String& ssid,
        const String& password)
    {
        if (ssid.length() == 0U)
        {
            return false;
        }

        Preferences preferences;

        if (!preferences.begin(
                "wifi",
                false))
        {
            return false;
        }

        const size_t ssidWritten =
            preferences.putString(
                "ssid",
                ssid
            );

        preferences.putString(
            "password",
            password
        );

        preferences.end();

        return ssidWritten > 0U;
    }

    bool WiFiService::StartSetupMode()
    {
        WiFi.disconnect(
            true,
            false
        );

        WiFi.mode(
            WIFI_AP
        );

        const bool started =
            WiFi.softAP(
                "SVEMS-SETUP"
            );

        if (!started)
        {
            Logger::Warning(
                "WIFI",
                "Setup AP Failed"
            );

            return false;
        }

        Logger::Info(
            "WIFI",
            "Setup AP Started"
        );

        SetupServer.on(
            "/",
            HTTP_GET,
            []()
            {
                const char* html =
                    "<!DOCTYPE html>"
                    "<html>"
                    "<head>"
                    "<meta name='viewport' "
                    "content='width=device-width, initial-scale=1'>"
                    "<title>SVEMS WiFi Setup</title>"
                    "</head>"
                    "<body>"
                    "<h2>SVEMS WiFi Setup</h2>"
                    "<form method='POST' action='/save'>"
                    "<label>SSID</label><br>"
                    "<input type='text' name='ssid'><br><br>"
                    "<label>Password</label><br>"
                    "<input type='password' name='password'><br><br>"
                    "<button type='submit'>Save</button>"
                    "</form>"
                    "</body>"
                    "</html>";

                SetupServer.send(
                    200,
                    "text/html",
                    html
                );
            }
        );

        SetupServer.on(
            "/save",
            HTTP_POST,
            []()
            {
                const String ssid =
                    SetupServer.arg(
                        "ssid"
                    );

                const String password =
                    SetupServer.arg(
                        "password"
                    );

                if (ssid.length() == 0U)
                {
                    SetupServer.send(
                        400,
                        "text/plain",
                        "SSID is required."
                    );

                    return;
                }

                if (!WiFiService::SaveConfig(
                        ssid,
                        password))
                {
                    SetupServer.send(
                        500,
                        "text/plain",
                        "Save failed."
                    );

                    return;
                }

                SetupServer.send(
                    200,
                    "text/html",
                    "<!DOCTYPE html>"
                    "<html>"
                    "<head>"
                    "<meta name='viewport' "
                    "content='width=device-width, initial-scale=1'>"
                    "<style>"
                    "body {"
                        "font-family: Arial, sans-serif;"
                        "padding: 24px;"
                        "font-size: 22px;"
                    "}"
                    "h2 {"
                        "font-size: 36px;"
                        "margin-bottom: 24px;"
                    "}"
                    "p {"
                        "font-size: 24px;"
                        "line-height: 1.5;"
                    "}"
                    "</style>"
                    "</head>"
                    "<body>"
                    "<h2>Saved</h2>"
                    "<p>WiFi configuration saved.</p>"
                    "<p>SVEMS will restart...</p>"
                    "</body>"
                    "</html>"
                );

                delay(
                    1000
                );

                ESP.restart();
            }
        );

        SetupServer.begin();

        Logger::Info(
            "WIFI",
            "Setup WebServer Started"
        );

        return true;
    }
}