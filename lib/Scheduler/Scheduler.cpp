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
#include "RS485.h"
#include "ModbusRTU.h"
#include "DisplayPowerManager.h"
#include "DataManager.h"
#include "OtaService.h"
#include "ChargeRelayDriver.h"
#include "StatusLED.h"
#include "ChargeProtection.h"
#include "VehicleCanService.h"
#include "VehicleBatteryCoordinator.h"

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

    //-------------------------------------------------------------
    // EPEVER Asynchronous Poll Type
    //-------------------------------------------------------------
    enum class EpeverAsyncPollType : uint8_t
    {
        None = 0,
        Solar,
        Battery,
        Load,
        Charging,
        Temperature,
        Soc,
        Energy
    };

    //-------------------------------------------------------------
    // EPEVER Asynchronous Poll Context
    //-------------------------------------------------------------
    struct EpeverAsyncPollContext
    {
        EpeverAsyncPollType activePoll =
            EpeverAsyncPollType::None;

        EpeverAsyncPollType nextPoll =
            EpeverAsyncPollType::Solar;

        uint32_t lastTransaction = 0;
    };
    static EpeverAsyncPollContext g_epeverAsyncPoll;
}

//-------------------------------------------------------------
// EPEVER Solar Async Test Mode
//-------------------------------------------------------------
static constexpr bool EPEVER_SOLAR_BATTERY_ASYNC_TEST = true;

// ---------------------------------------------------------
// Reverse Charge
// ---------------------------------------------------------
static void UpdateReverseCharge();

static ReverseChargeController g_reverseChargeController;

// test
// static uint32_t g_reverseChargeTestStartedAt = 0;

// static bool g_reverseChargeTestHardStarted =
//     false;

// static bool g_reverseChargeTestHardStopped =
//     false;
// test end

static bool
    g_reverseChargeManualStart =
        false;

static bool
    g_reverseChargeManualStop =
        false;

static char
    g_reverseChargePendingCommand[256] =
        { 0 };

static portMUX_TYPE
    g_reverseChargeCommandMux =
        portMUX_INITIALIZER_UNLOCKED;

static void ProcessReverseChargeCommand(
    const String& response
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

    if (
        command.isNull()
    )
    {
        return;
    }

    const char* commandName =
        command["command"] |
        "";

    //---------------------------------------------------------
    // Set Mode
    //---------------------------------------------------------

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

        return;
    }

    //---------------------------------------------------------
    // Hard Start
    //---------------------------------------------------------

    if (
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

        return;
    }

    //---------------------------------------------------------
    // Hard Stop
    //---------------------------------------------------------

    if (
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

        return;
    }

    //---------------------------------------------------------
    // Reset Communication Counters
    //---------------------------------------------------------

    if (
        strcmp(
            commandName,
            "resetCounters"
        ) == 0
    )
    {
        DataManager::CommStats.ResetCounters();

        SVEMS::Transport::HttpTransport::ResetCounters();

        ++DataManager::SystemRuntime.counterResetCount;

        Logger::Info(
            "SYSTEM",
            "Communication Counters Reset"
        );

        return;
    }

    if (strcmp(commandName, "otaUpdate") == 0)
    {
        if (DataManager::Vehicle.active)
        {
            Logger::Warning(
                "OTA",
                "Blocked: IG2 ON"
            );

            return;
        }

        if (
            SVEMS::Vehicle::ChargeRelayDriver::
                IsEnabled()
        )
        {
            Logger::Warning(
                "OTA",
                "Blocked: Reverse Charge ON"
            );

            return;
        }

        if (
            !SVEMS::Service::WiFiService::
                IsConnected()
        )
        {
            Logger::Warning(
                "OTA",
                "Blocked: WiFi Offline"
            );

            return;
        }
        
        if (
            strcmp(
                SVEMS_DEVICE_ID,
                "main-car"
            ) == 0 &&
            !DataManager::VehicleBattery.status.online
        )
        {
            Logger::Warning(
                "OTA",
                "Blocked: Vehicle Voltage Invalid"
            );

            return;
        }

        Logger::Info(
            "OTA",
            "Manual Update Requested"
        );

        StatusLED::SetOtaState(
            OtaLedState::Checking
        );
        
        SVEMS::Service::OtaService::
            CheckForUpdate();

        return;
    }

    //---------------------------------------------------------
    // Unknown Command
    //---------------------------------------------------------

    Logger::Warning(
        "REV CMD",
        commandName
    );

    Logger::Warning(
        "REV CMD",
        "Unknown Command"
    );
}

