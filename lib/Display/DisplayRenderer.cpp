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
#include <cstring>

#include "DisplayRenderer.h"
#include "HeaderWidget.h"
#include "ValueWidget.h"
#include "FooterWidget.h"


namespace DisplayRenderer
{
    const char* GetEnergyStatusText(
        DisplayModel::EnergyStatus status)
    {
        switch (status)
        {
            case DisplayModel::EnergyStatus::Charging:
                return "Charging";

            case DisplayModel::EnergyStatus::Idle:
                return "Idle";

            case DisplayModel::EnergyStatus::Night:
                return "Night";

            case DisplayModel::EnergyStatus::Warning:
                return "Warning";
            
            default:
                return "Idle";
        }
    }

    DisplayTheme::Color GetEnergyStatusColor(
        DisplayModel::EnergyStatus status)
    {
        switch (status)
        {
            case DisplayModel::EnergyStatus::Charging:
                return DisplayTheme::COLOR_ACTIVE;

            case DisplayModel::EnergyStatus::Idle:
                return DisplayTheme::COLOR_INFO;

            case DisplayModel::EnergyStatus::Night:
                return DisplayTheme::COLOR_DISABLED;

            case DisplayModel::EnergyStatus::Warning:
                return DisplayTheme::COLOR_ALARM;

            default:
                return DisplayTheme::COLOR_INFO;
        }
    }

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

        m_pageChanged =
            m_firstRender ||
            (page != m_lastPage);

        m_target->BeginFrame();

