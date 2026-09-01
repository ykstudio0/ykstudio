//-------------------------------------------------------------
// File : WidgetEventManager.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Converts touch events into UI actions
//-------------------------------------------------------------

#include "WidgetEventManager.h"
#include "DisplayLayout.h"
#include "TouchManager.h"
#include "Logger.h"
#include "Display.h"

namespace SVEMS::Manager
{
    SVEMS::UI::Action
    WidgetEventManager::ProcessDeviceConfig(
        const SVEMS::Touch::TouchPoint& point)
    {
        constexpr int16_t BUTTON_Y1 = 198;
        constexpr int16_t BUTTON_Y2 = 240;

        //-------------------------------------------------
        // WiFi Setup
        //-------------------------------------------------

        constexpr int16_t WIFI_X1 = 60;
        constexpr int16_t WIFI_X2 = 260;

        constexpr int16_t WIFI_Y1 = 38;
        constexpr int16_t WIFI_Y2 = 78;

        if (
            point.x >= WIFI_X1 &&
            point.x <  WIFI_X2 &&
            point.y >= WIFI_Y1 &&
            point.y <  WIFI_Y2)
        {
            return
                SVEMS::UI::Action::WiFiSetup;
        }
        
        //-------------------------------------------------
        // SAVE / CANCEL
        //-------------------------------------------------

        if (point.y >= BUTTON_Y1 &&
            point.y < BUTTON_Y2)
        {
            if (point.x < 160)
            {
                return
                    SVEMS::UI::Action::DeviceConfigSave;
            }

            return
                SVEMS::UI::Action::DeviceConfigCancel;
        }

        //-------------------------------------------------
        // Device Items
        //-------------------------------------------------

        constexpr int16_t ITEM_X1 = 25;
        constexpr int16_t ITEM_X2 = 180;

        constexpr int16_t MPPT_Y1  = 78;
        constexpr int16_t MPPT_Y2  = 108;

        constexpr int16_t BMS_Y1   = 108;
        constexpr int16_t BMS_Y2   = 138;

        constexpr int16_t SHT40_Y1 = 138;
        constexpr int16_t SHT40_Y2 = 168;

        constexpr int16_t RTC_Y1   = 168;
        constexpr int16_t RTC_Y2   = 198;

        if (point.x < ITEM_X1 ||
            point.x > ITEM_X2)
        {
            return
                SVEMS::UI::Action::None;
        }

        if (point.y >= MPPT_Y1 &&
            point.y < MPPT_Y2)
        {
            return
                SVEMS::UI::Action::DeviceMpptToggle;
        }

        if (point.y >= BMS_Y1 &&
            point.y < BMS_Y2)
        {
            return
                SVEMS::UI::Action::DeviceBmsToggle;
        }

        if (point.y >= SHT40_Y1 &&
            point.y < SHT40_Y2)
        {
            return
                SVEMS::UI::Action::DeviceSht40Toggle;
        }

        if (point.y >= RTC_Y1 &&
            point.y < RTC_Y2)
        {
            return
                SVEMS::UI::Action::DeviceRtcToggle;
        }

        return
            SVEMS::UI::Action::None;
    }
    
    // Process
    SVEMS::UI::Action WidgetEventManager::Process(
        SVEMS::Touch::Event event,
        const SVEMS::Touch::TouchPoint& point)
    {
        //-------------------------------------------------
        // Tap 이벤트만 처리
        //-------------------------------------------------

        if (event !=
            SVEMS::Touch::Event::Tap)
        {
            return
                SVEMS::UI::Action::None;
        }

        //-------------------------------------------------
        // WiFi Setup Confirm
        //-------------------------------------------------

        if (::Display::IsWiFiSetupConfirm())
        {
            return ProcessWiFiSetupConfirm(
                point);
        }

        //-------------------------------------------------
        // Device Config
        //-------------------------------------------------

        if (::Display::IsDeviceConfigMode())
        {
            return ProcessDeviceConfig(
                point);
        }

        //-------------------------------------------------
        // Header
        //-------------------------------------------------

        const SVEMS::UI::Action headerAction =
            ProcessHeader(
                point);

        if (headerAction !=
            SVEMS::UI::Action::None)
        {
            return headerAction;
        }

        //-------------------------------------------------
        // Footer
        //-------------------------------------------------

        const SVEMS::UI::Action footerAction =
            ProcessFooter(
                point);

        if (footerAction !=
            SVEMS::UI::Action::None)
        {
            return footerAction;
        }

        //-------------------------------------------------
        // Content Area
        //-------------------------------------------------

        return ProcessContent(
            point);
    }

