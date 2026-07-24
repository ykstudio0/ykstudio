//-------------------------------------------------------------
// File : DisplayRenderer.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.3
// Description : Display renderer implementation
//-------------------------------------------------------------

#include <math.h>
#include <stdio.h>

#include "DisplayRenderer.h"

namespace DisplayRenderer
{
    // Constructor
    Renderer::Renderer()
        : m_target(nullptr),
          m_ready(false)
    {

    }

    // Public
    bool Renderer::Begin(
        IRenderTarget& target)
    {
        m_target = &target;

        m_ready =
            m_target->Begin() &&
            m_target->IsReady();
        
        return m_ready;
    }

    bool Renderer::IsReady() const
    {
        return
            m_ready &&
            m_target != nullptr &&
            m_target->IsReady();
    }
    
    bool Renderer::RenderPage(
        DisplayPages::Page page,
        const DisplayModel::Model& model)
    {
        if (!IsReady())
        {
            return false;
        }

        if (!DisplayPages::IsValid(page))
        {
            return false;
        }

        m_target->BeginFrame();

        m_target->Clear(
            DisplayTheme::COLOR_BACKGROUND);

        DrawHeader(
            page,
            model.GetSystem());

        DrawContent(
            page,
            model);

        DrawFooter(page);

        m_target->EndFrame();

        return true;
    }

    // Common
    void Renderer::DrawHeader(
        DisplayPages::Page page,
        const DisplayModel::SystemData& system)
    {
        if (!IsReady())
        {
            return;
        }

        // const char* title =
        //     DisplayPages::GetTitle(page);

        // Header background
        m_target->FillRect(
            DisplayLayout::HEADER_X,
            DisplayLayout::HEADER_Y,
            DisplayLayout::HEADER_WIDTH,
            DisplayLayout::HEADER_HEIGHT,
            DisplayTheme::COLOR_HEADER_BACKGROUND);

        // Page title
        m_target->DrawText(
            DisplayLayout::HEADER_TITLE_X,
            DisplayLayout::HEADER_TITLE_Y,
            DisplayPages::GetTitle(page),
            DisplayTheme::COLOR_TITLE,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Title),
            DisplayTypes::TextAlign::Left);

        // Current time
        char timeText[12];

        FormatValue(
            system.currentTime,
            timeText,
            sizeof(timeText));

