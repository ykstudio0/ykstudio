//-------------------------------------------------------------
// File : Scheduler.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Description : 시간을 관리
//-------------------------------------------------------------

#include "Scheduler.h"
#include "Config.h"
#include "Epever.h"
#include "Logger.h"
#include "DataManager.h"
#include "Display.h"
#include "DeviceManager.h"
#include "TimeService.h"
#include "WiFiService.h"
#include "NtpService.h"
#include "TouchDevice.h"
#include "TouchManager.h"
#include "DisplayLayout.h"
#include "WidgetEventManager.h"
#include "UiAction.h"
#include "BMSService.h"
namespace
{
    SVEMS::Device::TouchDevice Touch;

    constexpr uint32_t EPEVER_BACKOFF_MS = 5000UL;

    bool g_epeverOnline = true;

    uint32_t g_epeverNextProbeTime = 0U;

    uint32_t Timer100ms = 0;
    uint32_t Timer1sec  = 0;
    uint32_t Timer5sec  = 0;
    uint32_t Timer30sec = 0;
    uint32_t Timer60sec = 0;

    void SetEpeverOffline(
        uint32_t now)
    {
        g_epeverOnline = false;

        g_epeverNextProbeTime =
            now + EPEVER_BACKOFF_MS;
    }

    void SetEpeverOnline()
    {
        g_epeverOnline = true;

        g_epeverNextProbeTime = 0U;
    }
}

bool Scheduler::Begin()
{
    if (!Touch.Begin())
    {
        Logger::Error(
            "SCHEDULER",
            "Touch device failed");
    }
    else if (!SVEMS::Manager::TouchManager::Begin(
        Touch))
    {
        Logger::Error(
            "SCHEDULER",
            "Touch manager failed");
    }

    uint32_t now = millis();
    Timer100ms = now;
    Timer1sec  = now;
    Timer5sec  = now;
    Timer30sec = now;
    Timer60sec = now;

    return true;
}

void Scheduler::RunInitial()
{

}

void Scheduler::Run()
{
    if constexpr (ENABLE_BMS_SERVICE)
    {
        SVEMS::Service::BMSService::Update();
    }
    
    uint32_t now = millis();

    while (now - Timer100ms >= 100)
    {
        Timer100ms += 100;
        Run100ms();
    }

    while (now - Timer1sec >= 1000)
    {
        Timer1sec += 1000;
        Run1Sec();
    }

    while (now - Timer5sec >= 5000)
    {
        Timer5sec += 5000;
        Run5Sec();
    }

    while (now - Timer30sec >= 30000)
    {
        Timer30sec += 30000;
        Run30Sec();
    }

    while (now - Timer60sec >= 60000)
    {
        Timer60sec += 60000;
        Run60Sec();
    }

    Service();
}

