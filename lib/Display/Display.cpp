//-------------------------------------------------------------
// File : Display.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : Display Service implementation
//-------------------------------------------------------------

#include "Display.h"
#include "Logger.h"
#include "LGFX_Config.h"
#include "TFTRenderTarget.h"
#include "DisplayRenderer.h"
#include "DisplayModel.h"
#include "PageManager.h"
#include "DisplayModelBuilder.h"

namespace
{
    bool g_initialized = false;

    // 실제 LovyanGFX 디스플레이 장치
    LGFX_SVEMS g_displayDevice;

    // LovyanGFX를 IRenderTarget으로 변환
    TFTRenderTarget g_renderTarget(
        g_displayDevice);

    // 화면 랜더링 담당
    DisplayRenderer::Renderer g_renderer;

    // 현재 페이지 관리
    PageManager::Manager g_pageManager;

    DisplayModel::Model g_model;
}

bool Display::Begin()
{
    g_initialized = false;

    g_pageManager.Begin();

    if (!InitializeLCD())
    {
        Logger::Error("DISPLAY", "Init failed");
            return false;
    }

    if (!g_renderer.Begin(g_renderTarget))
    {
        Logger::Error(
            "DISPLAY", "Renderer init failed");

        return false;
    }

    g_initialized = true;

    Logger::Info("DISPLAY", "Redady");

    return true;
}

bool Display::InitializeLCD()
{
    return g_renderTarget.Begin();
}

void Display::Update()
{
    // static uint32_t updateCount = 0U;

    if (!g_initialized)
    {
        // Serial.println("[DISPLAY] Not initialized");
        return;
    }

    // ++updateCount;

    // Serial.printf(
    //     "[DISPLAY] Update #%1u - Build begin\n",
    //     static_cast<unsigned long>(updateCount));

    DisplayModelBuilder::Build(g_model);

    // Serial.println(
        // "[DISPLAY] Build complete");
    
    // const bool renderResult =
    g_renderer.RenderPage(
        g_pageManager.Current(),
        g_model);

    // Serial.printf(
    //     "[DISPLAY] Render result: %s\n",
    //     renderResult ? "OK" : "FALSED");

    // TODO
    // 1. DataManager -> DisplayModel
    // 2. 현재 페이지 랜더링
}

void Display::NextPage()
{
    if (!g_initialized)
    {
        return;
    }

    g_pageManager.Next();
}

void Display::PreviousPage()
{
    if (!g_initialized)
    {
        return;
    }

    g_pageManager.Previous();
}
