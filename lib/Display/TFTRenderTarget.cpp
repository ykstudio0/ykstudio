//-------------------------------------------------------------
// File : TFTRenderTarget.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.3.4
// Description : LovyanGFX TFT render target implementation
//-------------------------------------------------------------

#include "TFTRenderTarget.h"
#include "DisplayLayout.h"

TFTRenderTarget::TFTRenderTarget(
    lgfx::LGFX_Device& display)
    : m_display(&display),
      m_ready(false),
      m_frameNumber(0U)
{

}

bool TFTRenderTarget::Begin()
{
    if (m_display == nullptr)
    {
        m_ready = false;
        return false;
    }

    m_display->init();

    m_display->setRotation(3);

    m_ready =
        m_display->width() == DisplayLayout::SCREEN_WIDTH &&
        m_display->height() == DisplayLayout::SCREEN_HEIGHT;

    return true;
}

bool TFTRenderTarget::IsReady() const
{
    return
        m_ready &&
        m_display != nullptr;
}

void TFTRenderTarget::Clear(DisplayTheme::Color color)
{
    Serial.println("========== CLEAR ==========");
    
    if (!IsReady())
    {
        return;
    }

    Serial.printf("[TFT] Clear called: 0x%04X\n", static_cast<unsigned int>(color));

    m_display->fillScreen(
        ToNativeColor(color));
}

void TFTRenderTarget::DrawText(
    int16_t x,
    int16_t y,
    const char* text,
    DisplayTheme::Color color,
    uint8_t fontSize,
    DisplayTypes::TextAlign align)
{
    if (!IsReady() || text == nullptr)
    {
        return;
    }

    m_display->setTextDatum(
        ToTextDatum(align));

    m_display->setTextColor(
        ToNativeColor(color));

    m_display->setTextSize(
        ToNativeFontSize(fontSize));

    m_display->drawString(
        text,
        x,
        y);
}

void TFTRenderTarget::DrawTextBg(
    int16_t x,
    int16_t y,
    const char* text,
    DisplayTheme::Color textColor,
    DisplayTheme::Color backgroundColor,
    uint8_t fontSize,
    DisplayTypes::TextAlign align)
{
    if (!IsReady() || text == nullptr)
    {
        return;
    }

    m_display->setTextDatum(
        ToTextDatum(align));

    m_display->setTextColor(
        ToNativeColor(textColor),
        ToNativeColor(backgroundColor));

    m_display->setTextSize(
        ToNativeFontSize(fontSize));

    m_display->drawString(
        text,
        x,
        y);
}

void TFTRenderTarget::DrawLine(
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    DisplayTheme::Color color,
    uint8_t width)
{
    if (!IsReady())
    {
        return;
    }

    if (width <= 1U)
    {
        m_display->drawLine(
            x1,
            y1,
            x2,
            y2,
            ToNativeColor(color));

        return;
    }

    for (uint8_t i = 0; i < width; ++i)
    {
        m_display->drawLine(
            x1,
            y1 + i,
            x2,
            y2 + i,
            ToNativeColor(color));
    }
}

void TFTRenderTarget::DrawRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    DisplayTheme::Color color,
    uint8_t borderWidth)
{
    if (!IsReady())
    {
        return;
    }

    for (uint8_t i = 0; i < borderWidth; ++i)
    {
        m_display->drawRect(
            x + i,
            y + i,
            width - (i * 2),
            height - (i * 2),
            ToNativeColor(color));
    }
}

void TFTRenderTarget::FillRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    DisplayTheme::Color color)
{
    if (!IsReady())
    {
        return;
    }

    m_display->fillRect(
        x,
        y,
        width,
        height,
        ToNativeColor(color));
}

void TFTRenderTarget::BeginFrame()
{
    if (!IsReady())
    {
        return;
    }

    ++m_frameNumber;
}

void TFTRenderTarget::EndFrame()
{
    if (!IsReady())
    {
        return;
    }
}

uint16_t TFTRenderTarget::ToNativeColor(
    DisplayTheme::Color color)
{
    return static_cast<uint16_t>(color);
}

textdatum_t TFTRenderTarget::ToTextDatum(
    DisplayTypes::TextAlign align)
{
    switch (align)
    {
        case DisplayTypes::TextAlign::Left:
            return textdatum_t::top_left;

        case DisplayTypes::TextAlign::Center:
            return textdatum_t::top_center;

        case DisplayTypes::TextAlign::Right:
            return textdatum_t::top_right;

        default:
            return textdatum_t::top_left;
    }
}

uint8_t TFTRenderTarget::ToNativeFontSize(
    uint8_t fontSize)
{
    if (fontSize == 0U)
    {
        return 1U;
    }

    return fontSize;
}