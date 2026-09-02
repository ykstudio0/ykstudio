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
#include "Display.h"
#include "Logger.h"
#include "DeviceManager.h"


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

        if (m_wifiSetupMode)
        {
            if (m_firstRender)
            {
                m_target->Clear(
                    DisplayTheme::COLOR_BACKGROUND);

                DrawWiFiSetupMode();

                m_firstRender =
                    false;
            }

            m_target->EndFrame();

            return true;
        }

        //---------------------------------------------------------
        // WiFi Setup Confirm Overlay
        //---------------------------------------------------------

        if (m_wifiSetupConfirm)
        {
            if (!m_wifiSetupConfirmDrawn)
            {
                DrawWiFiSetupConfirm();

                m_wifiSetupConfirmDrawn =
                    true;
            }

            m_target->EndFrame();

            return true;
        }

        if (m_deviceConfigMode)
        {
            if (
                m_firstRender ||
                !m_deviceConfigDrawn)
            {
                m_target->Clear(
                    DisplayTheme::COLOR_BACKGROUND);

                DrawDeviceConfigStatic();

                m_deviceConfigDrawn = true;
                m_deviceConfigDirty =
                    DeviceConfigDirty::None;

                m_firstRender = false;
            }
            else
            {
                switch (m_deviceConfigDirty)
                {
                    case DeviceConfigDirty::Mppt:
                        DrawDeviceConfigCheck(
                            90,
                            m_deviceConfigEdit.mppt);
                        break;

                    case DeviceConfigDirty::Bms:
                        DrawDeviceConfigCheck(
                            120,
                            m_deviceConfigEdit.bms);
                        break;

                    case DeviceConfigDirty::Sht40:
                        DrawDeviceConfigCheck(
                            150,
                            m_deviceConfigEdit.sht40);
                        break;

                    case DeviceConfigDirty::Rtc:
                        DrawDeviceConfigCheck(
                            180,
                            m_deviceConfigEdit.rtc);
                        break;

                    default:
                        break;
                }

                m_deviceConfigDirty =
                    DeviceConfigDirty::None;
            }

            m_target->EndFrame();

            return true;
        }

        //---------------------------------------------------------
        // Normal Page Render
        //---------------------------------------------------------

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

    void Renderer::ToggleDeviceMppt()
    {
        m_deviceConfigEdit.mppt =
            !m_deviceConfigEdit.mppt;

        m_deviceConfigDirty =
            DeviceConfigDirty::Mppt;
    }

    void Renderer::ToggleDeviceBms()
    {
        m_deviceConfigEdit.bms =
            !m_deviceConfigEdit.bms;

        m_deviceConfigDirty =
            DeviceConfigDirty::Bms;
    }

    void Renderer::ToggleDeviceSht40()
    {
        m_deviceConfigEdit.sht40 =
            !m_deviceConfigEdit.sht40;

        m_deviceConfigDirty =
            DeviceConfigDirty::Sht40;
    }

    void Renderer::ToggleDeviceRtc()
    {
        m_deviceConfigEdit.rtc =
            !m_deviceConfigEdit.rtc;

        m_deviceConfigDirty =
            DeviceConfigDirty::Rtc;
    }

    const SVEMS::Device::DeviceConfiguration&
    Renderer::GetDeviceConfigEdit() const
    {
        return m_deviceConfigEdit;
    }

    void Renderer::DrawDeviceConfigStatic()
    {
        const auto& config =
            m_deviceConfigEdit;

        constexpr int16_t CHECK_X = 32;
        constexpr int16_t TEXT_X  = 82;

        constexpr int16_t TITLE_Y  = 20;
        constexpr int16_t WIFI_Y   = 52;

        constexpr int16_t MPPT_Y   = 90;
        constexpr int16_t BMS_Y    = 120;
        constexpr int16_t SHT40_Y  = 150;
        constexpr int16_t RTC_Y    = 180;

        constexpr int16_t BUTTON_Y = 212;

        m_target->DrawText(
            160,
            TITLE_Y,
            "< DEVICE CONFIG >",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            160,
            WIFI_Y,
            "WIFI SETUP",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            CHECK_X,
            MPPT_Y,
            config.mppt ? "[*]" : "[ ]",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            TEXT_X,
            MPPT_Y,
            "MPPT",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            CHECK_X,
            BMS_Y,
            config.bms ? "[*]" : "[ ]",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            TEXT_X,
            BMS_Y,
            "BMS",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            CHECK_X,
            SHT40_Y,
            config.sht40 ? "[*]" : "[ ]",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            TEXT_X,
            SHT40_Y,
            "SHT40",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            CHECK_X,
            RTC_Y,
            config.rtc ? "[*]" : "[ ]",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            TEXT_X,
            RTC_Y,
            "RTC",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Left
        );

        m_target->DrawText(
            90,
            BUTTON_Y,
            "SAVE",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            230,
            BUTTON_Y,
            "CANCEL",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Center
        );
    }

    void Renderer::DrawDeviceConfigCheck(
        int16_t y,
        bool checked)
    {
        constexpr int16_t CHECK_X = 32;

        //-------------------------------------------------
        // 기존 체크박스 영역 지우기
        //-------------------------------------------------

        m_target->FillRect(
            CHECK_X,
            y,
            42,
            20,
            DisplayTheme::COLOR_BACKGROUND
        );

        //-------------------------------------------------
        // 체크박스 다시 그리기
        //-------------------------------------------------

        m_target->DrawText(
            CHECK_X,
            y,
            checked ? "[*]" : "[ ]",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Left
        );
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

        if (page == DisplayPages::Page::System)
        {
            if (ShouldDraw(false))
            {
                DisplayWidgets::HeaderWidget::DrawStatus(
                    *m_target,
                    "SET",
                    DisplayTheme::COLOR_VALUE);
            }
        }
        else
        {
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
                else if (subPage == 1U)
                {
                    DrawBatteryDetail(
                        model.GetBattery());
                }
                else if (subPage == 2U)
                {
                    DrawBatteryDetail2(
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

    void Renderer::DrawBatteryDetail2(
        const DisplayModel::BatteryData& data)
    {
        const DisplayModel::BatteryData& lastData =
            m_lastModel.GetBattery();

        // BMS Temperature
        const bool bmsTemperatureChanged =
            HasValueChanged(
                data.bmsTemperature,
                lastData.bmsTemperature);

        if (ShouldDraw(bmsTemperatureChanged))
        {
            DrawRowValue(
                data.bmsTemperature,
                0U);
        }

        // External Temperature
        const bool externalTemperatureChanged =
            HasValueChanged(
                data.externalTemperature,
                lastData.externalTemperature);

        if (ShouldDraw(externalTemperatureChanged))
        {
            DrawRowValue(
                data.externalTemperature,
                1U);
        }

        // Total Capacity
        const bool totalCapacityChanged =
            HasValueChanged(
                data.totalCapacity,
                lastData.totalCapacity);

        if (ShouldDraw(totalCapacityChanged))
        {
            DrawRowValue(
                data.totalCapacity,
                2U);
        }

        // Remaining Capacity
        const bool remainingCapacityChanged =
            HasValueChanged(
                data.remainingCapacity,
                lastData.remainingCapacity);

        if (ShouldDraw(remainingCapacityChanged))
        {
            DrawRowValue(
                data.remainingCapacity,
                3U);
        }

        // SOC
        const bool percentChanged =
            HasValueChanged(
                data.percent,
                lastData.percent);

        if (ShouldDraw(percentChanged))
        {
            DrawRowValue(
                data.percent,
                4U);
        }

        // BMS Communication Status
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

                if (totalSeconds < 86400UL)
                {
                    const uint32_t hours =
                        totalSeconds / 3600UL;

                    const uint32_t minutes =
                        (totalSeconds % 3600UL) / 60UL;

                    const uint32_t seconds =
                        totalSeconds % 60UL;

                    snprintf(
                        buffer,
                        bufferSize,
                        "%02lu:%02lu:%02lu",
                        static_cast<unsigned long>(hours),
                        static_cast<unsigned long>(minutes),
                        static_cast<unsigned long>(seconds));
                }
                else
                {
                    const uint32_t days =
                        totalSeconds / 86400UL;

                    const uint32_t hours =
                        (totalSeconds % 86400UL) / 3600UL;

                    const uint32_t minutes =
                        (totalSeconds % 3600UL) / 60UL;

                    snprintf(
                        buffer,
                        bufferSize,
                        "%lud %02lu:%02lu",
                        static_cast<unsigned long>(days),
                        static_cast<unsigned long>(hours),
                        static_cast<unsigned long>(minutes));
                }

                break;
            }
            // case DisplayTypes::ValueType::Duration:
            // {
            //     uint32_t totalSeconds =
            //         value.value > 0.0f
            //             ? static_cast<uint32_t>(value.value)
            //             : 0U;

            //     const uint32_t hours =
            //         totalSeconds / 3600UL;
                
            //     const uint32_t minutes =
            //         (totalSeconds % 3600UL) / 60UL;

            //     const uint32_t seconds =
            //         totalSeconds % 60UL;

            //     snprintf(
            //         buffer,
            //         bufferSize,
            //         "%02u:%02u:%02u",
            //         static_cast<unsigned long>(hours),
            //         static_cast<unsigned long>(minutes),
            //         static_cast<unsigned long>(seconds));

            //     break;
            // }

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

    void Renderer::DrawWiFiSetupConfirm()
    {
        constexpr int16_t POPUP_X = 45;
        constexpr int16_t POPUP_Y = 65;
        constexpr int16_t POPUP_W = 230;
        constexpr int16_t POPUP_H = 110;

        constexpr int16_t BUTTON_Y = 135;
        constexpr int16_t BUTTON_W = 75;
        constexpr int16_t BUTTON_H = 28;

        constexpr int16_t CANCEL_X = 70;
        constexpr int16_t OK_X = 175;

        //-------------------------------------------------
        // Popup background
        //-------------------------------------------------

        m_target->FillRect(
            POPUP_X,
            POPUP_Y,
            POPUP_W,
            POPUP_H,
            DisplayTheme::COLOR_BACKGROUND
        );

        m_target->DrawRect(
            POPUP_X,
            POPUP_Y,
            POPUP_W,
            POPUP_H,
            DisplayTheme::COLOR_VALUE,
            2U
        );

        //-------------------------------------------------
        // Title
        //-------------------------------------------------

        m_target->DrawText(
            160,
            80,
            "WiFi Setup",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Center
        );

        //-------------------------------------------------
        // Message
        //-------------------------------------------------

        m_target->DrawText(
            160,
            108,
            "Start setup mode?",
            DisplayTheme::COLOR_LABEL,
            1U,
            DisplayTypes::TextAlign::Center
        );

        //-------------------------------------------------
        // CANCEL button
        //-------------------------------------------------

        m_target->DrawRect(
            CANCEL_X,
            BUTTON_Y,
            BUTTON_W,
            BUTTON_H,
            DisplayTheme::COLOR_WARNING,
            1U
        );

        m_target->DrawText(
            CANCEL_X + (BUTTON_W / 2),
            BUTTON_Y + 8,
            "CANCEL",
            DisplayTheme::COLOR_WARNING,
            1U,
            DisplayTypes::TextAlign::Center
        );

        //-------------------------------------------------
        // OK button
        //-------------------------------------------------

        m_target->DrawRect(
            OK_X,
            BUTTON_Y,
            BUTTON_W,
            BUTTON_H,
            DisplayTheme::COLOR_SUCCESS,
            1U
        );

        m_target->DrawText(
            OK_X + (BUTTON_W / 2),
            BUTTON_Y + 8,
            "OK",
            DisplayTheme::COLOR_SUCCESS,
            1U,
            DisplayTypes::TextAlign::Center
        );
    }

    void Renderer::SetDeviceConfigMode(
        bool visible)
    {
        if (m_deviceConfigMode == visible)
        {
            return;
        }

        m_deviceConfigMode =
            visible;

        if (visible)
        {
            m_deviceConfigEdit =
                DeviceManager::GetConfiguration();
        }

        m_deviceConfigDirty =
            DeviceConfigDirty::None;

        m_deviceConfigDrawn =
            false;

        m_firstRender =
            true;
    }

    bool Renderer::IsDeviceConfigMode() const
    {
        return m_deviceConfigMode;
    }

    void Renderer::SetWiFiSetupConfirm(
        bool visible)
    {
        if (m_wifiSetupConfirm == visible)
        {
            return;
        }

        m_wifiSetupConfirm =
            visible;

        if (visible)
        {
            // 팝업을 다음 Render에서 딱 한 번 그리기
            m_wifiSetupConfirmDrawn =
                false;
        }
        else
        {
            // 팝업 제거 후 전체 페이지 복구
            m_wifiSetupConfirmDrawn =
                false;

            m_firstRender =
                true;
        }
    }

    bool Renderer::IsWiFiSetupConfirm() const
    {
        return m_wifiSetupConfirm;
    }

    void Renderer::SetWiFiSetupMode(
        bool active)
    {
        if (m_wifiSetupMode == active)
        {
            return;
        }

        m_wifiSetupMode =
            active;

        m_firstRender =
            true;
    }

    bool Renderer::IsWiFiSetupMode() const
    {
        return m_wifiSetupMode;
    }

    void Renderer::DrawWiFiSetupMode()
    {
        m_target->DrawText(
            160,
            50,
            "WiFi Setup Mode",
            DisplayTheme::COLOR_VALUE,
            2U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            160,
            95,
            "SSID: SVEMS-SETUP",
            DisplayTheme::COLOR_LABEL,
            1U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            160,
            125,
            "Open: 192.168.4.1",
            DisplayTheme::COLOR_SUCCESS,
            1U,
            DisplayTypes::TextAlign::Center
        );

        m_target->DrawText(
            160,
            160,
            "Save -> Restart",
            DisplayTheme::COLOR_LABEL,
            1U,
            DisplayTypes::TextAlign::Center
        );

        //-------------------------------------------------
        // Cancel
        //-------------------------------------------------

        m_target->DrawText(
            160,
            205,
            "CANCEL",
            DisplayTheme::COLOR_LABEL,
            2U,
            DisplayTypes::TextAlign::Center
        );
    }
}