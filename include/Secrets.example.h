//-------------------------------------------------------------
// File : Secrets.example.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.6.0
// Description : Wifi
//-------------------------------------------------------------

#pragma once

namespace SVEMS::Config
{
    // Wi-Fi
    inline constexpr char WIFI_SSID[] =
        "YOUR_WIFI_SSID";

    inline constexpr char WIFI_PASSWORD[] =
        "YOUR_WIFI_PASSWORD";

    // MQTT (향후)
    inline constexpr char MQTT_SERVER[] =
        "";

    inline constexpr uint16_t MQTT_PORT =
        1883;

    inline constexpr char MQTT_USER[] =
        "";

    inline constexpr char MQTT_PASSWORD[] =
        "";
}