        if (m_pageChanged)
        {
            m_target->Clear(
                DisplayTheme::COLOR_BACKGROUND);

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

    void Renderer::DrawContentStatic(
        DisplayPages::Page page)
    {
        size_t count = 0;

        auto rows =
            DisplayLayout::GetStaticRows(
                page,
                count);

        if(rows == nullptr)
            return;

        for(size_t i=0;i<count;i++)
        {
            if(!rows[i].visible)
                continue;

            DisplayWidgets::ValueWidget::DrawStatic(
                *m_target,
                rows[i].row,
                rows[i].label);
        }
    }
    
    void Renderer::DrawStatic(
        DisplayPages::Page page)
    {
        Serial.println("DrawStatic()");

        DisplayWidgets::HeaderWidget::DrawStatic(
            *m_target,
            DisplayPages::GetTitle(page));

        DrawContentStatic(page);

        DrawFooter(page);
    }

    void Renderer::DrawDynamic(
        DisplayPages::Page page,
        const DisplayModel::Model& model)
    {
        DrawHeader(
            page,
            model.GetOverview(),
            model.GetSystem());

        DrawContent(
            page,
            model);
    }

    void Renderer::BuildTimeText(
        const DisplayModel::SystemData& system,
        char* buffer,
        size_t bufferSize)
    {
        FormatValue(
            system.currentTime,
            buffer,
            bufferSize);
    }

    // Common
    void Renderer::DrawHeader(
        DisplayPages::Page page,
        const DisplayModel::OverviewData& overview,
        const DisplayModel::SystemData& system)
    {
        const char* energyText;
        DisplayTheme::Color energyColor;

        BuildEnergyDisplay(
            overview.energyStatus,
            energyText,
            energyColor);

        char currentTime[16];
        char previousTime[16];

        BuildTimeText(
            system,
            currentTime,
            sizeof(currentTime));

        BuildTimeText(
            m_lastModel.GetSystem(),
            previousTime,
            sizeof(previousTime));

        char currentStatus[8];
        char previousStatus[8];
        uint16_t statusColor;

        BuildStatus(
            system,
            currentStatus,
            sizeof(currentStatus),
            statusColor);

        BuildStatus(
            m_lastModel.GetSystem(),
            previousStatus,
            sizeof(previousStatus),
            statusColor);

        //---------------------------------------------------------
        // Static
        //---------------------------------------------------------

        if (m_firstRender || m_pageChanged)
        {
            DisplayWidgets::HeaderWidget::DrawStatic(
                *m_target,
                DisplayPages::GetTitle(page));
        }

        //---------------------------------------------------------
        // Energy
        //---------------------------------------------------------

        if (page == DisplayPages::Page::Overview)
        {
            if (m_firstRender ||
                m_pageChanged ||
                overview.energyStatus !=
                m_lastModel.GetOverview().energyStatus)
            {
                DisplayWidgets::HeaderWidget::DrawEnergy(
                    *m_target,
                    energyText,
                    energyColor);
            }
        }

        //---------------------------------------------------------
        // Time
        //---------------------------------------------------------

        if (m_firstRender ||
            m_pageChanged ||
            HasTextChanged(
                currentTime,
                previousTime))
        {
            DisplayWidgets::HeaderWidget::DrawTime(
                *m_target,
                currentTime);
        }

        //---------------------------------------------------------
        // Status
        //---------------------------------------------------------

        if (m_firstRender ||
            m_pageChanged ||
            HasTextChanged(
                currentStatus,
                previousStatus))
        {
            DisplayWidgets::HeaderWidget::DrawStatus(
                *m_target,
                currentStatus,
                statusColor);
        }
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
            HasValueChanged(
                data.solarPower,
                lastData.solarPower);

        if (solarChanged)
        {
            DrawRowValue(
                data.solarPower,
                0U);
        }

        const bool batteryChanged =
            HasValueChanged(
                data.batteryVoltage,
                lastData.batteryVoltage);
            
        if (batteryChanged)
        {
            DrawRowValue(
                data.batteryVoltage,
                1U);
        }

        const bool socChanged =
            HasValueChanged(
                data.batteryPercent,
                lastData.batteryPercent);
            
        if (socChanged)
        {
            DrawRowValue(
                data.batteryPercent,
                2U);
        }

        const bool loadChanged =
            HasValueChanged(
                data.loadPower,
                lastData.loadPower);
            
        if (loadChanged)
        {
            DrawRowValue(
                data.loadPower,
                3U);
        }

        const bool temperatureChanged =
            HasValueChanged(
                data.temperature,
                lastData.temperature);
            
        if (temperatureChanged)
        {
            DrawRowValue(
                data.temperature,
                4U);
        }

        const bool humidityChanged =
            HasValueChanged(
                data.humidity,
                lastData.humidity);
            
        if (humidityChanged)
        {
            DrawRowValue(
                data.humidity,
                5U);
        }
    }

    void Renderer::BuildEnergyDisplay(
        DisplayModel::EnergyStatus status,
        const char*& text,
        DisplayTheme::Color& color)
    {
        switch (status)
        {
        case DisplayModel::EnergyStatus::Charging:

            text = "Charging";
            color = DisplayTheme::COLOR_SUCCESS;
            break;

        case DisplayModel::EnergyStatus::Idle:

            text = "Idle";
            color = DisplayTheme::COLOR_INFO;
            break;

        case DisplayModel::EnergyStatus::Night:

            text = "Night";
            color = DisplayTheme::COLOR_LABEL;
            break;

        case DisplayModel::EnergyStatus::Warning:

            text = "Warning";
            color = DisplayTheme::COLOR_ALARM;
            break;

        default:

            text = "---";
            color = DisplayTheme::COLOR_INFO;
            break;
        }
    }

    void Renderer::BuildStatus(
        const DisplayModel::SystemData& system,
        char* buffer,
        size_t bufferSize,
        uint16_t& color)
    {
        if (bufferSize == 0)
            return;

        if (!system.wifiConnected)
        {
            strncpy(buffer, "NET", bufferSize);
            color = DisplayTheme::COLOR_WARNING;
        }
        else if (!system.rs485Ready)
        {
            strncpy(buffer, "485", bufferSize);
            color = DisplayTheme::COLOR_WARNING;
        }
        else if (!system.modbusReady)
        {
            strncpy(buffer, "MOD", bufferSize);
            color = DisplayTheme::COLOR_WARNING;
        }
        else if (!system.deviceManagerReady)
        {
            strncpy(buffer, "DEV", bufferSize);
            color = DisplayTheme::COLOR_WARNING;
        }
        else
        {
            strncpy(buffer, "OK", bufferSize);
            color = DisplayTheme::COLOR_ACTIVE;
        }

        buffer[bufferSize - 1] = '\0';
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
        const DisplayModel::SolarData& lastData =
            m_lastModel.GetSolar();

        const bool voltageChanged =
            HasValueChanged(
                data.voltage,
                lastData.voltage);

        if (voltageChanged)
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (currentChanged)
        {
            DrawRowValue(
                data.current,
                1U);
        }

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (powerChanged)
        {
            DrawRowValue(
                data.power,
                2U);
        }

        const bool stageChanged =
            HasDisplayTextChanged(
                data.chargingStage,
                lastData.chargingStage);

        if (stageChanged)
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                3U,
                data.chargingStage);
        }

