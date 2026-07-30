//-------------------------------------------------------------
// File : TFTRenderTarget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.3.4
// Description : LovyanGFX TFT render target
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>

#include "IRenderTarget.h"

// TFTRenderTarget
//
// DisplayRenderer의 추상 그리기 명령을
// LovyanGFX 기반 TFT 출력으로 변환한다.
// 
// DisplayRenderer는 TFT와 LovyanGFX의 존재를 알지 못한다.
class TFTRenderTarget : public IRenderTarget
{
public:
    explicit TFTRenderTarget(
        lgfx::LGFX_Device& display);

    bool Begin() override;

    bool IsReady() const override;

    void Clear(
        DisplayTheme::Color color) override;
        
    void DrawText(
        int16_t x,
        int16_t y,
        const char* text,
        DisplayTheme::Color color,
        uint8_t fonsSize,
        DisplayTypes::TextAlign align) override;

    void DrawTextBg(
        int16_t x,
        int16_t y,
        const char* text,
        DisplayTheme::Color textColor,
        DisplayTheme::Color backGroundColor,
        uint8_t fonsSize,
        DisplayTypes::TextAlign align) override;

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
    static uint16_t ToNativeColor(
        DisplayTheme::Color color);

    static uint8_t ToNativeFontSize(
        uint8_t fontSize);

    static textdatum_t ToTextDatum(
        DisplayTypes::TextAlign align);

    lgfx::LGFX_Device* m_display;

    bool m_ready;

    uint32_t m_frameNumber;
};