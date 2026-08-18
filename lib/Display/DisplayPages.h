//-------------------------------------------------------------
// File : DisplayPages.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Page
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace DisplayPages
{
    enum class Page : uint8_t
    {
        Overview = 0,
        Solar,
        Battery,
        Load,
        Temperature,
        System,

        Count
    };
    // 첫 번째 기본 페이지
    constexpr Page DEFAULT_PAGE =
        Page::Overview;

    // 전체 페이지 개수
    // Count는 실제 화면이 아니라 페이지 개수를 계산하기 위한 값이다.
    constexpr uint8_t PAGE_COUNT =
        static_cast<uint8_t>(Page::Count);

    // 페이지 ID가 정상 범위인지 확인
    inline bool IsValid(Page page)
    {
        return static_cast<uint8_t>(page) < PAGE_COUNT;
    }

    // 화면에 표시할 1부터 시작하는 페이지 번호 반환
    inline uint8_t GetPageNumber(Page page)
    {
        if (!IsValid(page))
        {
            return 0U;
        }

        return
            static_cast<uint8_t>(page) + 1U;
    }

    // 페이지 제목 반환
    inline const char* GetTitle(Page page)
    {
        switch (page)
        {
            case Page::Overview:
                return "OVERVIEW";

            case Page::Solar:
                return "SOLAR";

            case Page::Battery:
                return "BATTERY";

            case Page::Load:
                return "LOAD";

            case Page::Temperature:
                return "TEMPERATURE";

            case Page::System:
                return "SYSTEM";

            default:
                return "UNKNOWN";
        }
    }

    // 다음 페이지 반환
    // 마지막 페이지 다음에는 첫 페이지로 돌아간다.
    inline Page Next(Page current)
    {
        uint8_t next =
            static_cast<uint8_t>(current) + 1U;

        if (next >= PAGE_COUNT)
        {
            next = 0;
        }

        return static_cast<Page>(next);
    }

    // 이전 페이지 반환
    // 첫 페이지 이전에는 마지막 페이지로 이동한다.
    inline Page Previous(Page current)
    {
        uint8_t value =
            static_cast<uint8_t>(current);

        if (value == 0)
        {
            value = PAGE_COUNT - 1U;
        }
        else
        {
            value--;
        }

        return static_cast<Page>(value);
    }

    inline uint8_t GetSubPageCount(Page page)
    {
        switch (page)
        {
            case Page::Battery:
                return 3U;

            case Page::System:
                return 2U;

            default:
                return 1U;
        }
    }

    inline const char* GetDetailTitle(Page page)
    {
        switch (page)
        {
            case Page::Overview:
                return "OVR";

            case Page::Solar:
                return "SOLAR";

            case Page::Battery:
                return "BATTERY";

            case Page::Load:
                return "LOAD";

            case Page::Temperature:
                return "TEMP";

            case Page::System:
                return "SYSTEM";

            default:
                return "PAGE";
        }
    }
}