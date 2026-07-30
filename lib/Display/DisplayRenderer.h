//-------------------------------------------------------------
// File : DisplayRenderer.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.3
// Description : Display renderer
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "DisplayPages.h"
#include "DisplayLayout.h"
#include "DisplayTheme.h"
#include "DisplayModel.h"
#include "DisplayTypes.h"
#include "IRenderTarget.h"

namespace DisplayRenderer
{
    class Renderer
    {
    public:
        Renderer();

        // Renderer 초기화
        bool Begin(
            IRenderTarget& target);

        // 사용 가능 여부
        bool IsReady() const;

        // 지정한 페이지를 랜더링한다.
        bool RenderPage(
            DisplayPages::Page page,
            const DisplayModel::Model& model);

    private:
        // Common
        void DrawHeader(
            DisplayPages::Page page,
            const DisplayModel::OverviewData& overview,
            const DisplayModel::SystemData& system);

        void DrawFooter(
            DisplayPages::Page page);

        void DrawContent(
            DisplayPages::Page page,
            const DisplayModel::Model& model);

        // Individual Pages
        void DrawOverview(
            const DisplayModel::OverviewData& data);

        void DrawSolar(
            const DisplayModel::SolarData& data);

        void DrawBattery(
            const DisplayModel::BatteryData& data);

        void DrawLoad(
            const DisplayModel::LoadData& data);

        void DrawTemperature(
            const DisplayModel::TemperatureData& data);

        void DrawSystem(
            const DisplayModel::SystemData& data);

        // Drawing Helpers
        void DrawLabelValue(
            const char* label,
            const DisplayTypes::DisplayValue& value,
            uint8_t row);

        void DrawValue(
            const DisplayTypes::DisplayValue& value,
            int16_t x,
            int16_t y);

        void FormatValue(
            const DisplayTypes::DisplayValue& value,
            char* buffer,
            size_t bufferSize) const;
        
        void DrawHeaderStatus(
            const DisplayModel::SystemData& system);

        // Members
        IRenderTarget* m_target;

        bool m_ready;

        static constexpr size_t VALUE_BUFFER_SIZE = 32U;

        DisplayModel::Model m_lastModel;

        DisplayPages::Page m_lastPage =
            DisplayPages::Page::Overview;

        bool m_firstRender = true;
        bool m_pageChanged = false;

        void DrawStatic(
            DisplayPages::Page page);

        void DrawDynamic(
            DisplayPages::Page page,
            const DisplayModel::Model& model);

        void DrawLabelText(
            const char* label,
            const char* text,
            uint8_t row);

        bool HasValueChanged(
            const DisplayTypes::DisplayValue& current,
            const DisplayTypes::DisplayValue& previous) const;

        bool HasTextChanged(
            const char* current,
            const char* previous) const;

        //-------------------------------------------------------------
        // Header Helpers
        //-------------------------------------------------------------
        void BuildEnergyDisplay(
            DisplayModel::EnergyStatus status,
            const char*& text,
            DisplayTheme::Color& color);

        void BuildTimeText(
            const DisplayModel::SystemData& system,
            char* buffer,
            size_t bufferSize);

        void BuildStatusText(
            const DisplayModel::SystemData& system,
            char* buffer,
            size_t bufferSize);
    };
}