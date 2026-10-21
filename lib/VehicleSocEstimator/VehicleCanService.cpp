//-------------------------------------------------------------
// File : VehicleCanService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-17
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : VehicleCanService
//-------------------------------------------------------------

#include "VehicleCanService.h"
#include "Logger.h"

VehicleCanReceiver VehicleCanService::Receiver;
bool VehicleCanService::Ready = false;
uint32_t VehicleCanService::RxCount = 0;
VehicleEngineMonitor VehicleCanService::EngineMonitor;
VehicleIbsParser VehicleCanService::IbsParser;

bool VehicleCanService::Begin()
{
    if (Ready)
    {
        return true;
    }

    EngineMonitor.Begin();
    IbsParser.Begin();

    Ready = Receiver.Begin();

    return Ready;
}

void VehicleCanService::Update()
{
    if (!Ready)
    {
        return;
    }

    twai_message_t message;

    //-------------------------------------------------------------
    // CAN Update Interval Diagnostic
    //-------------------------------------------------------------
    static uint32_t lastUpdateMillis = 0;
    static uint32_t maxIntervalMillis = 0;
    static uint32_t lastReportMillis = 0;

    const uint32_t currentMillis = millis();

    if (lastUpdateMillis != 0)
    {
        const uint32_t interval =
            currentMillis - lastUpdateMillis;

        if (interval > maxIntervalMillis)
        {
            maxIntervalMillis = interval;
        }
    }

    lastUpdateMillis = currentMillis;

    if (currentMillis - lastReportMillis >= 5000)
    {
        lastReportMillis = currentMillis;

        Logger::Info(
            "VEH CAN",
            "Max Update Interval=" +
            String(maxIntervalMillis) +
            " ms");

        maxIntervalMillis = 0;
    }

    uint8_t processed = 0;

    while (processed < 64 &&
       Receiver.ReceiveRaw(message))
    {
        ++processed;
        ++RxCount;

        const uint32_t frameMillis = millis();

        EngineMonitor.OnCanFrame(
            message.identifier,
            message.data,
            message.data_length_code,
            frameMillis);

        IbsParser.OnCanFrame(
            message.identifier,
            message.data,
            message.data_length_code,
            frameMillis);
    }

    EngineMonitor.Update(millis());
}

bool VehicleCanService::IsReady()
{
    return Ready;
}

//-------------------------------------------------------------
// PrintStatus
// Description:
//   Print vehicle CAN receiver statistics.
//-------------------------------------------------------------
void VehicleCanService::PrintStatus()
{
    if (!Ready)
    {
        Logger::Warning(
            "VEH CAN",
            "Not Ready");

        return;
    }

    twai_status_info_t status = {};

    if (twai_get_status_info(&status) != ESP_OK)
    {
        Logger::Warning(
            "VEH CAN",
            "Status Read Failed");

        return;
    }

    char message[128];

    snprintf(
        message,
        sizeof(message),
        "RX=%lu MISSED=%lu OVERRUN=%lu QUEUED=%lu",
        static_cast<unsigned long>(RxCount),
        static_cast<unsigned long>(status.rx_missed_count),
        static_cast<unsigned long>(status.rx_overrun_count),
        static_cast<unsigned long>(status.msgs_to_rx));

    Logger::Info(
        "VEH CAN",
        message);

    const uint32_t nowMillis = millis();

    const char* engineState = "Unknown";

    switch (EngineMonitor.GetState())
    {
        case VehicleEngineState::EngineRunning:
            engineState = "Running";
            break;

        case VehicleEngineState::EngineStopped:
            engineState = "Stopped";
            break;

        default:
            break;
    }

    char engineMessage[128];

    snprintf(
        engineMessage,
        sizeof(engineMessage),
        "RPM=%.1f State=%s Fresh=%s",
        EngineMonitor.GetRpm(),
        engineState,
        EngineMonitor.IsRpmFresh(nowMillis)
            ? "Yes"
            : "No"
    );

    Logger::Info(
        "VEH ENG",
        engineMessage);

    //-------------------------------------------------------------
    // IBS SOC Status
    //-------------------------------------------------------------
    char ibsMessage[96];

    if (IbsParser.HasSoc())
    {
        snprintf(
            ibsMessage,
            sizeof(ibsMessage),
            "SOC=%u%% Fresh=%s LastMs=%lu",
            static_cast<unsigned int>(IbsParser.GetSoc()),
            IbsParser.IsSocFresh(millis()) ? "Yes" : "No",
            static_cast<unsigned long>(
                IbsParser.GetLastSocMillis()));
    }
    else
    {
        snprintf(
            ibsMessage,
            sizeof(ibsMessage),
            "SOC=Unavailable");
    }

    Logger::Info(
        "VEH IBS",
        ibsMessage);
}

//-------------------------------------------------------------
// Engine Status
//-------------------------------------------------------------
VehicleEngineState VehicleCanService::GetEngineState()
{
    return EngineMonitor.GetState();
}

float VehicleCanService::GetEngineRpm()
{
    return EngineMonitor.GetRpm();
}

bool VehicleCanService::IsEngineRpmFresh()
{
    return EngineMonitor.IsRpmFresh(millis());
}

//-------------------------------------------------------------
// IBS SOC
//-------------------------------------------------------------
bool VehicleCanService::HasIbsSoc()
{
    return IbsParser.HasSoc();
}

uint8_t VehicleCanService::GetIbsSoc()
{
    return IbsParser.GetSoc();
}

bool VehicleCanService::IsIbsSocFresh()
{
    return IbsParser.IsSocFresh(millis());
}

uint32_t VehicleCanService::GetLastIbsSocMillis()
{
    return IbsParser.GetLastSocMillis();
}