// 0.1 Second Tasks
void Scheduler::Run100ms()
{
    if (!SVEMS::Manager::TouchManager::Update())
    {
        return;
    }

    const SVEMS::Touch::Event event =
        SVEMS::Manager::TouchManager::GetEvent();

    if (event ==
        SVEMS::Touch::Event::None)
    {
        return;
    }

    SVEMS::Touch::TouchPoint point;

    SVEMS::Manager::TouchManager::GetPoint(
        point);

    char message[40];

    switch (event)
    {
        case SVEMS::Touch::Event::Pressed:
            snprintf(
                message,
                sizeof(message),
                "Pressed X=%u Y=%u",
                static_cast<unsigned int>(
                    point.x),
                static_cast<unsigned int>(
                    point.y));

            Logger::Info(
                "TOUCH",
                message);

            break;

        case SVEMS::Touch::Event::Released:
                snprintf(
                    message,
                    sizeof(message),
                    "Released X=%u Y=%u",
                    static_cast<unsigned int>(
                        point.x),
                    static_cast<unsigned int>(
                        point.y));

                Logger::Info(
                    "TOUCH",
                    message);
            
            break;

        case SVEMS::Touch::Event::Tap:
        {
            snprintf(
                message,
                sizeof(message),
                "Tap X=%u Y=%u",
                static_cast<unsigned int>(
                    point.x),
                static_cast<unsigned int>(
                    point.y));

            Logger::Info(
                "TOUCH",
                message);

            const SVEMS::UI::Action action =
                SVEMS::Manager::WidgetEventManager::Process(
                    event,
                    point);

            switch (action)
            {
                case SVEMS::UI::Action::PreviousPage:
                    Logger::Info(
                        "UI",
                        "Previous Page");

                    Display::PreviousPage();
                    break;

                case SVEMS::UI::Action::NextPage:
                    Logger::Info(
                        "UI",
                        "Next Page");

                    Display::NextPage();
                    break;

                case SVEMS::UI::Action::SelectPage:
                    Logger::Info(
                        "UI",
                        "Select Page");

                    // 향후 페이지 선택 Popup 연결
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }
}

// 1 Second Tasks
void Scheduler::Run1Sec()
{
    DeviceManager::Update();
    
    if constexpr (ENABLE_EPEVER_POLLING)
    {
        const uint32_t now =
            millis();

        //-------------------------------------------------
        // EPEVER Online
        //-------------------------------------------------

        if (g_epeverOnline)
        {
            if (!Epever::ReadSolar())
            {
                SetEpeverOffline(now);
            }

            return;
        }

        //-------------------------------------------------
        // EPEVER Offline
        //
        // Backoff 기간에는 아무 요청도 하지 않는다.
        //-------------------------------------------------

        if (static_cast<int32_t>(
                now -
                g_epeverNextProbeTime) < 0)
        {
            return;
        }

        //-------------------------------------------------
        // Recovery Probe
        //-------------------------------------------------

        if (Epever::ReadSolar())
        {
            SetEpeverOnline();

            Logger::Info(
                "EPEVER",
                "Communication Restored");
        }
        else
        {
            g_epeverNextProbeTime =
                now +
                EPEVER_BACKOFF_MS;
        }
    }
}

// 5 Second Tasks
void Scheduler::Run5Sec()
{
    if constexpr (ENABLE_EPEVER_POLLING)
    {
        if (!g_epeverOnline)
        {
            return;
        }

        const uint32_t now =
            millis();

        if (!PollBattery())
        {
            SetEpeverOffline(now);
            return;
        }

        if (!PollLoad())
        {
            SetEpeverOffline(now);
            return;
        }

        if (!PollChargingStatus())
        {
            SetEpeverOffline(now);
            return;
        }
    }
}

// 30 Second Tasks
void Scheduler::Run30Sec()
{
    if constexpr (ENABLE_EPEVER_POLLING)
    {
        if (!g_epeverOnline)
        {
            return;
        }

        const uint32_t now =
            millis();

        if (!PollTemperature())
        {
            SetEpeverOffline(now);
            return;
        }

        if (!PollSOC())
        {
            SetEpeverOffline(now);
            return;
        }
    }
}

// 60 Second Tasks
void Scheduler::Run60Sec()
{
    if constexpr (ENABLE_EPEVER_POLLING)
    {
    }
}

// Poll Solar Information
bool Scheduler::PollSolar()
{
    return Epever::ReadSolar();
}

// Poll Battery Information
bool Scheduler::PollBattery()
{
    return Epever::ReadBattery();
}

bool Scheduler::PollLoad()
{
    return Epever::ReadLoad();
}

// Poll Temperature Information
bool Scheduler::PollTemperature()
{
    return Epever::ReadTemperature();
}

// Poll SOC Information
bool Scheduler::PollSOC()
{
    return Epever::ReadSOC();
}

bool Scheduler::PollChargingStatus()
{
    Epever::ReadChargingStatus();
}
// void Scheduler::UpdateDisplay()
// {

// }

void Scheduler::Service()
{
    if constexpr (ENABLE_BMS_SERVICE)
    {
        SVEMS::Service::BMSService::Update();
    }

    DataManager::UpdateOnlineStatus(millis());

    SVEMS::Service::TimeService::Update();
    
    ServiceLogger();
    ServiceDisplay();
    ServiceIoT();

    DataManager::ClearUpdates();
}

void Scheduler::ServiceLogger()
{
    char buffer[32];

    if (DataManager::Solar.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Solar.voltage);
        Logger::Info("PV", buffer);
        
        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Solar.current);
        Logger::Info("PV", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Solar.power);
        Logger::Info("PV", buffer);
    }

    if (DataManager::Battery.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Battery.voltage);
        Logger::Info("BATTERY", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Battery.current);
        Logger::Info("BATTERY", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Battery.power);
        Logger::Info("BATTERY", buffer);
    }
    
    if (DataManager::Load.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Load.voltage);
        Logger::Info("LOAD", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Load.current);
        Logger::Info("LOAD", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Load.power);
        Logger::Info("LOAD", buffer);
    }

    if (DataManager::Temperature.controllerStatus.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f ℃", 
            DataManager::Temperature.battery);
        Logger::Info("BAT TEMP", buffer);
        
        snprintf(buffer, sizeof(buffer), "%.1f ℃", 
            DataManager::Temperature.controllerBoard);
        Logger::Info("DEV TEMP", buffer);
    }

    if (DataManager::Soc.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%u %%", 
            static_cast<unsigned int>(DataManager::Soc.value));
        Logger::Info("SOC", buffer);
    }
}

void Scheduler::ServiceDisplay()
{
    static uint32_t previousMs = 0U;

    const uint32_t now = millis();

    if (now - previousMs < 500U)
    {
        return;
    }

    previousMs = now;
    
    Display::Update();
}

void Scheduler::ServiceIoT()
{
    // Wi-Fi / MQTT / Web 모듈 구현 후 연결
    SVEMS::Service::WiFiService::Update();

    SVEMS::Service::NtpService::Update();
}