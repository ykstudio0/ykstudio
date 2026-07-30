//-------------------------------------------------------------
// File : HeaderWidget.cpp
// Author : JongOh Kim + ChatGPT
// Project : SVEMS
// Version : 0.4.6
// Description : Common Header Widget
//-------------------------------------------------------------

#include "HeaderWidget.h"

#include "DisplayLayout.h"
#include "DisplayTheme.h"

namespace DisplayWidgets
{

    //-------------------------------------------------------------
    // DrawStatic
    //-------------------------------------------------------------
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

        // Divider
        target.DrawLine(
            DisplayLayout::HEADER_X,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayLayout::HEADER_X +
                DisplayLayout::HEADER_WIDTH - 1,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayTheme::COLOR_DIVIDER,
            DisplayTheme::DIVIDER_WIDTH);
    }

    //-------------------------------------------------------------
    // DrawEnergy
    //-------------------------------------------------------------
    void HeaderWidget::DrawEnergy(
        IRenderTarget& target,
        const char* energyStatusText,
        DisplayTheme::Color energyStatusColor)
    {
        target.FillRect(
            DisplayLayout::HEADER_ENERGY_X -
                DisplayLayout::HEADER_ENERGY_WIDTH / 2,
            DisplayLayout::HEADER_Y,
            DisplayLayout::HEADER_ENERGY_WIDTH,
            DisplayLayout::HEADER_HEIGHT,
            DisplayTheme::COLOR_HEADER_BACKGROUND);

        if (energyStatusText == nullptr ||
            energyStatusText[0] == '\0')
        {
            return;
        }

        target.DrawTextBg(
            DisplayLayout::HEADER_ENERGY_X,
            DisplayLayout::HEADER_ENERGY_Y,
            energyStatusText,
            energyStatusColor,
            DisplayTheme::COLOR_HEADER_BACKGROUND,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Large),
            DisplayTypes::TextAlign::Center);
    }

    //-------------------------------------------------------------
    // DrawTime
    //-------------------------------------------------------------
    void HeaderWidget::DrawTime(
        IRenderTarget& target,
        const char* timeText)
    {
        target.FillRect(
            DisplayLayout::HEADER_TIME_X -
                DisplayLayout::HEADER_TIME_WIDTH,
            DisplayLayout::HEADER_Y,
            DisplayLayout::HEADER_TIME_WIDTH,
            DisplayLayout::HEADER_HEIGHT,
            DisplayTheme::COLOR_HEADER_BACKGROUND);

        if (timeText == nullptr ||
            timeText[0] == '\0')
        {
            return;
        }

        target.DrawTextBg(
            DisplayLayout::HEADER_TIME_X,
            DisplayLayout::HEADER_TIME_Y,
            timeText,
            DisplayTheme::COLOR_TEXT,
            DisplayTheme::COLOR_HEADER_BACKGROUND,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Small),
            DisplayTypes::TextAlign::Right);
    }

    //-------------------------------------------------------------
    // DrawStatus
    //-------------------------------------------------------------
    void HeaderWidget::DrawStatus(
        IRenderTarget& target,
        const char* statusText,
        DisplayTheme::Color statusColor)
    {
        target.FillRect(
            DisplayLayout::HEADER_STATUS_X,
            DisplayLayout::HEADER_Y,
            DisplayLayout::HEADER_STATUS_WIDTH,
            DisplayLayout::HEADER_HEIGHT,
            DisplayTheme::COLOR_HEADER_BACKGROUND);

        if (statusText == nullptr ||
            statusText[0] == '\0')
        {
            return;
        }

        target.DrawTextBg(
            DisplayLayout::HEADER_STATUS_X,
            DisplayLayout::HEADER_STATUS_Y,
            statusText,
            statusColor,
            DisplayTheme::COLOR_HEADER_BACKGROUND,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Small),
            DisplayTypes::TextAlign::Left);
    }

} // namespace DisplayWidgets