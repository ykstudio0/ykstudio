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
#include "HeaderWidget.h"
#include "ValueWidget.h"
#include "FooterWidget.h"

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

        bool pageChanged =
            m_firstRender ||
            (page != m_lastPage);

        (void)pageChanged;

        m_target->BeginFrame();

        // m_target->Clear(
        //     DisplayTheme::COLOR_BACKGROUND);

        // DrawHeader(
        //     page,
        //     model.GetSystem());

        // DrawContent(
        //     page,
        //     model);

        // DrawFooter(page);
        if (pageChanged)
        {
            DrawStatic(page);
        }

        DrawDynamic(
            page,
            model);

        m_target->EndFrame();
        
        m_lastModel = model;
        m_lastPage = page;
        m_firstRender = false;

        return true;
    }

    void Renderer::DrawStatic(
        DisplayPages::Page page)
    {
        Serial.println("DrawStatic()");

        DisplayWidgets::HeaderWidget::DrawStatic(
            *m_target,
            DisplayPages::GetTitle(page));

        DrawFooter(page);
    }

    void Renderer::DrawDynamic(
        DisplayPages::Page page,
        const DisplayModel::Model& model)
    {
        // Serial.println("DrawDynamic");

        // Header 동적 영역 지우기
        // m_target->FillRect(
        //     DisplayLayout::HEADER_X,
        //     DisplayLayout::HEADER_Y,
        //     DisplayLayout::HEADER_WIDTH,
        //     DisplayLayout::HEADER_HEIGHT,
        //     DisplayTheme::COLOR_HEADER_BACKGROUND);

        // // Content 영역 지우기
        // m_target->FillRect(
        //     DisplayLayout::CONTENT_X,
        //     DisplayLayout::CONTENT_Y,
        //     DisplayLayout::CONTENT_WIDTH,
        //     DisplayLayout::CONTENT_HEIGHT,
        //     DisplayTheme::COLOR_BACKGROUND);

        DrawHeader(
            page,
            model.GetSystem());

        DrawContent(
            page,
            model);
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

        // Current time
        const auto& previous =
            m_lastModel.GetSystem();

        const bool forceRedraw =
            m_firstRender ||
            (page != m_lastPage);

        const bool timeChanged =
            forceRedraw ||
            system.currentTime.value !=
                previous.currentTime.value ||
            system.currentTime.state !=
                previous.currentTime.state ||
            system.currentTime.visible !=
                previous.currentTime.visible ||
            system.currentTime.type !=
                previous.currentTime.type ||
            system.currentTime.decimals !=
                previous.currentTime.decimals;

        char timeText[12] = {};

        if (timeChanged)
        {
            FormatValue(
                system.currentTime,
                timeText,
                sizeof(timeText));
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

        DisplayWidgets::HeaderWidget::Draw(
            *m_target,
            nullptr,
            timeChanged ? timeText : nullptr,
            statusText,
            statusColor);
    }

    void Renderer::DrawFooter(
        DisplayPages::Page page)
    {
        Serial.println("DrawFooter");
        const char* previousText = "< Prev";
        const char* pageText     = "1 / 4";
        const char* nextText     = "Next >";
        
        DisplayWidgets::FooterWidget::Draw(
            *m_target,
            previousText,
            pageText,
            nextText);
            
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
        const DisplayModel::OverviewData& lastData =
        m_lastModel.GetOverview();

        const bool solarChanged =
            m_firstRender ||
            (data.solarPower.value !=
                lastData.solarPower.value) ||
            (data.solarPower.state !=
                lastData.solarPower.state) ||
            (data.solarPower.visible !=
                lastData.solarPower.visible);

        if (solarChanged)
        {
            DrawLabelValue(
                "Solar",
                data.solarPower,
                0U);
        }

        const bool batteryChanged =
            m_firstRender ||
            (data.batteryVoltage.value !=
                lastData.batteryVoltage.value) ||
            (data.batteryVoltage.state !=
                lastData.batteryVoltage.state) ||
            (data.batteryVoltage.visible !=
                lastData.batteryVoltage.visible);

        if (batteryChanged)
        {
            DrawLabelValue(
                "Battery",
                data.batteryVoltage,
                1U);
        }

        const bool socChanged =
            m_firstRender ||
            (data.batteryPercent.value !=
                lastData.batteryPercent.value) ||
            (data.batteryPercent.state !=
                lastData.batteryPercent.state) ||
            (data.batteryPercent.visible !=
                lastData.batteryPercent.visible);

        if (socChanged)
        {
            DrawLabelValue(
                "SOC",
                data.batteryPercent,
                2U);
        }

        const bool loadChanged =
            m_firstRender ||
            (data.loadPower.value !=
                lastData.loadPower.value) ||
            (data.loadPower.state !=
                lastData.loadPower.state) ||
            (data.loadPower.visible !=
                lastData.loadPower.visible);

        if (loadChanged)
        {
            DrawLabelValue(
                "Load",
                data.loadPower,
                3U);
        }

        const bool temperatureChanged =
            m_firstRender ||
            (data.temperature.value !=
                lastData.temperature.value) ||
            (data.temperature.state !=
                lastData.temperature.state) ||
            (data.temperature.visible !=
                lastData.temperature.visible);

        if (temperatureChanged)
        {
            DrawLabelValue(
                "Temp",
                data.temperature,
                4U);
        }

        const bool humidityChanged =
            m_firstRender ||
            (data.humidity.value !=
                lastData.humidity.value) ||
            (data.humidity.state !=
                lastData.humidity.state) ||
            (data.humidity.visible !=
                lastData.humidity.visible);

        if (humidityChanged)
        {
            DrawLabelValue(
                "Humidity",
                data.humidity,
                5U);
        }
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
        const DisplayModel::TemperatureData& lastData =
            m_lastModel.GetTemperature();

        const bool controllerChanged =
            m_firstRender ||
            (data.controllerTemperature.value !=
                lastData.controllerTemperature.value) ||
            (data.controllerTemperature.state !=
                lastData.controllerTemperature.state) ||
            (data.controllerTemperature.visible !=
                lastData.controllerTemperature.visible);

        if (controllerChanged)
        {
            DrawLabelValue(
                "Controller",
                data.controllerTemperature,
                0U);
        }

        const bool batteryChanged =
            m_firstRender ||
            (data.batteryTemperature.value !=
                lastData.batteryTemperature.value) ||
            (data.batteryTemperature.state !=
                lastData.batteryTemperature.state) ||
            (data.batteryTemperature.visible !=
                lastData.batteryTemperature.visible);

        if (batteryChanged)
        {
            DrawLabelValue(
                "Battery",
                data.batteryTemperature,
                1U);
        }

        const bool cabinChanged =
            m_firstRender ||
            (data.cabinTemperature.value !=
                lastData.cabinTemperature.value) ||
            (data.cabinTemperature.state !=
                lastData.cabinTemperature.state) ||
            (data.cabinTemperature.visible !=
                lastData.cabinTemperature.visible);

        if (cabinChanged)
        {
            DrawLabelValue(
                "Temp",
                data.cabinTemperature,
                2U);
        }

        const bool humidityChanged =
            m_firstRender ||
            (data.cabinHumidity.value !=
                lastData.cabinHumidity.value) ||
            (data.cabinHumidity.state !=
                lastData.cabinHumidity.state) ||
            (data.cabinHumidity.visible !=
                lastData.cabinHumidity.visible);

        if (humidityChanged)
        {
            DrawLabelValue(
                "Humidity",
                data.cabinHumidity,
                3U);
        }
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

        if (!value.visible)
        {
            return;
        }

        char valueText[VALUE_BUFFER_SIZE];

        FormatValue(
            value,
            valueText,
            sizeof(valueText));

        if (valueText[0] == '\0')
        {
            return;
        }

        const bool showUnit =
            value.state != DisplayTypes::WidgetState::NoData &&
            value.state != DisplayTypes::WidgetState::Offline;

        const DisplayTheme::Color valueColor =
            DisplayTheme::GetValueColor(
                value.state);

        const char* unit =
            showUnit
                ? DisplayTypes::GetUnit(value.type)
                : "";

        DisplayWidgets::ValueWidget::Draw(
            *m_target,
            row,
            label,
            valueText,
            unit,
            valueColor);
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
            DisplayTypes::TextAlign::Right);
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

        // 아직 데이터를 한 번도 수신하지 않은 상태
        if (value.state == DisplayTypes::WidgetState::NoData)
        {
            snprintf(
                buffer,
                bufferSize,
                "--");

            return;
        }

        // 장비 또는 데이터 소스가 오프라인인 상태
        if (value.state == DisplayTypes::WidgetState::Offline)
        {
            snprintf(
                buffer,
                bufferSize,
                "OFF");

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