    // ProcessFooter
    SVEMS::UI::Action WidgetEventManager::ProcessFooter(
        const SVEMS::Touch::TouchPoint& point)
    {
        if (DisplayLayout::FOOTER_PREVIOUS_RECT.Contains(
                point.x,
                point.y))
        {
            return
                SVEMS::UI::Action::PreviousPage;
        }

        if (DisplayLayout::FOOTER_NEXT_RECT.Contains(
                point.x,
                point.y))
        {
            return
                SVEMS::UI::Action::NextPage;
        }

        if (DisplayLayout::FOOTER_CENTER_RECT.Contains(
                point.x,
                point.y))
        {
            return
                SVEMS::UI::Action::SelectPage;
        }

        return
            SVEMS::UI::Action::None;
    }

    // SVEMS::UI::Action WidgetEventManager::ProcessContent(
    //     const SVEMS::Touch::TouchPoint& point)
    // {
    //     if (point.y < DisplayLayout::CONTENT_Y ||
    //         point.y >= DisplayLayout::FOOTER_Y)
    //     {
    //         return
    //             SVEMS::UI::Action::None;
    //     }

    //     return
    //         SVEMS::UI::Action::NextSubPage;
    // }
    SVEMS::UI::Action WidgetEventManager::ProcessContent(
        const SVEMS::Touch::TouchPoint& point)
    {
        char message[80];

        snprintf(
            message,
            sizeof(message),
            "Y=%u CONTENT_Y=%d FOOTER_Y=%d",
            static_cast<unsigned int>(point.y),
            DisplayLayout::CONTENT_Y,
            DisplayLayout::FOOTER_Y);

        Logger::Info(
            "CONTENT",
            message);

        if (point.y < DisplayLayout::CONTENT_Y ||
            point.y >= DisplayLayout::FOOTER_Y)
        {
            Logger::Info(
                "CONTENT",
                "Outside");

            return
                SVEMS::UI::Action::None;
        }

        Logger::Info(
            "CONTENT",
            "Inside");

        return
            SVEMS::UI::Action::NextSubPage;
    }

    SVEMS::UI::Action WidgetEventManager::ProcessHeader(
        const SVEMS::Touch::TouchPoint& point)
    {
        //-------------------------------------------------
        // System page only
        //-------------------------------------------------

        if (::Display::CurrentPage() !=
            DisplayPages::Page::System)
        {
            return SVEMS::UI::Action::None;
        }

        //-------------------------------------------------
        // Header status area
        //-------------------------------------------------

        const bool inStatusArea =
            point.x >=
                DisplayLayout::HEADER_STATUS_X &&
            point.x <
                DisplayLayout::HEADER_STATUS_X +
                DisplayLayout::HEADER_STATUS_WIDTH &&
            point.y >= 0 &&
            point.y <
                DisplayLayout::HEADER_HEIGHT;

        if (!inStatusArea)
        {
            return SVEMS::UI::Action::None;
        }

        return SVEMS::UI::Action::DeviceConfig;
    }

    SVEMS::UI::Action
    WidgetEventManager::ProcessWiFiSetupConfirm(
        const SVEMS::Touch::TouchPoint& point)
    {
        constexpr int16_t BUTTON_Y = 135;
        constexpr int16_t BUTTON_H = 28;

        constexpr int16_t CANCEL_X = 70;
        constexpr int16_t CANCEL_W = 75;

        constexpr int16_t OK_X = 175;
        constexpr int16_t OK_W = 75;

        const bool inButtonY =
            point.y >= BUTTON_Y &&
            point.y <
                BUTTON_Y +
                BUTTON_H;

        if (!inButtonY)
        {
            return
                SVEMS::UI::Action::None;
        }

        //-------------------------------------------------
        // CANCEL
        //-------------------------------------------------

        if (
            point.x >= CANCEL_X &&
            point.x <
                CANCEL_X +
                CANCEL_W
        )
        {
            return
                SVEMS::UI::Action::Cancel;
        }

        //-------------------------------------------------
        // OK
        //-------------------------------------------------

        if (
            point.x >= OK_X &&
            point.x <
                OK_X +
                OK_W
        )
        {
            return
                SVEMS::UI::Action::Confirm;
        }

        return
            SVEMS::UI::Action::None;
    }
}