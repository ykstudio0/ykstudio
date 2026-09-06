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

class StatusLED
{
public:
    static void Begin();
    static void Task();

    static void SetState(LedState state);
    static LedState GetState();

private:
    static LedState currentState;

    static uint32_t lastHeartbeatTime;
    static bool heartbeatActive;

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS =
        // 60UL * 1000UL;
        10UL * 1000UL;

    static constexpr uint32_t HEARTBEAT_ON_TIME_MS =
        150UL;

    static void ShowColor(
        uint8_t r,
        uint8_t g,
        uint8_t b
    );
};

#endif