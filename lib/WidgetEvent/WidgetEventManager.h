//-------------------------------------------------------------
// File : WidgetEventManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Converts touch events into UI actions
//-------------------------------------------------------------

#pragma once

#include "TouchEvent.h"
#include "TouchPoint.h"
#include "UiAction.h"

namespace SVEMS::Manager
{
    class WidgetEventManager
    {
    public:
        static SVEMS::UI::Action Process(
            SVEMS::Touch::Event event,
            const SVEMS::Touch::TouchPoint& point);

    private:
        static SVEMS::UI::Action ProcessFooter(
            const SVEMS::Touch::TouchPoint& point);
        
        static SVEMS::UI::Action ProcessContent(
            const SVEMS::Touch::TouchPoint& point);

        static SVEMS::UI::Action ProcessHeader(
            const SVEMS::Touch::TouchPoint& point);

        static SVEMS::UI::Action ProcessWiFiSetupConfirm(
            const SVEMS::Touch::TouchPoint& point);

        static SVEMS::UI::Action ProcessDeviceConfig(
            const SVEMS::Touch::TouchPoint& point);

        static SVEMS::UI::Action ProcessWiFiSetupMode(
            const SVEMS::Touch::TouchPoint& point);
    };
}