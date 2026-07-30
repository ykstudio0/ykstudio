//-------------------------------------------------------------
// File : IRenderTarget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.3
// Description : Abstract render target interface
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "DisplayTypes.h"
#include "DisplayTheme.h"

// IRenderTarget
// DisplayRenderer가 실제 출력 장치에 의존하지 않도록 하는 
// 추상 랜더링 인터페이스다.
//
// 구현 대상 예:
// - SerialRenderTarget
// - MockRenderTarget
// - TftRenderTarget
class IRenderTarget
{
public:
    virtual ~IRenderTarget() = default;

    /*
     *  출력 장치를 초기화한다.
     *
     *  초기화 성공 여부를 반환한다.
    */
    virtual bool Begin() = 0;

    /*
     *  출력 장치가 사용 가능한 상태인지 반환한다.
    */
    virtual bool IsReady() const = 0;
    /*
    * 화면 전체를 지정된 색으로 지운다.
    */
    virtual void Clear(
        DisplayTheme::Color color) = 0;

    /*
     * 문자열을 지정된 위치에 출력한다.
     *
     *  x, y:
     *    텍스트 기준 좌표
     * 
     *  text:
     *    출력 문자열
     * 
     *  color:
     *    RGB565 텍스트 색상
     * 
     *  fontSize:
     *    Theme에서 결정된 추상 글꼴 크기
     * 
     *  align:
     *    Left / Center / Right 정렬
     */
    virtual void DrawText(
        int16_t x,
        int16_t y,
        const char* text,
        DisplayTheme::Color color,
        uint8_t fontSize,
        DisplayTypes::TextAlign align) = 0;
    
    /*
     * 배경색을 함께 사용하여 문자열을 출력한다.
     *
     * TFT에서 이전 문자열의 잔상을 방지하기 위해
     * 문자 배경을 지정하여 출력한다.
     */
    virtual void DrawTextBg(
        int16_t x,
        int16_t y,
        const char* text,
        DisplayTheme::Color textColor,
        DisplayTheme::Color backgroundColor,
        uint8_t fontSize,
        DisplayTypes::TextAlign align)
    {
        (void)backgroundColor;

        DrawText(
            x,
            y,
            text,
            textColor,
            fontSize,
            align);
    }

    /*
     * 선을 그린다.
    */
    virtual void DrawLine(
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        DisplayTheme::Color color,
        uint8_t width = 1U) = 0;

    /*
     * 사각형의 테두리를 그린다.
     */
    virtual void DrawRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        DisplayTheme::Color color,
        uint8_t borderWidth = 1U) = 0;

    /*
     * 내부가 채워진 사각형을 그린다.
     */
    virtual void FillRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        DisplayTheme::Color color) = 0;

    /*
     * 한 프레임의 랜더링을 시작한다.
     *
     * Serial이나 단순 TFT에서는 아무 작업도 하지 않을 수 있다.
     * Sprite 또는 더블 버퍼 방식에서는 프레임 준비에 사용할 수 있다.
     */
    virtual void BeginFrame()
    {

    }

    /*
     * 완성된 프레임을 화면에 반영한다.
     *
     * Sprite 또는 더블 버퍼 방식에서는 실제 LCD 전송에 사용한다.
     */
    virtual void EndFrame()
    {

    }
};