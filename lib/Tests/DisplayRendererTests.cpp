//-------------------------------------------------------------
// File : DisplayRendererTests.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-23
// Project : SVEMS
// Version : 0.3.3
// Description : Render tests for DisplayRenderer
//-------------------------------------------------------------

#include <Arduino.h>

#include "DisplayRenderer.h"
#include "DisplayModel.h"
#include "SerialRenderTarget.h"
#include "IRenderTarget.h"
#include "Tests.h"

namespace
{
    void TestOverviewRendering(
        IRenderTarget& target)
    {
        DisplayModel::Model model;

        auto& system =
            model.GetSystem();

        system.currentTime.SetValue(
            14.0f * 3600.0f +
            25.0f * 60.0f +
            37.0f);

        system.wifiConnected = true;
        system.rs485Ready = true;
        system.modbusReady = true;
        system.deviceManagerReady = true;

        model.GetSolar().power.SetValue(162.5f);
        model.GetBattery().voltage.SetValue(13.42f);
        model.GetBattery().percent.SetValue(84.0f);
        model.GetLoad().power.SetValue(7.2f);
        model.GetTemperature().cabinTemperature.SetValue(26.8f);
        model.GetTemperature().cabinHumidity.SetValue(61.0f);

        model.SyncOverview();

        DisplayRenderer::Renderer renderer;

        if (!renderer.Begin(target))
        {
            Serial.println(
                F("[FAIL] [DISPLAY RENDERER] Begin failed"));

            return;
        }

        if (!renderer.RenderPage(
                DisplayPages::Page::Overview,
                0U,
                model))
        {
            Serial.println(
                F("[FAIL] [DISPLAY RENDERER] Render failed"));

            return;
        }

        Serial.println(
            F("[PASS] [DISPLAY RENDERER] Overview rendered"));

        if (!renderer.RenderPage(
                DisplayPages::Page::Battery,
                1U,
                model))
        {
            Serial.println(
                F("[FAIL] [DISPLAY SUBPAGE] Render failed"));

            return;
        }

        Serial.println(
            F("[PASS] [DISPLAY SUBPAGE] Battery rendered"));
    }
}

namespace Tests
{
    void RunDisplayRendererTests(
        IRenderTarget& target)
    {
        Serial.println();
        Serial.println(
            F("========== DISPLAY RENDERER TESTS =========="));

        TestOverviewRendering(target);

        Serial.println(
            F("============================================"));
    }
}