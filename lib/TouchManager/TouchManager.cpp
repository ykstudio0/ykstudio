//-------------------------------------------------------------
// File : TouchManager.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Touch input state and event manager
//-------------------------------------------------------------

#include <Arduino.h>

#include "TouchManager.h"
#include "Logger.h"

namespace
{
    constexpr uint16_t SCREEN_WIDTH =
        320U;

    constexpr uint16_t SCREEN_HEIGHT =
        240U;

    constexpr uint32_t TAP_MAX_DURATION_MS =
        500UL;

    constexpr uint16_t TAP_MAX_MOVE_PIXELS =
        15U;
}

namespace SVEMS::Manager
{
    SVEMS::Touch::TouchPoint
        TouchManager::PressPoint = {};

    uint32_t
        TouchManager::PressStartedMs = 0U;
    
    SVEMS::Device::TouchDevice*
        TouchManager::Device = nullptr;

    bool TouchManager::Ready = false;

    bool TouchManager::PreviousTouched = false;

    bool TouchManager::CurrentTouched = false;

    SVEMS::Touch::Event
        TouchManager::CurrentEvent =
            SVEMS::Touch::Event::None;

    SVEMS::Touch::TouchPoint
        TouchManager::CurrentPoint = {};

    //---------------------------------------------------------
    // Begin
    //---------------------------------------------------------

    bool TouchManager::Begin(
        SVEMS::Device::TouchDevice& device)
    {
        Device =
            &device;

        Ready =
            Device->IsReady();

        PreviousTouched = false;
        CurrentTouched = false;

        CurrentEvent =
            SVEMS::Touch::Event::None;

        CurrentPoint = {};

        if (!Ready)
        {
            Logger::Error(
                "TOUCH MGR",
                "Device not ready");

            return false;
        }

        Logger::Info(
            "TOUCH MGR",
            "Ready");

        return true;
    }

    //---------------------------------------------------------
    // Update
    //---------------------------------------------------------

    bool TouchManager::Update()
    {
        CurrentEvent =
            SVEMS::Touch::Event::None;

        if (!Ready ||
            Device == nullptr)
        {
            return false;
        }

        if (!Device->Update())
        {
            CurrentTouched = false;

            return false;
        }

        CurrentTouched =
            Device->IsTouched();

        if (CurrentTouched)
        {
            uint16_t rawX = 0U;
            uint16_t rawY = 0U;

            if (!Device->ReadPoint(
                    rawX,
                    rawY))
            {
                return false;
            }

            ConvertPoint(
                rawX,
                rawY,
                CurrentPoint);
        }

        //-----------------------------------------------------
        // Pressed
        //-----------------------------------------------------

        if (!PreviousTouched &&
            CurrentTouched)
        {
            CurrentEvent =
                SVEMS::Touch::Event::Pressed;

            PressPoint =
                CurrentPoint;

            PressStartedMs =
                millis();
        }

        //-----------------------------------------------------
        // Released
        //-----------------------------------------------------

        else if (PreviousTouched &&
                 !CurrentTouched)
        {
             const uint32_t pressDuration =
                millis() -
                PressStartedMs;

            const int deltaX =
                abs(
                    static_cast<int>(CurrentPoint.x) -
                    static_cast<int>(PressPoint.x));

            const int deltaY =
                abs(
                    static_cast<int>(CurrentPoint.y) -
                    static_cast<int>(PressPoint.y));

            const bool isTap =
                pressDuration <=
                    TAP_MAX_DURATION_MS &&
                deltaX <=
                    TAP_MAX_MOVE_PIXELS &&
                deltaY <=
                    TAP_MAX_MOVE_PIXELS;

            CurrentEvent =
                isTap
                    ? SVEMS::Touch::Event::Tap
                    : SVEMS::Touch::Event::Released;
        }

        PreviousTouched =
            CurrentTouched;

        return true;
    }

    //---------------------------------------------------------
    // IsReady
    //---------------------------------------------------------

    bool TouchManager::IsReady()
    {
        return Ready;
    }

    //---------------------------------------------------------
    // IsTouched
    //---------------------------------------------------------

    bool TouchManager::IsTouched()
    {
        return
            Ready &&
            CurrentTouched;
    }

    //---------------------------------------------------------
    // GetEvent
    //---------------------------------------------------------

    SVEMS::Touch::Event
        TouchManager::GetEvent()
    {
        return CurrentEvent;
    }

    //---------------------------------------------------------
    // GetPoint
    //---------------------------------------------------------

    bool TouchManager::GetPoint(
        SVEMS::Touch::TouchPoint& point)
    {
        if (!Ready)
        {
            return false;
        }

        point =
            CurrentPoint;

        return true;
    }

    //---------------------------------------------------------
    // IsInside
    //---------------------------------------------------------

    bool TouchManager::IsInside(
        uint16_t x,
        uint16_t y,
        uint16_t width,
        uint16_t height)
    {
        if (!Ready ||
            width == 0U ||
            height == 0U)
        {
            return false;
        }

        const uint32_t right =
            static_cast<uint32_t>(x) +
            static_cast<uint32_t>(width);

        const uint32_t bottom =
            static_cast<uint32_t>(y) +
            static_cast<uint32_t>(height);

        return
            CurrentPoint.x >= x &&
            CurrentPoint.x < right &&
            CurrentPoint.y >= y &&
            CurrentPoint.y < bottom;
    }

    //---------------------------------------------------------
    // ConvertPoint
    //---------------------------------------------------------

    void TouchManager::ConvertPoint(
        uint16_t rawX,
        uint16_t rawY,
        SVEMS::Touch::TouchPoint& point)
    {
        // FT6336G raw portrait coordinates:
        //
        // Raw X : 0 ~ 239
        // Raw Y : 0 ~ 319
        //
        // Current LCD orientation:
        //
        // Screen X : 0 ~ 319
        // Screen Y : 0 ~ 239

        const int screenX =
            static_cast<int>(rawY);

        const int screenY =
            static_cast<int>(
                SCREEN_HEIGHT - 1U) -
            static_cast<int>(rawX);

        point.x =
            static_cast<uint16_t>(
                constrain(
                    screenX,
                    0,
                    static_cast<int>(
                        SCREEN_WIDTH - 1U)));

        point.y =
            static_cast<uint16_t>(
                constrain(
                    screenY,
                    0,
                    static_cast<int>(
                        SCREEN_HEIGHT - 1U)));
    }
}