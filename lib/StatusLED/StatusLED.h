#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

#include "Pins.h"

enum class LedState
{
    Off,
    Boot,
    Ready,
    Charging,
    WiFiConnecting,
    RS485,
    Warning,
    Error
};

enum class OtaLedState
{
    Idle,
    Checking,
    Updating,
    Success,
    Failed
};

class StatusLED
{
public:
    static void Begin();
    static void Task();

    static void SetState(LedState state);
    static LedState GetState();

    static void SetOtaState(OtaLedState state);
    static OtaLedState GetOtaState();

private:
    static LedState currentState;

    static OtaLedState otaState;

    static uint32_t lastHeartbeatTime;
    static bool heartbeatActive;

    static uint32_t otaLastToggleTime;
    static bool otaLedOn;

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS =
        // 60UL * 1000UL;
        5UL * 1000UL;

    static constexpr uint32_t HEARTBEAT_ON_TIME_MS =
        150UL;

    static constexpr uint32_t OTA_CHECK_INTERVAL_MS =
        500UL;

    static constexpr uint32_t OTA_UPDATE_INTERVAL_MS =
        150UL;

    static void ShowColor(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );
};

#endif