        if (timeText[0] != '\0')
        {
            m_target->DrawText(
                DisplayLayout::HEADER_TIME_X,
                DisplayLayout::HEADER_TIME_Y,
                timeText,
                DisplayTheme::COLOR_TEXT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Small),
                DisplayTypes::TextAlign::Right);
        }

        // System status
        DrawHeaderStatus(system);
        
        // Header divider
        m_target->DrawLine(
            DisplayLayout::HEADER_X,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayLayout::SCREEN_RIGHT,
            DisplayLayout::HEADER_DIVIDER_Y,
            DisplayTheme::COLOR_DIVIDER,
            DisplayTheme::DIVIDER_WIDTH);
    }

    void Renderer::DrawFooter(
        DisplayPages::Page page)
    {
        // 다음 단계에서 구현한다.
        (void)page;
    }

    void Renderer::DrawContent(
        DisplayPages::Page page,
        const DisplayModel::Model& model)
    {
        switch (page)
        {
            case DisplayPages::Page::Overview:
                DrawOverview(
                    model.GetOverview());
                break;

            case DisplayPages::Page::Solar:
                DrawSolar(
                    model.GetSolar());
                break;

            case DisplayPages::Page::Battery:
                DrawBattery(
                    model.GetBattery());
                break;

            case DisplayPages::Page::Load:
                DrawLoad(
                    model.GetLoad());
                break;

            case DisplayPages::Page::Temperature:
                DrawTemperature(
                    model.GetTemperature());
                break;

            case DisplayPages::Page::System:
                DrawSystem(
                    model.GetSystem());
                break;
        }
    }

    // Individual Pages
    void Renderer::DrawOverview(
        const DisplayModel::OverviewData& data)
    {
        DrawLabelValue(
            "Solar",
            data.solarPower,
            0U);

        DrawLabelValue(
            "Battery",
            data.batteryVoltage,
            1U);

        DrawLabelValue(
            "SOC",
            data.batteryPercent,
            2U);

        DrawLabelValue(
            "Load",
            data.loadPower,
            3U);

        DrawLabelValue(
            "Temp",
            data.temperature,
            4U);

        DrawLabelValue(
            "Humidity",
            data.humidity,
            5U);
    }

    void Renderer::DrawHeaderStatus(
        const DisplayModel::SystemData& system)
    {
        if (!IsReady())
        {
            return;
        }

        const char* statusText = "OK";

        DisplayTheme::Color statusColor =
            DisplayTheme::COLOR_ACTIVE;

        if (!system.wifiConnected)
        {
            statusText = "NET";
            statusColor = DisplayTheme::COLOR_WARNING;
        }
        else if (!system.rs485Ready)
        {
            statusText = "485";
            statusColor = DisplayTheme::COLOR_ALARM;
        }
        else if (!system.modbusReady)
        {
            statusText = "MOD";
            statusColor = DisplayTheme::COLOR_ALARM;
        }
        else if (!system.deviceManagerReady)
        {
            statusText = "DEV";
            statusColor = DisplayTheme::COLOR_WARNING;
        }

        m_target->DrawText(
            DisplayLayout::HEADER_STATUS_X,
            DisplayLayout::HEADER_STATUS_Y,
            statusText,
            statusColor,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Small),
            DisplayTypes::TextAlign::Left);
    }

    void Renderer::DrawSolar(
        const DisplayModel::SolarData& data)
    {
        // 다음 단계에서 구현한다.
        (void)data;
    }

    void Renderer::DrawBattery(
        const DisplayModel::BatteryData& data)
    {
        // 다음 단계에서 구현한다.
        (void)data;
    }

    void Renderer::DrawLoad(
        const DisplayModel::LoadData& data)
    {
        // 다음 단계에서 구현한다.
        (void)data;
    }

    void Renderer::DrawTemperature(
        const DisplayModel::TemperatureData& data)
    {
        // 다음 단계에서 구현한다.
        (void)data;
    }

    void Renderer::DrawSystem(
        const DisplayModel::SystemData& data)
    {
        // 다음 단계에서 구현한다.
        (void)data;
    }

    // Drawing Helpers
    void Renderer::DrawLabelValue(
        const char* label,
        const DisplayTypes::DisplayValue& value,
        uint8_t row)
    {
        if (!IsReady())
        {
            return;
        }

        // Row 위치 계산
        const int16_t y =
            DisplayLayout::GetRowY(row);

        // Label
        m_target->DrawText(
            DisplayLayout::CONTENT_LABEL_X,
            y,
            label,
            DisplayTheme::COLOR_LABEL,
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Normal),
            DisplayTypes::TextAlign::Left);
        
        // Value
        DrawValue(
            value,
            DisplayLayout::CONTENT_VALUE_X,
            y);

        // Unit
        const char* unit =
            DisplayTypes::GetUnit(
                value.type);

        if (unit != nullptr &&
            unit[0] != '\0')
        {
            m_target->DrawText(
                DisplayLayout::CONTENT_UNIT_X,
                y,
                unit,
                DisplayTheme::COLOR_UNIT,
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Normal),
                DisplayTypes::TextAlign::Left);
        }
    }

    void Renderer::DrawValue(
        const DisplayTypes::DisplayValue& value,
        int16_t x,
        int16_t y)
    {
        if (!IsReady())
        {
            return;
        }

        if (!value.visible)
        {
            return;
        }

        char text[VALUE_BUFFER_SIZE];

        FormatValue(
            value,
            text,
            sizeof(text));

        // 빈 문자열이면 출력하지 않는다.
        if (text[0] == '\0')
        {
            return;
        }

        const DisplayTheme::Color color =
            DisplayTheme::GetValueColor(
                value.state);

        const uint8_t fontSize =
                DisplayTheme::GetFontSize(
                    DisplayTheme::FontRole::Value);

        m_target->DrawText(
            x,
            y,
            text,
            color,
            fontSize,
            DisplayTypes::TextAlign::Left);
    }

    void Renderer::FormatValue(
        const DisplayTypes::DisplayValue& value,
        char* buffer,
        size_t bufferSize) const
    {
        // 유효하지 않은 출력 버퍼
        if (buffer == nullptr ||
            bufferSize == 0U)
        {
            return;
        }

        // 항상 빈 문자열로 먼저 초기화한다.
        buffer[0] = '\0';

        // 표시하지 않는 값
        if (!value.visible)
        {
            return;
        }

        // NaN 또눈 무한대 값 방어
        if (isnan(value.value) ||
            isinf(value.value))
        {
            snprintf(
                buffer,
                bufferSize,
                "--");

            return;
        }
        
        // 지나치게 큰 소수 자릿수 방지
        uint8_t decimals =
            value.decimals;
        
        if (decimals > 6U)
        {
            decimals = 6U;
        }

        switch (value.type)
        {
            // 시간: 자정 이후 초 단위 값으로 해석
            case DisplayTypes::ValueType::Time:
            {
                uint32_t totalSeconds =
                    value.value > 0.0f
                        ? static_cast<uint32_t>(value.value)
                        : 0U;
                
                // 24시간 범위로 순환
                totalSeconds %= 86400UL;

                const uint32_t hours =
                    totalSeconds / 3600UL;
                
                const uint32_t minutes =
                    (totalSeconds % 3600UL) / 60UL;

                const uint32_t seconds =
                    totalSeconds % 60UL;

                snprintf(
                    buffer,
                    bufferSize,
                    "%02u:%02u:%02u",
                    static_cast<unsigned long>(hours),
                    static_cast<unsigned long>(minutes),
                    static_cast<unsigned long>(seconds));

                break;
            }

            // 지속시간: 누적 초 단위 값으로 해석
            case DisplayTypes::ValueType::Duration:
            {
                uint32_t totalSeconds =
                    value.value > 0.0f
                        ? static_cast<uint32_t>(value.value)
                        : 0U;

                const uint32_t hours =
                    totalSeconds / 3600UL;
                
                const uint32_t minutes =
                    (totalSeconds % 3600UL) / 60UL;

                const uint32_t seconds =
                    totalSeconds % 60UL;

                snprintf(
                    buffer,
                    bufferSize,
                    "%02u:%02u:%02u",
                    static_cast<unsigned long>(hours),
                    static_cast<unsigned long>(minutes),
                    static_cast<unsigned long>(seconds));

                break;
            }

            // Text는 현재 displayValue가 문자열을 저장하지 않으므로
            // 임시 대체 문자열을 추력한다.
            case DisplayTypes::ValueType::Text:
            {
                snprintf(
                    buffer,
                    bufferSize,
                    "--");

                break;
            }

            // 일반 숫자
            case DisplayTypes::ValueType::None:
            case DisplayTypes::ValueType::Number:
            case DisplayTypes::ValueType::Voltage:
            case DisplayTypes::ValueType::Current:
            case DisplayTypes::ValueType::Power:
            case DisplayTypes::ValueType::Energy:
            case DisplayTypes::ValueType::Temperature:
            case DisplayTypes::ValueType::Humidity:
            case DisplayTypes::ValueType::Percent:
            default:
            {
                snprintf(
                    buffer,
                    bufferSize,
                    "%.*f",
                    static_cast<int>(decimals),
                    static_cast<double>(value.value));

                break;
            }
        }
    }
}