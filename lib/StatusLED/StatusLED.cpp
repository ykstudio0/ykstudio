//-------------------------------------------------------------
// File : StatusLED.cpp
// Author : JongOh Kim + ChatGPT
// Project : SVEMS
// Description : Status LED / Heart Signal
//-------------------------------------------------------------

#include "StatusLED.h"

#include "Config.h"

#include <Adafruit_NeoPixel.h>

namespace
{
    Adafruit_NeoPixel pixel(
        STATUS_LED_COUNT,
        PIN_STATUS_LED,
        NEO_GRB + NEO_KHZ800
    );
}

LedState StatusLED::currentState =
    LedState::Off;

uint32_t StatusLED::lastHeartbeatTime =
    0;

bool StatusLED::heartbeatActive =
    false;

// ---------------------------------------------------------
// Begin
// ---------------------------------------------------------

void StatusLED::Begin()
{
    pixel.begin();

    pixel.setBrightness(
        STATUS_LED_BRIGHTNESS
    );

    pixel.clear();
    pixel.show();

    SetState(
        LedState::Off
    );

    lastHeartbeatTime =
        millis();

    heartbeatActive =
        false;
}

// ---------------------------------------------------------
// Task
// ---------------------------------------------------------

void StatusLED::Task()
{
    const uint32_t now =
        millis();

    // -----------------------------------------------------
    // Heart Signal Start
    // -----------------------------------------------------

    if (
        !heartbeatActive &&
        now - lastHeartbeatTime >=
            HEARTBEAT_INTERVAL_MS
    )
    {
        // Soft green heartbeat
        ShowColor(
            0,
            80,
            0
        );

        heartbeatActive =
            true;

        lastHeartbeatTime =
            now;
    }

    // -----------------------------------------------------
    // Heart Signal End
    // -----------------------------------------------------

    if (
        heartbeatActive &&
        now - lastHeartbeatTime >=
            HEARTBEAT_ON_TIME_MS
    )
    {
        ShowColor(
            0,
            0,
            0
        );

        heartbeatActive =
            false;

        lastHeartbeatTime =
            now;
    }
}

// ---------------------------------------------------------
// GetState
// ---------------------------------------------------------

LedState StatusLED::GetState()
{
    return currentState;
}

// ---------------------------------------------------------
// SetState
// ---------------------------------------------------------

void StatusLED::SetState(
    LedState state
)
{
    currentState =
        state;

    switch (state)
    {
        case LedState::Off:
            ShowColor(
                0,
                0,
                0
            );
            break;

        case LedState::Boot:
            ShowColor(
                0,
                0,
                255
            );
            break;

        case LedState::Ready:
            ShowColor(
                0,
                255,
                0
            );
            break;

        case LedState::Charging:
            ShowColor(
                0,
                100,
                0
            );
            break;

        case LedState::WiFiConnecting:
            ShowColor(
                255,
                255,
                0
            );
            break;

        case LedState::RS485:
            ShowColor(
                255,
                0,
                255
            );
            break;

        case LedState::Warning:
            ShowColor(
                255,
                120,
                0
            );
            break;

        case LedState::Error:
            ShowColor(
                255,
                0,
                0
            );
            break;
    }
}

// ---------------------------------------------------------
// ShowColor
// ---------------------------------------------------------

void StatusLED::ShowColor(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    pixel.setPixelColor(
        0,
        pixel.Color(
            r,
            g,
            b
        )
    );

    pixel.show();
}