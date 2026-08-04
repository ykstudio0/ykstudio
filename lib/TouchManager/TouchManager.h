//-------------------------------------------------------------
// File : TouchManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Touch input state and event manager
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "TouchDevice.h"
#include "TouchPoint.h"
#include "TouchEvent.h"

namespace SVEMS::Manager
{
    class TouchManager
    {
    public:
        static bool Begin(
            SVEMS::Device::TouchDevice& device);

        static bool Update();

        static bool IsReady();

        static bool IsTouched();

        static SVEMS::Touch::Event GetEvent();

        static bool GetPoint(
            SVEMS::Touch::TouchPoint& point);

        static bool IsInside(
            uint16_t x,
            uint16_t y,
            uint16_t width,
            uint16_t height);

    private:
        static void ConvertPoint(
            uint16_t rawX,
            uint16_t rawY,
            SVEMS::Touch::TouchPoint& point);

        static SVEMS::Device::TouchDevice*
            Device;

        static bool Ready;
        static bool PreviousTouched;
        static bool CurrentTouched;

        static SVEMS::Touch::Event
            CurrentEvent;

        static SVEMS::Touch::TouchPoint
            CurrentPoint;

        static SVEMS::Touch::TouchPoint PressPoint;

        static uint32_t PressStartedMs;
    };
}