//-------------------------------------------------------------
// File : DisplayLayout.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Layout
//-------------------------------------------------------------

#pragma once

#include "DisplayTypes.h"

namespace DisplayLayout
{
    // LCD Size
    constexpr int16_t SCREEN_WIDTH  = 320;
    constexpr int16_t SCREEN_HEIGHT = 240;

    // 화면 외곽 여백
    constexpr int16_t MARGIN_LEFT   = 8;
    constexpr int16_t MARGIN_RIGHT  = 8;
    constexpr int16_t MARGIN_TOP    = 6;
    constexpr int16_t MARGIN_BOTTOM = 6;

    // 공통 영역 높이
    constexpr int16_t HEADER_HEIGHT = 32;
    constexpr int16_t FOOTER_HEIGHT = 28;

    // 구분선 두께
    constexpr int16_t DIVIDER_HEIGHT = 1;

    // Header 영역
    constexpr int16_t HEADER_X = 
        0;
    constexpr int16_t HEADER_Y = 
        0;
    constexpr int16_t HEADER_WIDTH = 
        SCREEN_WIDTH;

    // Header 아래 구분선
    constexpr int16_t HEADER_DIVIDER_Y = 
        HEADER_Y + HEADER_HEIGHT;

    // Content 영역
    constexpr int16_t CONTENT_X = 
        MARGIN_LEFT;
    
    constexpr int16_t CONTENT_Y = 
        HEADER_DIVIDER_Y +
        DIVIDER_HEIGHT +
        MARGIN_TOP;

    constexpr int16_t CONTENT_WIDTH =
        SCREEN_WIDTH -
        MARGIN_LEFT -
        MARGIN_RIGHT;

    constexpr int16_t CONTENT_HEIGHT =
        SCREEN_HEIGHT -
        HEADER_HEIGHT -
        FOOTER_HEIGHT -
        DIVIDER_HEIGHT -
        DIVIDER_HEIGHT -
        MARGIN_TOP -
        MARGIN_BOTTOM;

    // Footer 위 구분선
    constexpr int16_t FOOTER_DIVIDER_Y =
        SCREEN_HEIGHT -
        FOOTER_HEIGHT -
        DIVIDER_HEIGHT;

    // Footer 영역
    constexpr int16_t FOOTER_X =
        0;

    constexpr int16_t FOOTER_Y =
        SCREEN_HEIGHT -
        FOOTER_HEIGHT;

    constexpr int16_t FOOTER_WIDTH =
        SCREEN_WIDTH;

    // Header 내부 좌표
    constexpr int16_t HEADER_TITLE_X =
        MARGIN_LEFT;

    constexpr int16_t HEADER_TITLE_Y =
        8;

    constexpr int16_t HEADER_TIME_X =
        224;

    constexpr int16_t HEADER_TIME_Y =
        8;

    constexpr int16_t HEADER_STATUS_X =
        292;

    constexpr int16_t HEADER_STATUS_Y =
        8;

    // Footer 내부 좌표
    constexpr int16_t FOOTER_PREVIOUS_X =
        MARGIN_LEFT;

    constexpr int16_t FOOTER_PREVIOUS_Y =
        FOOTER_Y + 7;

    constexpr int16_t FOOTER_PAGE_X =
        132;

    constexpr int16_t FOOTER_PAGE_Y =
        FOOTER_Y + 7;

    constexpr int16_t FOOTER_NEXT_X =
        272;

    constexpr int16_t FOOTER_NEXT_Y =
        FOOTER_Y + 7;

    // Content 내부 공통 간격
    constexpr int16_t CONTENT_ROW_HEIGHT =
        28;

    constexpr int16_t CONTENT_LABEL_X =
        CONTENT_X;

    constexpr int16_t CONTENT_VALUE_X =
        150;

    constexpr int16_t CONTENT_UNIT_X =
        260;

    constexpr int16_t CONTENT_FIRST_ROW_Y =
        CONTENT_Y + 4;

    // 행 번호를 실제 Y 좌표로 변환
    // row는 0부터 시작한다.
    constexpr int16_t GetRowY(uint8_t row)
    {
        return CONTENT_FIRST_ROW_Y +
        static_cast<int16_t>(row) *
        CONTENT_ROW_HEIGHT;
    }

    // 좌표 설정 검증
    constexpr bool IsValid()
    {
        return
            SCREEN_WIDTH > 0 &&
            SCREEN_HEIGHT > 0 &&
            HEADER_HEIGHT > 0 &&
            FOOTER_HEIGHT > 0 &&
            CONTENT_WIDTH > 0 &&
            CONTENT_HEIGHT > 0 &&
            FOOTER_Y > HEADER_DIVIDER_Y;
    }
}