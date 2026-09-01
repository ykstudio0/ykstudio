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

    Logger::Info("DISPLAY", "Ready");

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

    // TEST :: Auto Page Rotation (Temprary)
    // static uint32_t lastPageChange = 0;

    // if (millis() - lastPageChange >= 5000)
    // {
    //     lastPageChange = millis();
    //     g_pageManager.Next();
    // }
    // TEST :: Auto Page Rotation (Temprary End)

    DisplayModelBuilder::Build(g_model);

    g_renderer.RenderPage(
        g_pageManager.Current(),
        g_pageManager.SubPage(),
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

void Display::NextSubPage()
{
    if (!g_initialized)
    {
        return;
    }

    g_pageManager.NextSubPage();
}

DisplayPages::Page Display::CurrentPage()
{
    if (!g_initialized)
    {
        return DisplayPages::Page::Overview;
    }

    return g_pageManager.Current();
}

void Display::SetWiFiSetupConfirm(
    bool visible)
{
    if (!g_initialized)
    {
        return;
    }

    g_renderer.SetWiFiSetupConfirm(
        visible);
}

bool Display::IsWiFiSetupConfirm()
{
    if (!g_initialized)
    {
        return false;
    }

    return g_renderer.IsWiFiSetupConfirm();
}

void Display::SetWiFiSetupMode(
    bool active)
{
    if (!g_initialized)
    {
        return;
    }

    g_renderer.SetWiFiSetupMode(
        active);
}

void Display::SetDeviceConfigMode(
    bool visible)
{
    if (!g_initialized)
    {
        return;
    }

    g_renderer.SetDeviceConfigMode(
        visible);
}

bool Display::IsDeviceConfigMode()
{
    if (!g_initialized)
    {
        return false;
    }

    return g_renderer.IsDeviceConfigMode();
}

void Display::ToggleDeviceMppt()
{
    g_renderer.ToggleDeviceMppt();
}

void Display::ToggleDeviceBms()
{
    g_renderer.ToggleDeviceBms();
}

void Display::ToggleDeviceSht40()
{
    g_renderer.ToggleDeviceSht40();
}

void Display::ToggleDeviceRtc()
{
    g_renderer.ToggleDeviceRtc();
}

bool Display::SaveDeviceConfig()
{
    if (!g_initialized)
    {
        return false;
    }

    const auto& config =
        g_renderer.GetDeviceConfigEdit();

    //-------------------------------------------------
    // 먼저 NVS 저장
    //-------------------------------------------------

    if (!SVEMS::Device::
            DeviceConfigurationStorage::Save(
                config))
    {
        Logger::Warning(
            "DEV CFG",
            "Save Failed");

        return false;
    }

    //-------------------------------------------------
    // 저장 성공 후 Runtime 적용
    //-------------------------------------------------

    DeviceManager::SetConfiguration(
        config);

    Logger::Info(
        "DEV CFG",
        "Applied");

    //-------------------------------------------------
    // Device Config 종료
    //-------------------------------------------------

    g_renderer.SetDeviceConfigMode(
        false);

    return true;
}

void Display::CancelDeviceConfig()
{
    if (!g_initialized)
    {
        return;
    }

    Logger::Info(
        "DEV CFG",
        "Cancelled");

    g_renderer.SetDeviceConfigMode(
        false);
}