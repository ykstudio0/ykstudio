//-------------------------------------------------------------
// File : ValueWidget.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Label, value and unit row widget implementation
//-------------------------------------------------------------

#include "ValueWidget.h"
#include "DisplayLayout.h"
#include "DisplayTheme.h"

namespace DisplayWidgets
{
    void ValueWidget::Draw(
        IRenderTarget& target,
        uint8_t row,
        const char* label,
        const char* valueText,
        const char* unit,
        DisplayTheme::Color valueColor)
    {
        const int16_t y =
            DisplayLayout::GetRowY(row);

        // Label
        if (label != nullptr &&
            label[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_LABEL_X,
                y,
                label,
                DisplayTheme::COLOR_LABEL,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Normal),
                DisplayTypes::TextAlign::Left);
        }

        // value
        if (valueText != nullptr &&
            valueText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_VALUE_X,
                y,
                valueText,
                valueColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Value),
                DisplayTypes::TextAlign::Right);
        }

        // Unit
        if (valueText != nullptr &&
            valueText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_UNIT_X,
                y,
                unit,
                valueColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Normal),
                DisplayTypes::TextAlign::Left);
        }
    }
}