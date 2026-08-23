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
    // Process
    SVEMS::UI::Action WidgetEventManager::Process(
        SVEMS::Touch::Event event,
        const SVEMS::Touch::TouchPoint& point)
    {
        if (
            event ==
                SVEMS::Touch::Event::Tap &&
            ::Display::IsWiFiSetupConfirm()
        )
        {
            return ProcessWiFiSetupConfirm(
                point);
        }

        if (event !=
            SVEMS::Touch::Event::Tap)
        {
            return 
                SVEMS::UI::Action::None;
        }

        const SVEMS::UI::Action headerAction =
            ProcessHeader(
                point);

        if (headerAction !=
            SVEMS::UI::Action::None)
        {
            return headerAction;
        }

        //-------------------------------------------------
        // Footer 우선 처리
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

        return SVEMS::UI::Action::WiFiSetup;
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