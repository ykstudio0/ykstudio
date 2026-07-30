//-------------------------------------------------------------
// File : SerialRenderTarget.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.3
// Description : Serial render target implementation
//-------------------------------------------------------------

#include "SerialRenderTarget.h"

SerialRenderTarget::SerialRenderTarget(
    Print& serial)
    : m_serial(&serial),
      m_ready(false),
      m_frameNumber(0U)
{

}

bool SerialRenderTarget::Begin()
{
    if (m_serial == nullptr)
    {
        m_ready = false;
        return false;
    }

    // Serial.begin()은 main 또는 System 계층에서
    // 이미 수행했다고 가정한다.
    m_ready = true;

    return true;
}

bool SerialRenderTarget::IsReady() const
{
    return 
        m_ready &&
        m_serial != nullptr;
}

void SerialRenderTarget::Clear(
    DisplayTheme::Color color)
{
    if (!IsReady())
    {
        return;
    }

    m_serial->print(
        "[CLEAR] color=0x");

    m_serial->println(
        static_cast<uint16_t>(color),
        HEX);
}

void SerialRenderTarget::DrawText(
    int16_t x,
    int16_t y,
    const char* text,
    DisplayTheme::Color color,
    uint8_t fontSize,
    DisplayTypes::TextAlign align)
{
    if (!IsReady())
    {
        return;
    }

    m_serial->print("[TEXT] ");

    m_serial->print("x=");
    m_serial->print(x);

    m_serial->print(" y=");
    m_serial->print(y);

    m_serial->print(" align=");
    m_serial->print(
        GetAlignName(align));

    m_serial->print(" font=");
    m_serial->print(fontSize);

    m_serial->print(" color=0x");
    m_serial->print(
        static_cast<uint16_t>(color),
        HEX);

    m_serial->print(" text=\"");
    m_serial->print(
        text != nullptr
            ? text
            : "");

    m_serial->println("\"");
}

// void SerialRenderTarget::DrawTextBg(
//     int16_t x,
//     int16_t y,
//     const char* text,
//     DisplayTheme::Color textColor,
//     DisplayTheme::Color backgroundColor,
//     uint8_t fontSize,
//     DisplayTypes::TextAlign align)
// {
//     (void)backgroundColor;

//     DrawText(
//         x,
//         y,
//         text,
//         textColor,
//         fontSize,
//         align);
// }

void SerialRenderTarget::DrawLine(
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

    m_serial->print("[LINE] ");
    
    m_serial->print("x1=");
    m_serial->print(x1);

    m_serial->print(" y1=");
    m_serial->print(y1);

    m_serial->print(" x2=");
    m_serial->print(x2);

    m_serial->print(" y2=");
    m_serial->print(y2);

    m_serial->print(" width=");
    m_serial->print(width);

    m_serial->print(" color=0x");
    m_serial->println(
        static_cast<uint16_t>(color),
        HEX);
}

void SerialRenderTarget::DrawRect(
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

    m_serial->print("[RECT] ");
    
    m_serial->print("x=");
    m_serial->print(x);

    m_serial->print(" y=");
    m_serial->print(y);

    m_serial->print(" w=");
    m_serial->print(width);

    m_serial->print(" h=");
    m_serial->print(height);

    m_serial->print(" border=");
    m_serial->print(borderWidth);

    m_serial->print(" color=0x");
    m_serial->println(
        static_cast<uint16_t>(color),
        HEX);
}

void SerialRenderTarget::FillRect(
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

    m_serial->print("[FILL_RECT] ");
    
    m_serial->print("x=");
    m_serial->print(x);

    m_serial->print(" y=");
    m_serial->print(y);

    m_serial->print(" w=");
    m_serial->print(width);

    m_serial->print(" h=");
    m_serial->print(height);

    m_serial->print(" color=0x");
    m_serial->println(
        static_cast<uint16_t>(color),
        HEX);
}

void SerialRenderTarget::BeginFrame()
{
    if (!IsReady())
    {
        return;
    }

    ++m_frameNumber;

    m_serial->println();

    m_serial->print(
        "========== DISPLAY FRAME #");

    m_serial->print(m_frameNumber);

    m_serial->println(
        " BEGIN ==========");
}

void SerialRenderTarget::EndFrame()
{
    if (!IsReady())
    {
        return;
    }

    m_serial->print(
        "========== DISPLAY FRAME #");

    m_serial->print(m_frameNumber);

    m_serial->println(
        " END ============");

    m_serial->println();
}

const char* SerialRenderTarget::GetAlignName(
    DisplayTypes::TextAlign align)
{
    switch (align)
    {
        case DisplayTypes::TextAlign::Left:
            return "Left";

        case DisplayTypes::TextAlign::Center:
            return "Center";

        case DisplayTypes::TextAlign::Right:
            return "Right";

        default:
            return "Unknown";
        
    }
}