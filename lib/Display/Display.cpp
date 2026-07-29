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

    delay(1000);
    
    g_pageManager.Next();

    delay(1000);

    g_pageManager.Next();

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
    // static uint32_t lastRenderMs = 0U;

    if (!g_initialized)
    {
        return;
    }

    // const uint32_t now = millis();

    // if (now - lastRenderMs < 1000U)
    // {
    //     return;
    // }

    // lastRenderMs = now;

    DisplayModelBuilder::Build(g_model);

    g_renderer.RenderPage(
        g_pageManager.Current(),
        g_model);
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
