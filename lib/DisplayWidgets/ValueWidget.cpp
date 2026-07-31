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
    void ValueWidget::DrawValue(
        IRenderTarget& target,
        uint8_t row,
        const char* valueText,
        const char* unit,
        DisplayTheme::Color valueColor)
    {
        const int16_t rowY =
            DisplayLayout::CONTENT_FIRST_ROW_Y +
            static_cast<int16_t>(row) *
            DisplayLayout::CONTENT_ROW_HEIGHT;

        target.FillRect(
            DisplayLayout::CONTENT_DYNAMIC_X,
            rowY,
            DisplayLayout::CONTENT_DYNAMIC_WIDTH,
            DisplayLayout::CONTENT_ROW_HEIGHT,
            DisplayTheme::COLOR_BACKGROUND);

        if (valueText != nullptr &&
            valueText[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_VALUE_X,
                rowY,
                valueText,
                valueColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Value),
                DisplayTypes::TextAlign::Right);
        }

        if (unit != nullptr &&
            unit[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_UNIT_X,
                rowY,
                unit,
                valueColor,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Normal),
                DisplayTypes::TextAlign::Left);
        }
    }

    void ValueWidget::DrawStatic(
        IRenderTarget& target,
        uint8_t row,
        const char* label)
    {
        const int16_t rowY =
            DisplayLayout::CONTENT_FIRST_ROW_Y +
            static_cast<int16_t>(row) *
            DisplayLayout::CONTENT_ROW_HEIGHT;

        if (label != nullptr &&
            label[0] != '\0')
        {
            target.DrawText(
                DisplayLayout::CONTENT_LABEL_X,
                rowY,
                label,
                DisplayTheme::COLOR_LABEL,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Normal),
                DisplayTypes::TextAlign::Left);
        }
    }

    void DisplayWidgets::ValueWidget::DrawTextValue(
        IRenderTarget& target,
        uint8_t row,
        const DisplayTypes::DisplayText& value)
    {
        const int16_t rowY = 
            DisplayLayout::CONTENT_FIRST_ROW_Y +
            static_cast<int16_t>(row) *
            DisplayLayout::CONTENT_ROW_HEIGHT;

        target.FillRect(
            DisplayLayout::CONTENT_DYNAMIC_X,
            rowY,
            DisplayLayout::CONTENT_DYNAMIC_WIDTH,
            DisplayLayout::CONTENT_ROW_HEIGHT,
            DisplayTheme::COLOR_BACKGROUND);

        const char* text =
            value.text != nullptr
                ? value.text
                : "";

        if (text[0] == '\0')
        {
            return;
        }

        target.DrawText(
            DisplayLayout::CONTENT_VALUE_X,
            rowY,
            text,
            value.color,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Value),
            DisplayTypes::TextAlign::Right);
        
    }
}