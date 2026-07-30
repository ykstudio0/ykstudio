//-------------------------------------------------------------
// File : SerialRenderTarget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.3
// Description : Serial render target
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "IRenderTarget.h"

class SerialRenderTarget : public IRenderTarget
{
public:
    explicit SerialRenderTarget(
        Print& serial);
    
    bool Begin() override;

    bool IsReady() const override;

    void Clear(
        DisplayTheme::Color color) override;

    void DrawText(
        int16_t x,
        int16_t y,
        const char* text,
        DisplayTheme::Color color,
        uint8_t fontSize,
        DisplayTypes::TextAlign align) override;

    // void DrawTextBg(
    //     int16_t x,
    //     int16_t y,
    //     const char* text,
    //     DisplayTheme::Color color,
    //     uint8_t fontSize,
    //     DisplayTypes::TextAlign align) override;

    void DrawLine(
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        DisplayTheme::Color color,
        uint8_t width = 1U) override;

    void DrawRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        DisplayTheme::Color color,
        uint8_t borderWidth = 1U) override;

    void FillRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        DisplayTheme::Color color) override;

    void BeginFrame() override;

    void EndFrame() override;

private:
        static const char* GetAlignName(
            DisplayTypes::TextAlign align);

        Print* m_serial;

        bool m_ready;

        uint32_t m_frameNumber;
};