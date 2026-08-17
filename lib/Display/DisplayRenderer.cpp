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
    bool Renderer::ShouldDraw(
        const bool changed) const
    {
        return
            m_firstRender ||
            m_pageChanged ||
            changed;
    }
    
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
        uint8_t subPage,
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
            (page != m_lastPage) ||
            (subPage != m_lastSubPage);

        m_target->BeginFrame();

        if (m_pageChanged)
        {
            m_target->Clear(
                DisplayTheme::COLOR_BACKGROUND);

            DrawStatic(page, subPage);
        }

        DrawDynamic(
            page,
            subPage,
            model);

        m_target->EndFrame();

        m_lastModel = model;
        m_lastPage = page;
        m_lastSubPage = subPage;
        m_firstRender = false;

        return true;
    }

    void Renderer::DrawContentStatic(
        DisplayPages::Page page,
        uint8_t subPage)
    {
        size_t count = 0;

        auto rows =
            DisplayLayout::GetStaticRows(
                page,
                subPage,
                count);

        if (rows == nullptr)
        {
            return;
        }

        for (size_t i = 0; i < count; i++)
        {
            if (!rows[i].visible)
            {
                continue;
            }

            DisplayWidgets::ValueWidget::DrawStatic(
                *m_target,
                rows[i].row,
                rows[i].label);
        }
    }
    
    void Renderer::DrawStatic(
        DisplayPages::Page page,
        uint8_t subPage)
    {
        Serial.println("DrawStatic()");

        const char* title =
            DisplayPages::GetTitle(page);

        char detailTitle[24];

        //-------------------------------------------------
        // Sub Page Title
        //-------------------------------------------------

        if (subPage > 0U)
        {
            snprintf(
                detailTitle,
                sizeof(detailTitle),
                "%s DTL(%u)",
                DisplayPages::GetDetailTitle(page),
                static_cast<unsigned>(subPage));

            title = detailTitle;
        }

        DisplayWidgets::HeaderWidget::DrawStatic(
            *m_target,
            title);

        DrawContentStatic(
            page,
            subPage);

        DrawFooter(page);
    }

    void Renderer::DrawDynamic(
        DisplayPages::Page page,
        uint8_t subPage,
        const DisplayModel::Model& model)
    {
        DrawHeader(
            page,
            model.GetHeader());

        DrawContent(
            page,
            subPage,
            model);
    }

    // Common
    void Renderer::DrawHeader(
        DisplayPages::Page page,
        const DisplayModel::HeaderData& header)
    {
        const DisplayModel::HeaderData& lastHeader =
            m_lastModel.GetHeader();

        // //---------------------------------------------------------
        // // Static
        // //---------------------------------------------------------

        // if (m_firstRender || m_pageChanged)
        // {
        //     DisplayWidgets::HeaderWidget::DrawStatic(
        //         *m_target,
        //         DisplayPages::GetTitle(page));
        // }

        //---------------------------------------------------------
        // Energy
        //---------------------------------------------------------

        if (page == DisplayPages::Page::Overview)
        {
            const bool energyChanged =
                HasDisplayTextChanged(
                    header.energy,
                    lastHeader.energy);

            if (ShouldDraw(energyChanged))
            {
                DisplayWidgets::HeaderWidget::DrawEnergy(
                    *m_target,
                    header.energy.text,
                    header.energy.color);
            }
        }

        //---------------------------------------------------------
        // Time
        //---------------------------------------------------------

        const bool timeChanged =
            HasTextChanged(
                header.timeText,
                lastHeader.timeText);

        if (ShouldDraw(timeChanged))
        {
            DisplayWidgets::HeaderWidget::DrawTime(
                *m_target,
                header.timeText);
        }

        //---------------------------------------------------------
        // Status
        //---------------------------------------------------------

        const bool statusChanged =
            HasDisplayTextChanged(
                header.status,
                lastHeader.status);

        if (ShouldDraw(statusChanged))
        {
            DisplayWidgets::HeaderWidget::DrawStatus(
                *m_target,
                header.status.text,
                header.status.color);
        }
    }

    void Renderer::DrawFooter(
        DisplayPages::Page page)
    {
        char pageText[16];

        snprintf(
            pageText,
            sizeof(pageText),
            "%u / %u",
            static_cast<unsigned>(
                DisplayPages::GetPageNumber(page)),
            static_cast<unsigned>(
                DisplayPages::PAGE_COUNT));

        DisplayWidgets::FooterWidget::Draw(
            *m_target,
            "< Prev",
            pageText,
            "Next >");
    }

    void Renderer::DrawContent(
        DisplayPages::Page page,
        uint8_t subPage,
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
            {
                if (subPage == 0U)
                {
                    DrawBattery(
                        model.GetBattery());
                }
                else
                {
                    DrawBatteryDetail(
                        model.GetBattery());
                }

                break;
            }

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
                    model.GetSystem(),
                    subPage);
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

        if (ShouldDraw(solarChanged))
        {
            DrawRowValue(
                data.solarPower,
                0U);
        }

        const bool batteryChanged =
            HasValueChanged(
                data.batteryRemaining,
                lastData.batteryRemaining);
            
        if (ShouldDraw(batteryChanged))
        {
            DrawRowValue(
                data.batteryRemaining,
                1U);
        }

        const bool socChanged =
            HasValueChanged(
                data.batteryPercent,
                lastData.batteryPercent);
            
        if (ShouldDraw(socChanged))
        {
            DrawRowValue(
                data.batteryPercent,
                2U);
        }

        const bool loadChanged =
            HasValueChanged(
                data.loadPower,
                lastData.loadPower);
            
        if (ShouldDraw(loadChanged))
        {
            DrawRowValue(
                data.loadPower,
                3U);
        }

        const bool temperatureChanged =
            HasValueChanged(
                data.temperature,
                lastData.temperature);
            
        if (ShouldDraw(temperatureChanged))
        {
            DrawRowValue(
                data.temperature,
                4U);
        }

        const bool humidityChanged =
            HasValueChanged(
                data.humidity,
                lastData.humidity);
            
        if (ShouldDraw(humidityChanged))
        {
            DrawRowValue(
                data.humidity,
                5U);
        }
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

        if (ShouldDraw(voltageChanged))
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (ShouldDraw(currentChanged))
        {
            DrawRowValue(
                data.current,
                1U);
        }

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (ShouldDraw(powerChanged))
        {
            DrawRowValue(
                data.power,
                2U);
        }

        const bool stageChanged =
            HasDisplayTextChanged(
                data.chargingStage,
                lastData.chargingStage);

        if (ShouldDraw(stageChanged))
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

        if (ShouldDraw(inputChanged))
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

        if (ShouldDraw(dailyEnergyChanged))
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

        //---------------------------------------------------------
        // Row 0 : SOC
        //---------------------------------------------------------

        const bool socChanged =
            HasValueChanged(
                data.percent,
                lastData.percent);

        if (ShouldDraw(socChanged))
        {
            DrawRowValue(
                data.percent,
                0U);
        }


        //---------------------------------------------------------
        // Row 1 : Remaining Capacity
        //---------------------------------------------------------

        const bool remainingChanged =
            HasValueChanged(
                data.remainingCapacity,
                lastData.remainingCapacity);

        if (ShouldDraw(remainingChanged))
        {
            DrawRowValue(
                data.remainingCapacity,
                1U);
        }


        //---------------------------------------------------------
        // Row 2 : Power
        //---------------------------------------------------------

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (ShouldDraw(powerChanged))
        {
            DrawRowValue(
                data.power,
                2U);
        }


        //---------------------------------------------------------
        // Row 3 : Current
        //---------------------------------------------------------

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (ShouldDraw(currentChanged))
        {
            DrawRowValue(
                data.current,
                3U);
        }


        //---------------------------------------------------------
        // Row 4 : Temperature
        //---------------------------------------------------------

        const bool tempChanged =
            HasValueChanged(
                data.temperature,
                lastData.temperature);

        if (ShouldDraw(tempChanged))
        {
            DrawRowValue(
                data.temperature,
                4U);
        }


        //---------------------------------------------------------
        // Row 5 : BMS Communication Status
        //---------------------------------------------------------

        const bool statusChanged =
            HasDisplayTextChanged(
                data.communicationStatus,
                lastData.communicationStatus);

        if (ShouldDraw(statusChanged))
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                5U,
                data.communicationStatus);
        }
    }

    void Renderer::DrawBatteryDetail(
        const DisplayModel::BatteryData& data)
    {
        const DisplayModel::BatteryData& lastData =
            m_lastModel.GetBattery();

        // Voltage
        const bool voltageChanged =
            HasValueChanged(
                data.voltage,
                lastData.voltage);

        if (ShouldDraw(voltageChanged))
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        // Cell 1
        const bool cell1Changed =
            HasValueChanged(
                data.cellVoltage1,
                lastData.cellVoltage1);

        if (ShouldDraw(cell1Changed))
        {
            DrawRowValue(
                data.cellVoltage1,
                1U);
        }

        // Cell 2
        const bool cell2Changed =
            HasValueChanged(
                data.cellVoltage2,
                lastData.cellVoltage2);

        if (ShouldDraw(cell2Changed))
        {
            DrawRowValue(
                data.cellVoltage2,
                2U);
        }

        // Cell 3
        const bool cell3Changed =
            HasValueChanged(
                data.cellVoltage3,
                lastData.cellVoltage3);

        if (ShouldDraw(cell3Changed))
        {
            DrawRowValue(
                data.cellVoltage3,
                3U);
        }

        // Cell 4
        const bool cell4Changed =
            HasValueChanged(
                data.cellVoltage4,
                lastData.cellVoltage4);

        if (ShouldDraw(cell4Changed))
        {
            DrawRowValue(
                data.cellVoltage4,
                4U);
        }

        // Cell Delta
        const bool deltaChanged =
            HasValueChanged(
                data.cellDelta,
                lastData.cellDelta);

        if (ShouldDraw(deltaChanged))
        {
            DrawRowValue(
                data.cellDelta,
                5U);
        }
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

        if (ShouldDraw(voltageChanged))
        {
            DrawRowValue(
                data.voltage,
                0U);
        }

        const bool currentChanged =
            HasValueChanged(
                data.current,
                lastData.current);

        if (ShouldDraw(currentChanged))
        {
            DrawRowValue(
                data.current,
                1U);
        }

        const bool powerChanged =
            HasValueChanged(
                data.power,
                lastData.power);

        if (ShouldDraw(powerChanged))
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

        const bool cabinChanged =
            HasValueChanged(
                data.cabinTemperature,
                lastData.cabinTemperature);

        if (ShouldDraw(cabinChanged))
        {
            DrawRowValue(
                data.cabinTemperature,
                0U);
        }
        
        const bool batteryChanged =
            HasValueChanged(
                data.batteryTemperature,
                lastData.batteryTemperature);

        if (ShouldDraw(batteryChanged))
        {
            DrawRowValue(
                data.batteryTemperature,
                1U);
        }

        const bool bmsChanged =
            HasValueChanged(
                data.bmsTemperature,
                lastData.bmsTemperature);

        if (ShouldDraw(bmsChanged))
        {
            DrawRowValue(
                data.bmsTemperature,
                2U);
        }

        const bool controllerChanged =
        HasValueChanged(
            data.controllerTemperature,
            lastData.controllerTemperature);    

        if (ShouldDraw(controllerChanged))
        {
            DrawRowValue(
                data.controllerTemperature,
                3U);
        }

        

        
    }

    void Renderer::DrawSystem(
        const DisplayModel::SystemData& data,
        uint8_t subPage)
    {
        const DisplayModel::SystemData& lastData =
            m_lastModel.GetSystem();

        //-------------------------------------------------
        // SYSTEM DTL(1)
        //-------------------------------------------------

        if (subPage == 1U)
        {
            const bool solarChanged =
                HasValueChanged(
                    data.solarOfflineCount,
                    lastData.solarOfflineCount);

            if (ShouldDraw(solarChanged))
            {
                DrawRowValue(
                    data.solarOfflineCount,
                    0U);
            }

            const bool chargeChanged =
                HasValueChanged(
                    data.chargeOfflineCount,
                    lastData.chargeOfflineCount);

            if (ShouldDraw(chargeChanged))
            {
                DrawRowValue(
                    data.chargeOfflineCount,
                    1U);
            }

            const bool bmsChanged =
                HasValueChanged(
                    data.bmsOfflineCount,
                    lastData.bmsOfflineCount);

            if (ShouldDraw(bmsChanged))
            {
                DrawRowValue(
                    data.bmsOfflineCount,
                    2U);
            }

            const bool loadChanged =
                HasValueChanged(
                    data.loadOfflineCount,
                    lastData.loadOfflineCount);

            if (ShouldDraw(loadChanged))
            {
                DrawRowValue(
                    data.loadOfflineCount,
                    3U);
            }

            const bool controllerChanged =
                HasValueChanged(
                    data.controllerOfflineCount,
                    lastData.controllerOfflineCount);

            if (ShouldDraw(controllerChanged))
            {
                DrawRowValue(
                    data.controllerOfflineCount,
                    4U);
            }

            const bool socChanged =
                HasValueChanged(
                    data.socOfflineCount,
                    lastData.socOfflineCount);

            if (ShouldDraw(socChanged))
            {
                DrawRowValue(
                    data.socOfflineCount,
                    5U);
            }

            return;
        }

        // RSSI
        const bool wifiSignalChanged =
            HasValueChanged(
                data.wifiSignal,
                lastData.wifiSignal);

        if (ShouldDraw(wifiSignalChanged))
        {
            DrawRowValue(
                data.wifiSignal,
                0U);
        }

        // EPEVER
        const bool epeverChanged =
            HasDisplayTextChanged(
                data.epeverStatus,
                lastData.epeverStatus);

        if (ShouldDraw(epeverChanged))
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                1U,
                data.epeverStatus);
        }

        // BMS
        const bool bmsChanged =
            HasDisplayTextChanged(
                data.bmsStatus,
                lastData.bmsStatus);

        if (ShouldDraw(bmsChanged))
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                2U,
                data.bmsStatus);
        }

        // DEVICES
        const bool deviceCountChanged =
            HasDisplayTextChanged(
                data.deviceCount,
                lastData.deviceCount);

        if (ShouldDraw(deviceCountChanged))
        {
            DisplayWidgets::ValueWidget::DrawTextValue(
                *m_target,
                3U,
                data.deviceCount);
        }

        // HEAP
        const bool heapChanged =
            HasValueChanged(
                data.heapPercent,
                lastData.heapPercent);

        if (ShouldDraw(heapChanged))
        {
            DrawRowValue(
                data.heapPercent,
                4U);
        }

        // UPTIME
        const bool uptimeChanged =
            HasValueChanged(
                data.uptime,
                lastData.uptime);

        if (ShouldDraw(uptimeChanged))
        {
            DrawRowValue(
                data.uptime,
                5U);
        }
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