static void ConsumeReverseChargePendingCommand()
{
    char commandBuffer[256] =
        { 0 };

    //---------------------------------------------------------
    // Copy Pending Command
    //---------------------------------------------------------

    portENTER_CRITICAL(
        &g_reverseChargeCommandMux
    );

    memcpy(
        commandBuffer,
        g_reverseChargePendingCommand,
        sizeof(commandBuffer)
    );

    g_reverseChargePendingCommand[0] =
        '\0';

    portEXIT_CRITICAL(
        &g_reverseChargeCommandMux
    );

    //---------------------------------------------------------
    // No Pending Command
    //---------------------------------------------------------

    if (
        commandBuffer[0] ==
        '\0'
    )
    {
        return;
    }

    //---------------------------------------------------------
    // Process Command
    //---------------------------------------------------------

    ProcessReverseChargeCommand(
        String(
            commandBuffer
        )
    );
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
    VehicleCanService::Update();

    if constexpr (ENABLE_BMS_SERVICE)
    {
        SVEMS::Service::BMSService::Update();
    }

    ChargeProtection::Update();

    //---------------------------------------------------------
    // Asynchronous Modbus Processing
    //---------------------------------------------------------
    if constexpr (ENABLE_EPEVER_POLLING)
    {
        if constexpr (EPEVER_SOLAR_BATTERY_ASYNC_TEST)
        {
            ModbusRTU::UpdateAsync();

            PollSolarAsync();
            PollBatteryAsync();
            PollLoadAsync();
            PollChargingAsync();
            PollTemperatureAsync();
            PollSocAsync();
            PollEnergyAsync();
        }
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

    DisplayPowerManager::Update();

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

    static bool consumeWakeTouch =
        false;

    //-------------------------------------------------
    // Wake touch
    //-------------------------------------------------
    if (
        event ==
        SVEMS::Touch::Event::Pressed
    )
    {
        const bool wokeDisplay =
            DisplayPowerManager::NotifyActivity();

        if (wokeDisplay)
        {
            consumeWakeTouch  =
                true;

            return;
        }
    }

    //---------------------------------------------
    // Wake touch 전체 소비
    //---------------------------------------------

    if (consumeWakeTouch)
    {
        if (
            event ==
            SVEMS::Touch::Event::Tap
        )
        {
            consumeWakeTouch =
                false;
        }

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

                case SVEMS::UI::Action::WiFiSetupModeCancel:
                    Logger::Info(
                        "UI",
                        "WiFi Setup Mode Cancel");

                    SVEMS::Service::WiFiService::
                        StopSetupMode();

                    Display::SetWiFiSetupMode(
                        false);

                    break;

                case SVEMS::UI::Action::DeviceConfig:
                    Logger::Info(
                        "UI",
                        "Device Config");

                    Display::SetDeviceConfigMode(
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

                case SVEMS::UI::Action::DeviceMpptToggle:
                    ::Display::ToggleDeviceMppt();
                    break;

                case SVEMS::UI::Action::DeviceBmsToggle:
                    ::Display::ToggleDeviceBms();
                    break;

                case SVEMS::UI::Action::DeviceSht40Toggle:
                    ::Display::ToggleDeviceSht40();
                    break;

                case SVEMS::UI::Action::DeviceRtcToggle:
                    ::Display::ToggleDeviceRtc();
                    break;

                case SVEMS::UI::Action::DeviceConfigSave:
                    Logger::Info(
                        "UI",
                        "Device Config Save");

                    ::Display::SaveDeviceConfig();
                    break;

                case SVEMS::UI::Action::DeviceConfigCancel:
                    Logger::Info(
                        "UI",
                        "Device Config Cancel");

                    ::Display::CancelDeviceConfig();
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

    ConsumeReverseChargePendingCommand();

    // ---------------------------------------------------------
    // Reverse Charge Controller
    // ---------------------------------------------------------

    UpdateReverseCharge();

    Logger::Info(
        "RS485",
        "NoRx=" +
        String(
            RS485::GetConsecutiveNoRxCount()
        )
    );

    Logger::Info(
        "MODBUS",
        "FrameErr=" +
        String(
            ModbusRTU::GetConsecutiveFrameErrorCount()
        )
    );

    if constexpr (ENABLE_EPEVER_POLLING)
    {
        if constexpr (EPEVER_SOLAR_BATTERY_ASYNC_TEST)
        {
            if (g_epeverAsyncPoll.activePoll ==
                EpeverAsyncPollType::None)
            {
                EpeverAsyncPollType candidate =
                    g_epeverAsyncPoll.nextPoll;

                // Solar / Battery / Load / Charging
                // Temperature / SOC / Energy
                for (uint8_t i = 0; i < 7; ++i)
                {
                    switch (candidate)
                    {
                        case EpeverAsyncPollType::Solar:
                            StartSolarAsyncPoll();
                            candidate = EpeverAsyncPollType::Battery;
                            break;

                        case EpeverAsyncPollType::Battery:
                            StartBatteryAsyncPoll();
                            candidate = EpeverAsyncPollType::Load;
                            break;

                        case EpeverAsyncPollType::Load:
                            StartLoadAsyncPoll();
                            candidate = EpeverAsyncPollType::Charging;
                            break;

                        case EpeverAsyncPollType::Charging:
                            StartChargingAsyncPoll();
                            candidate = EpeverAsyncPollType::Temperature;
                            break;

                        case EpeverAsyncPollType::Temperature:
                            StartTemperatureAsyncPoll();
                            candidate = EpeverAsyncPollType::Soc;
                            break;

                        case EpeverAsyncPollType::Soc:
                            StartSocAsyncPoll();
                            candidate = EpeverAsyncPollType::Energy;
                            break;

                        case EpeverAsyncPollType::Energy:
                            StartEnergyAsyncPoll();
                            candidate = EpeverAsyncPollType::Solar;
                            break;

                        default:
                            candidate = EpeverAsyncPollType::Solar;
                            break;
                    }

                    //-------------------------------------------------
                    // Successful transaction start
                    //-------------------------------------------------
                    if (g_epeverAsyncPoll.activePoll !=
                        EpeverAsyncPollType::None)
                    {
                        g_epeverAsyncPoll.nextPoll = candidate;
                        break;
                    }
                }
            }
        }
        else
        {
            PollEpeverDistributed();
        }
    }
    VehicleBatteryCoordinator::Update();
}

// 5 Second Tasks
void Scheduler::Run5Sec()
{
    VehicleCanService::PrintStatus();
    VehicleBatteryCoordinator::PrintStatus();
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

//-------------------------------------------------------------
// Scheduler - Asynchronous Solar Polling
//-------------------------------------------------------------
void Scheduler::PollSolarAsync()
{
    //---------------------------------------------------------
    // Only handle an active Solar transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Solar)
    {
        return;
    }

    const ModbusRTU::AsyncState state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Waiting - Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
    {
        return;
    }

    //---------------------------------------------------------
    // Success - Process received Solar data
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncSolarResponse())
        {
            Epever::HandleAsyncSolarFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Failed - Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncSolarFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Idle - Request start handled in a later step
    //---------------------------------------------------------
}

//-------------------------------------------------------------
// Scheduler - Start Asynchronous Solar Poll
//-------------------------------------------------------------
void Scheduler::StartSolarAsyncPoll()
{
    //---------------------------------------------------------
    // Check MPPT availability
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
    {
        return;
    }

    //---------------------------------------------------------
    // Prevent overlapping transactions
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
    {
        return;
    }

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
    {
        return;
    }

    //---------------------------------------------------------
    // Check polling intervals
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
    {
        return;
    }

    if (now - g_epeverPoll.solar <
        EPEVER_SOLAR_INTERVAL_MS)
    {
        return;
    }

    //---------------------------------------------------------
    // Start Solar transaction
    //---------------------------------------------------------
    if (Epever::StartAsyncSolar())
    {
        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Solar;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.solar = now;
    }

    if (Epever::StartAsyncSolar())
    {
        Logger::Info("EPEVER ASYNC", "START SOLAR");

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Solar;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.solar = now;
    }
}

//-------------------------------------------------------------
// Process asynchronous EPEVER Battery polling result
//-------------------------------------------------------------
void Scheduler::PollBatteryAsync()
{
    //---------------------------------------------------------
    // Only handle active Battery transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Battery)
    {
        return;
    }

    const auto state = ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Response still pending
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
    {
        return;
    }

    //---------------------------------------------------------
    // Response received successfully
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncBatteryResponse())
        {
            Epever::HandleAsyncBatteryFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Transaction failed
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncBatteryFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "Battery transaction unexpectedly idle"
        );

        Epever::HandleAsyncBatteryFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

//-------------------------------------------------------------
// Start asynchronous EPEVER Battery polling
//-------------------------------------------------------------
void Scheduler::StartBatteryAsyncPoll()
{
    //---------------------------------------------------------
    // RS485 communication required
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Another asynchronous transaction is active
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    const uint32_t now = millis();

    //---------------------------------------------------------
    // Minimum interval between Modbus requests
    //---------------------------------------------------------
    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    //---------------------------------------------------------
    // Battery polling interval
    //---------------------------------------------------------
    if (now - g_epeverPoll.battery <
        EPEVER_BATTERY_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start non-blocking Battery request
    //---------------------------------------------------------
    if (Epever::StartAsyncBattery())
    {
        Logger::Info("EPEVER ASYNC", "START BATTERY");

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Battery;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.battery = now;
    }
}

void Scheduler::StartLoadAsyncPoll()
{
    //---------------------------------------------------------
    // RS485 communication required
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Another asynchronous transaction is active
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    const uint32_t now = millis();

    //---------------------------------------------------------
    // Minimum interval between Modbus requests
    //---------------------------------------------------------
    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    //---------------------------------------------------------
    // Load polling interval
    //---------------------------------------------------------
    if (now - g_epeverPoll.load <
        EPEVER_LOAD_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start non-blocking Load request
    //---------------------------------------------------------
    if (Epever::StartAsyncLoad())
    {
        Logger::Info(
            "EPEVER ASYNC",
            "START LOAD"
        );

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Load;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.load = now;
    }
}

void Scheduler::PollLoadAsync()
{
    //---------------------------------------------------------
    // Check active poll
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Load)
        return;

    const auto state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
        return;

    //---------------------------------------------------------
    // Successful response
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncLoadResponse())
        {
            Epever::HandleAsyncLoadFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncLoadFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "Load unexpected Idle state"
        );

        Epever::HandleAsyncLoadFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

void Scheduler::StartChargingAsyncPoll()
{
    //---------------------------------------------------------
    // Check RS485 availability
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Check active transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    //---------------------------------------------------------
    // Check polling intervals
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    if (now - g_epeverPoll.charging <
        EPEVER_CHARGE_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start asynchronous request
    //---------------------------------------------------------
    if (Epever::StartAsyncChargingStatus())
    {
        Logger::Info(
            "EPEVER ASYNC",
            "START CHARGING"
        );

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Charging;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.charging = now;
    }
}

void Scheduler::PollChargingAsync()
{
    //---------------------------------------------------------
    // Check active poll
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Charging)
        return;

    const auto state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
        return;

    //---------------------------------------------------------
    // Successful response
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncChargingStatusResponse())
        {
            Epever::HandleAsyncChargingStatusFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncChargingStatusFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "Charging unexpected Idle state"
        );

        Epever::HandleAsyncChargingStatusFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

void Scheduler::StartTemperatureAsyncPoll()
{
    //---------------------------------------------------------
    // Check RS485 availability
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Check active transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    //---------------------------------------------------------
    // Check polling intervals
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    if (now - g_epeverPoll.temperature <
        EPEVER_TEMPERATURE_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start asynchronous request
    //---------------------------------------------------------
    if (Epever::StartAsyncTemperature())
    {
        Logger::Info(
            "EPEVER ASYNC",
            "START TEMPERATURE"
        );

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Temperature;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.temperature = now;
    }
}

void Scheduler::PollTemperatureAsync()
{
    //---------------------------------------------------------
    // Check active poll
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Temperature)
        return;

    const auto state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
        return;

    //---------------------------------------------------------
    // Successful response
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncTemperatureResponse())
        {
            Epever::HandleAsyncTemperatureFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncTemperatureFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "Temperature unexpected Idle state"
        );

        Epever::HandleAsyncTemperatureFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

void Scheduler::StartSocAsyncPoll()
{
    //---------------------------------------------------------
    // Check RS485 availability
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Check active transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    //---------------------------------------------------------
    // Check polling intervals
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    if (now - g_epeverPoll.soc <
        EPEVER_SOC_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start asynchronous request
    //---------------------------------------------------------
    if (Epever::StartAsyncSoc())
    {
        Logger::Info(
            "EPEVER ASYNC",
            "START SOC"
        );

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Soc;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.soc = now;
    }
}

void Scheduler::PollSocAsync()
{
    //---------------------------------------------------------
    // Check active poll
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Soc)
        return;

    const auto state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
        return;

    //---------------------------------------------------------
    // Successful response
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncSocResponse())
        {
            Epever::HandleAsyncSocFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncSocFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "SOC unexpected Idle state"
        );

        Epever::HandleAsyncSocFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

void Scheduler::PollEnergyAsync()
{
    //---------------------------------------------------------
    // Check active poll
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::Energy)
        return;

    const auto state =
        ModbusRTU::GetAsyncState();

    //---------------------------------------------------------
    // Transaction in progress
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Waiting)
        return;

    //---------------------------------------------------------
    // Successful response
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Success)
    {
        if (!Epever::ProcessAsyncEnergyResponse())
        {
            Epever::HandleAsyncEnergyFailure();
        }

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Communication failure
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Failed)
    {
        Epever::HandleAsyncEnergyFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;

        return;
    }

    //---------------------------------------------------------
    // Unexpected Idle state
    //---------------------------------------------------------
    if (state == ModbusRTU::AsyncState::Idle)
    {
        Logger::Warning(
            "EPEVER ASYNC",
            "Energy unexpected Idle state"
        );

        Epever::HandleAsyncEnergyFailure();

        ModbusRTU::ResetAsync();

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::None;
    }
}

void Scheduler::StartEnergyAsyncPoll()
{
    //---------------------------------------------------------
    // Check RS485 availability
    //---------------------------------------------------------
    if (!DeviceManager::IsRs485Required())
        return;

    //---------------------------------------------------------
    // Check active transaction
    //---------------------------------------------------------
    if (g_epeverAsyncPoll.activePoll !=
        EpeverAsyncPollType::None)
        return;

    if (ModbusRTU::GetAsyncState() !=
        ModbusRTU::AsyncState::Idle)
        return;

    //---------------------------------------------------------
    // Check polling intervals
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (now - g_epeverAsyncPoll.lastTransaction <
        EPEVER_MIN_GAP_MS)
        return;

    if (now - g_epeverPoll.energy <
        EPEVER_ENERGY_INTERVAL_MS)
        return;

    //---------------------------------------------------------
    // Start asynchronous request
    //---------------------------------------------------------
    if (Epever::StartAsyncEnergy())
    {
        Logger::Info(
            "EPEVER ASYNC",
            "START ENERGY"
        );

        g_epeverAsyncPoll.activePoll =
            EpeverAsyncPollType::Energy;

        g_epeverAsyncPoll.lastTransaction = now;
        g_epeverPoll.energy = now;
    }
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
    if (!DeviceManager::IsRs485Required())
    {
        return;
    }
    
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

    // SVEMS::Vehicle::ChargeControlService::Update(
    //     g_reverseChargeController.IsChargeEnabled()
    // );

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

void Scheduler::SetReverseChargePendingCommand(
    const String& command
)
{
    char buffer[256] =
        { 0 };

    command.toCharArray(
        buffer,
        sizeof(buffer)
    );

    portENTER_CRITICAL(
        &g_reverseChargeCommandMux
    );

    memcpy(
        g_reverseChargePendingCommand,
        buffer,
        sizeof(
            g_reverseChargePendingCommand
        )
    );

    portEXIT_CRITICAL(
        &g_reverseChargeCommandMux
    );
}
