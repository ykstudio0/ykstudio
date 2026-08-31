//-------------------------------------------------------------
// File : Scheduler.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Description : 시간을 관리
//-------------------------------------------------------------
#include <ArduinoJson.h>

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
#include "IoTService.h"
#include "VehicleInput.h"
#include "VehicleVoltageService.h"
#include "ChargeControlService.h"
#include "ReverseChargeTypes.h"
#include "ReverseChargeController.h"
#include "HttpTransport.h"

// ---------------------------------------------------------
// Reverse Charge
// ---------------------------------------------------------
static void UpdateReverseCharge();

static ReverseChargeController g_reverseChargeController;

static bool
    g_reverseChargeManualStart =
        false;

static bool
    g_reverseChargeManualStop =
        false;

namespace
{
    SVEMS::Device::TouchDevice Touch;

    constexpr uint32_t EPEVER_BACKOFF_MS              = 5000UL;
    constexpr uint32_t EPEVER_MIN_GAP_MS              = 1000UL;
    constexpr uint32_t EPEVER_SOLAR_INTERVAL_MS       = 3000UL;
    constexpr uint32_t EPEVER_BATTERY_INTERVAL_MS     = 15000UL;
    constexpr uint32_t EPEVER_LOAD_INTERVAL_MS        = 15000UL;
    constexpr uint32_t EPEVER_CHARGE_INTERVAL_MS      = 5000UL;
    constexpr uint32_t EPEVER_TEMPERATURE_INTERVAL_MS = 60000UL;
    constexpr uint32_t EPEVER_SOC_INTERVAL_MS         = 60000UL;
    constexpr uint32_t EPEVER_ENERGY_INTERVAL_MS      = 60000UL;

    uint32_t Timer100ms = 0;
    uint32_t Timer1sec  = 0;
    uint32_t Timer5sec  = 0;
    uint32_t Timer30sec = 0;
    uint32_t Timer60sec = 0;

    struct EpeverPollState
    {
        uint32_t lastTransaction = 0U;

        uint32_t solar = 0U;
        uint32_t battery = 0U;
        uint32_t load = 0U;
        uint32_t charging = 0U;
        uint32_t temperature = 0U;
        uint32_t soc = 0U;
        uint32_t energy = 0U;
    };

    EpeverPollState g_epeverPoll;
}

bool Scheduler::Begin()
{
    // char coreMessage[32];

    // snprintf(
    //     coreMessage,
    //     sizeof(coreMessage),
    //     "Core = %d",
    //     xPortGetCoreID());

    // Logger::Info(
    //     "SCHEDULER",
    //     coreMessage);

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

    if (!SVEMS::Service::IoTService::Begin())
    {
        Logger::Error(
            "SCHEDULER",
            "IoT service failed");
    }

    g_reverseChargeController.Begin();

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
    VehicleInput::Update();

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
                case SVEMS::UI::Action::WiFiSetup:
                    Logger::Info(
                        "UI",
                        "WiFi Setup");

                    Display::SetWiFiSetupConfirm(
                        true);

                    break;

                case SVEMS::UI::Action::Cancel:
                    Logger::Info(
                        "UI",
                        "WiFi Setup Cancel"
                    );

                    Display::SetWiFiSetupConfirm(
                        false);

                    break;

                case SVEMS::UI::Action::Confirm:
                    Logger::Info(
                        "UI",
                        "WiFi Setup Confirm"
                    );

                    Display::SetWiFiSetupConfirm(
                        false);

                    SVEMS::Service::WiFiService::
                        StartSetupMode();

                    {
                        Display::SetWiFiSetupMode(
                            true);
                    }
                    
                    break;
                
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

                case SVEMS::UI::Action::NextSubPage:
                    Logger::Info(
                        "UI",
                        "Next Sub Page");

                    Display::NextSubPage();
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

    SVEMS::Vehicle::VehicleVoltageService::Update();
    
    UpdateReverseCharge();

    String response;

    if (
        SVEMS::Transport::HttpTransport::
            FetchReverseChargeCommand(
                response
            )
    )
    {
        JsonDocument doc;

        const DeserializationError error =
            deserializeJson(
                doc,
                response
            );

        if (error)
        {
            Logger::Warning(
                "REV CMD",
                "JSON parse failed"
            );

            return;
        }

        JsonVariant command =
            doc["command"];

        //---------------------------------------------------------
        // No Command
        //---------------------------------------------------------

        if (
            command.isNull()
        )
        {
            return;
        }

        //---------------------------------------------------------
        // Command Type
        //---------------------------------------------------------

        const char* commandName =
            command["command"] |
            "";

        if (
            strcmp(
                commandName,
                "setMode"
            ) == 0
        )
        {
            const char* mode =
                command["mode"] |
                "";

            if (
                strcmp(
                    mode,
                    "Normal"
                ) == 0
            )
            {
                g_reverseChargeController.SetMode(
                    ReverseCharge::Mode::Normal
                );

                Logger::Info(
                    "REV CMD",
                    "SetMode = Normal"
                );
            }
            else if (
                strcmp(
                    mode,
                    "Soft"
                ) == 0
            )
            {
                g_reverseChargeController.SetMode(
                    ReverseCharge::Mode::Soft
                );

                Logger::Info(
                    "REV CMD",
                    "SetMode = Soft"
                );
            }
            else
            {
                Logger::Warning(
                    "REV CMD",
                    "Invalid Mode"
                );
            }
        }
        else if (
            strcmp(
                commandName,
                "hardStart"
            ) == 0
        )
        {
            g_reverseChargeController.SetMode(
                ReverseCharge::Mode::Hard
            );

            g_reverseChargeManualStart =
                true;

            Logger::Info(
                "REV CMD",
                "Hard Start"
            );
        }
        else if (
            strcmp(
                commandName,
                "hardStop"
            ) == 0
        )
        {
            g_reverseChargeManualStop =
                true;

            Logger::Info(
                "REV CMD",
                "Hard Stop"
            );
        }
        else
        {
            Logger::Warning(
                "REV CMD",
                "Unknown Command"
            );
        }
    }

    if constexpr (ENABLE_EPEVER_POLLING)
    {
        PollEpeverDistributed();
    }
}