        const bool inputChanged =
            HasDisplayTextChanged(
                data.inputVoltage,
                lastData.inputVoltage);

        if (inputChanged)
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                4U,
                data.inputVoltage);
        }

        const bool dailyEnergyChanged =
            HasValueChanged(
                data.dailyEnergy,
                lastData.dailyEnergy);

        if (dailyEnergyChanged)
        {
            DrawRowValue(
                data.dailyEnergy,
                5U);
        }
    }

    void Renderer::DrawBattery(
        const DisplayModel::BatteryData& data)
    {
        const DisplayModel::BatteryData& lastData =
            m_lastModel.GetBattery();

        const bool voltageChanged =
            HasValueChanged(
                data.voltage,
                lastData.voltage);

        if (voltageChanged)
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (currentChanged)
        {
            DrawRowValue(
                data.current,
                1U);
        }

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (powerChanged)
        {
            DrawRowValue(
                data.power,
                2U);
        }    

        const bool socChanged =
            HasValueChanged(
                data.percent,
                lastData.percent);

        if (socChanged)
        {
            DrawRowValue(
                data.percent,
                3U);
        }    

        const bool tempChanged =
            HasValueChanged(
                data.temperature,
                lastData.temperature);

        if (tempChanged)
        {
            DrawRowValue(
                data.temperature,
                4U);
        }    

        // const bool statusChanged =
        //     HasDisplayTextChanged(
        //         data.status,
        //         lastData.status);
        // if (statusChanged)
        // {
        //     HasTextChanged(
        //         data.status,
        //         5U);
        // }
    }

    void Renderer::DrawLoad(
        const DisplayModel::LoadData& data)
    {
        const DisplayModel::LoadData& lastData =
            m_lastModel.GetLoad();

        const bool voltageChanged =
            HasValueChanged(
                data.voltage,
                lastData.voltage);

        if (voltageChanged)
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (currentChanged)
        {
            DrawRowValue(
                data.current,
                1U);
        }

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (powerChanged)
        {
            DrawRowValue(
                data.power,
                2U);
        }    
    }

    void Renderer::DrawTemperature(
        const DisplayModel::TemperatureData& data)
    {
        const DisplayModel::TemperatureData& lastData =
            m_lastModel.GetTemperature();

        const bool controllerChanged =
            HasValueChanged(
                data.controllerTemperature,
                lastData.controllerTemperature);    

        if (controllerChanged)
        {
            DrawRowValue(
                data.controllerTemperature,
                0U);
        }

        const bool batteryChanged =
            HasValueChanged(
                data.batteryTemperature,
                lastData.batteryTemperature);

        if (batteryChanged)
        {
            DrawRowValue(
                data.batteryTemperature,
                1U);
        }

        const bool cabinChanged =
            HasValueChanged(
                data.cabinTemperature,
                lastData.cabinTemperature);

        if (cabinChanged)
        {
            DrawRowValue(
                data.cabinTemperature,
                2U);
        }

        const bool humidityChanged =
            HasValueChanged(
                data.cabinHumidity,
                lastData.cabinHumidity);

        if (humidityChanged)
        {
            DrawRowValue(
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
    void Renderer::DrawRowValue(
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

        DisplayWidgets::ValueWidget::DrawValue(
            *m_target,
            row,
            valueText,
            unit,
            valueColor);
    }

    bool Renderer::HasValueChanged(
        const DisplayTypes::DisplayValue& current,
        const DisplayTypes::DisplayValue& previous) const
    {
        return
            m_firstRender ||
            m_pageChanged ||
            (current.value != previous.value) ||
            (current.state != previous.state) ||
            (current.visible != previous.visible);
    }

    bool Renderer::HasTextChanged(
        const char* current,
        const char* previous) const
    {
        return 
            m_firstRender ||
            m_pageChanged ||
            (strcmp(current, previous) != 0);
    }

    bool Renderer::HasDisplayTextChanged(
        const DisplayTypes::DisplayText& current,
        const DisplayTypes::DisplayText& previous) const
    {
        if (m_firstRender || m_pageChanged)
        {
            return true;
        }

        const char* currentText =
            current.text != nullptr
                ? current.text
                : "";

        const char* previousText =
            previous.text != nullptr
                ? previous.text
                : "";

        return 
            strcmp(currentText, previousText) != 0 ||
            current.color != previous.color;
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