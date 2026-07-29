//-------------------------------------------------------------
// File : HeaderWidget.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Common display header widget implementation
//-------------------------------------------------------------

#include "HeaderWidget.h"

#include "DisplayLayout.h"
#include "DisplayTheme.h"

namespace DisplayWidgets
{
    void HeaderWidget::DrawStatic(
        IRenderTarget& target,
        const char* title)
    {
        // Header background
        target.FillRect(
            DisplayLayout::HEADER_X,
            DisplayLayout::HEADER_Y,
            DisplayLayout::HEADER_WIDTH,
            DisplayLayout::HEADER_HEIGHT,
            DisplayTheme::COLOR_HEADER_BACKGROUND);

        // Page title
        if (title != nullptr &&
            title[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::HEADER_TITLE_X,
                DisplayLayout::HEADER_TITLE_Y,
                title,
                DisplayTheme::COLOR_TITLE,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Title),
                DisplayTypes::TextAlign::Left);
        }

        // Header divider
        target.DrawLine(
            DisplayLayout::HEADER_X,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayLayout::HEADER_X +
                DisplayLayout::HEADER_WIDTH - 1,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayTheme::COLOR_DIVIDER,
            DisplayTheme::DIVIDER_WIDTH);
    }
    
    void HeaderWidget::Draw(
        IRenderTarget& target,
        const char* title,
        const char* energyStatusText,
        DisplayTheme::Color energyStatusColor,
        const char* timeText,
        const char* statusText,
        DisplayTheme::Color statusColor)
    {
        (void)title;
        // DrawStatic(
        //     target,
        //     title);

        // Energy status
        if (energyStatusText != nullptr &&
            energyStatusText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::HEADER_ENERGY_X,
                DisplayLayout::HEADER_ENERGY_Y,
                energyStatusText,
                energyStatusColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Large),
                DisplayTypes::TextAlign::Center);
        }
        
        // Current time
        if (timeText != nullptr &&
            timeText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::HEADER_TIME_X,
                DisplayLayout::HEADER_TIME_Y,
                timeText,
                DisplayTheme::COLOR_TEXT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Right);
        }

        // System status
        if (statusText != nullptr &&
            statusText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::HEADER_STATUS_X,
                DisplayLayout::HEADER_STATUS_Y,
                statusText,
                statusColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Left);
        }
    }
}