// 5 Second Tasks
void Scheduler::Run5Sec()
{
}

// 30 Second Tasks
void Scheduler::Run30Sec()
{
}

// 60 Second Tasks
void Scheduler::Run60Sec()
{
}

// Poll Solar Information
bool Scheduler::PollSolar()
{
    return Epever::ReadSolar();
}

// Poll Energy Information
bool Scheduler::PollEnergy()
{
    Logger::Info(
        "ENERGY",
        "PollEnergy called"
    );

    return Epever::ReadEnergy();
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
    return Epever::ReadChargingStatus();
}
// void Scheduler::UpdateDisplay()
// {

// }

void Scheduler::Service()
{
    // if constexpr (ENABLE_BMS_SERVICE)
    // {
    //     SVEMS::Service::BMSService::Update();
    // }

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

    SVEMS::Service::IoTService::Update();
}

void Scheduler::PollEpeverDistributed()
{
    static uint8_t nextPoll = 0U;

    const uint32_t now =
        millis();

    if (now - g_epeverPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
    {
        return;
    }

    for (uint8_t i = 0U; i < 7U; i++)
    {
        const uint8_t poll =
            nextPoll;

        nextPoll =
            static_cast<uint8_t>(
                (nextPoll + 1U) % 7U);

        switch (poll)
        {
            case 0:
                if (now - g_epeverPoll.solar >=
                    EPEVER_SOLAR_INTERVAL_MS)
                {
                    PollSolar();

                    g_epeverPoll.solar = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 1:
                if (now - g_epeverPoll.battery >=
                    EPEVER_BATTERY_INTERVAL_MS)
                {
                    PollBattery();

                    g_epeverPoll.battery = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 2:
                if (now - g_epeverPoll.load >=
                    EPEVER_LOAD_INTERVAL_MS)
                {
                    PollLoad();

                    g_epeverPoll.load = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 3:
                if (now - g_epeverPoll.charging >=
                    EPEVER_CHARGE_INTERVAL_MS)
                {
                    PollChargingStatus();

                    g_epeverPoll.charging = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 4:
                if (now - g_epeverPoll.temperature >=
                    EPEVER_TEMPERATURE_INTERVAL_MS)
                {
                    PollTemperature();

                    g_epeverPoll.temperature = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 5:
                if (now - g_epeverPoll.soc >=
                    EPEVER_SOC_INTERVAL_MS)
                {
                    PollSOC();

                    g_epeverPoll.soc = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;

            case 6:
                if (now - g_epeverPoll.energy >=
                    EPEVER_ENERGY_INTERVAL_MS)
                {
                    PollEnergy();

                    g_epeverPoll.energy = now;
                    g_epeverPoll.lastTransaction = now;

                    return;
                }
                break;
        }
    }
}

static void UpdateReverseCharge()
{
    ReverseCharge::Input input;

    input.ig2Active =
        DataManager::Vehicle.active;

    input.vehicleVoltage =
        DataManager::VehicleBattery.voltage;

    input.voltageValid =
        DataManager::VehicleBattery.status.online;

    input.manualStart =
        g_reverseChargeManualStart;

    input.manualStop =
        g_reverseChargeManualStop;

    g_reverseChargeController.Update(
        input
    );

    g_reverseChargeManualStart =
        false;

    g_reverseChargeManualStop =
        false;

    DataManager::ReverseCharge.mode =
        g_reverseChargeController.GetMode();

    DataManager::ReverseCharge.state =
        g_reverseChargeController.GetState();

    DataManager::ReverseCharge.safety =
        g_reverseChargeController.GetSafetyReason();

    DataManager::ReverseCharge.enabled =
        g_reverseChargeController.IsChargeEnabled();

    SVEMS::Vehicle::ChargeControlService::Update(
        DataManager::ReverseCharge.enabled
    );

    SVEMS::Vehicle::ChargeControlService::Update(
        g_reverseChargeController.IsChargeEnabled()
    );

    String message;

    message.reserve(
        96
    );

    message +=
        "Mode=";

    message +=
        String(
            static_cast<int>(
                g_reverseChargeController.GetMode()
            )
        );

    message +=
        " State=";

    message +=
        String(
            static_cast<int>(
                g_reverseChargeController.GetState()
            )
        );

    message +=
        " Safety=";

    message +=
        String(
            static_cast<int>(
                g_reverseChargeController.GetSafetyReason()
            )
        );

    message +=
        " Enabled=";

    message +=
        g_reverseChargeController.IsChargeEnabled()
            ? "1"
            : "0";

    message +=
        " V=";

    message +=
        String(
            input.vehicleVoltage,
            2
        );

    message +=
        " IG2=";

    message +=
        input.ig2Active
            ? "1"
            : "0";

    Logger::Info(
        "REVCHG",
        message
    );
}