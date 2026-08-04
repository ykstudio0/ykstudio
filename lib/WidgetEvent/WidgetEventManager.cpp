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

        return ProcessFooter(
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
}