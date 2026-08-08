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

namespace SVEMS::Manager
{
    // Process
    SVEMS::UI::Action WidgetEventManager::Process(
        SVEMS::Touch::Event event,
        const SVEMS::Touch::TouchPoint& point)
    {
        if (event !=
            SVEMS::Touch::Event::Tap)
        {
            return 
                SVEMS::UI::Action::None;
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
}