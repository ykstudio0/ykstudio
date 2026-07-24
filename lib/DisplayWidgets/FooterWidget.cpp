//-------------------------------------------------------------
// File : FooterWidget.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Common display footer widget implementation
//-------------------------------------------------------------

#include "FooterWidget.h"
#include "DisplayLayout.h"
#include "DisplayTheme.h"

namespace DisplayWidgets
{
    void FooterWidget::Draw(
        IRenderTarget& target,
        const char* leftText,
        const char* centerText,
        const char* rightText)
    {
        // Footer Background
        target.FillRect(
            DisplayLayout::FOOTER_X,
            DisplayLayout::FOOTER_Y,
            DisplayLayout::FOOTER_WIDTH,
            DisplayLayout::FOOTER_HEIGHT,
            DisplayTheme::COLOR_FOOTER_BACKGROUND);

        // Divider
        target.DrawLine(
            DisplayLayout::FOOTER_X,
            DisplayLayout::FOOTER_DIVIDER_Y,
            DisplayLayout::FOOTER_RIGHT,
            DisplayLayout::FOOTER_DIVIDER_Y,
            DisplayTheme::COLOR_DIVIDER,
            DisplayTheme::DIVIDER_WIDTH);

        // Left
        if (leftText && leftText[0])
        {
            target.DrawText(
                DisplayLayout::FOOTER_PREVIOUS_X,
                DisplayLayout::FOOTER_PREVIOUS_Y,
                leftText,
                DisplayTheme::COLOR_TEXT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Left);
        }

        // Center
        if (centerText && centerText[0])
        {
            target.DrawText(
                DisplayLayout::FOOTER_PAGE_X,
                DisplayLayout::FOOTER_PAGE_Y,
                centerText,
                DisplayTheme::COLOR_TEXT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Center);
        }

        // Right
        if (rightText && rightText[0])
        {
            target.DrawText(
                DisplayLayout::FOOTER_NEXT_X,
                DisplayLayout::FOOTER_NEXT_Y,
                rightText,
                DisplayTheme::COLOR_TEXT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Right);
        }
    }
}