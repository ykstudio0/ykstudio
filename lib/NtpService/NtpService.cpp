//-------------------------------------------------------------
// File : NtpService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.5.5
// Description : Non-blocking NTP time synchronization service
//-------------------------------------------------------------

#include <Arduino.h>
#include <time.h>
#include <esp_sntp.h>

#include "NtpService.h"
#include "WiFiService.h"
#include "Logger.h"
#include "TimeService.h"
#include "RTCDateTime.h"
#include "DeviceManager.h"

namespace
{
    constexpr uint32_t NTP_RETRY_INTERVAL_MS =
        30000UL;

    constexpr uint32_t NTP_SYNC_TIMEOUT_MS =
        15000UL;

    constexpr uint32_t NTP_RESYNC_INTERVAL_MS =
        24UL * 60UL * 60UL * 1000UL;

    // 2024-01-01 00:00:00 UTC
    // 시스템 시간이 초기 기본값인지 판단하기 위한 기준값
    constexpr time_t VALID_EPOCH_TIME =
        1704067200;

    constexpr char TIME_ZONE[] =
        "KST-9";

    constexpr char NTP_SERVER_1[] =
        // "pool.ntp.org";
        "time2.kriss.re.kr";

    constexpr char NTP_SERVER_2[] =
        "time.google.com";

    constexpr char NTP_SERVER_3[] =
        "time.windows.com";
}

namespace SVEMS::Service
{
    bool NtpService::Configured = false;

    bool NtpService::Synchronized = false;

    bool NtpService::AppliedToRtc = false;

    uint32_t NtpService::LastAttemptMs = 0U;

    uint32_t NtpService::SyncStartedMs = 0U;

    void NtpService::Begin()
    {
        Configured = false;
        Synchronized = false;
        AppliedToRtc = false;

        SyncNotificationPending = false;

        LastAttemptMs = 0U;
        SyncStartedMs = 0U;
        LastSuccessfulSyncMs = 0U;

        sntp_set_time_sync_notification_cb(
            OnTimeSynchronized);

        sntp_set_sync_interval(
            NTP_RESYNC_INTERVAL_MS);

        Logger::Info(
            "NTP",
            "Ready");
    }

    void NtpService::OnTimeSynchronized(
        struct timeval* timeValue)
    {
        (void)timeValue;
        
        SyncNotificationPending = true;
    }

    void NtpService::Update()
    {
        const uint32_t nowMs =
            millis();

        if (!WiFiService::IsConnected())
        {
            return;
        }

        if (!Configured)
        {
            if (LastAttemptMs != 0U &&
                (nowMs - LastAttemptMs) <
                    NTP_RETRY_INTERVAL_MS)
            {
                return;
            }

            StartSynchronization();

            return;
        }

        if (SyncNotificationPending)
        {
            SyncNotificationPending = false;

            const time_t currentTime =
                time(nullptr);

            if (currentTime < VALID_EPOCH_TIME)
            {
                Logger::Warning(
                    "NTP",
                    "Invalid synchronized time");

                return;
            }

            struct tm timeInfo;

            if (localtime_r(
                    &currentTime,
                    &timeInfo) == nullptr)
            {
                Logger::Error(
                    "NTP",
                    "Local time conversion failed");

                return;
            }

            Synchronized = true;
            AppliedToRtc = false;

            Logger::Info(
                "NTP",
                "Synchronized");

            char message[40];

            snprintf(
                message,
                sizeof(message),
                "%04d-%02d-%02d %02d:%02d:%02d",
                timeInfo.tm_year + 1900,
                timeInfo.tm_mon + 1,
                timeInfo.tm_mday,
                timeInfo.tm_hour,
                timeInfo.tm_min,
                timeInfo.tm_sec);

            Logger::Info(
                "NTP",
                message);

            if (DeviceManager::GetConfiguration().rtc)
            {
                AppliedToRtc =
                    ApplySynchronizedTime(
                        timeInfo);
            }
            else
            {
                AppliedToRtc = false;
            }

            LastSuccessfulSyncMs =
                nowMs;
        }

        if (!Synchronized &&
            (nowMs - SyncStartedMs) >=
            NTP_SYNC_TIMEOUT_MS)
        {
            Configured = false;

            Logger::Warning(
                "NTP",
                "Sync timeout");
        }
    }

    bool NtpService::IsSynchronized()
    {
        return Synchronized;
    }

    bool NtpService::GetLocalTime(
        struct tm& timeInfo)
    {
        if (!Synchronized)
        {
            return false;
        }

        const time_t currentTime =
            time(nullptr);

        if (currentTime < VALID_EPOCH_TIME)
        {
            return false;
        }

        return
            localtime_r(
                &currentTime,
                &timeInfo) != nullptr;
    }

    void NtpService::StartSynchronization()
    {
        Configured = true;
        Synchronized = false;
        AppliedToRtc = false;
        
        LastAttemptMs =
            millis();

        SyncStartedMs =
            LastAttemptMs;

        Configured = true;

        Synchronized = false;

        Logger::Info(
            "NTP",
            "Synchronizing...");

        configTzTime(
            TIME_ZONE,
            NTP_SERVER_1,
            NTP_SERVER_2,
            NTP_SERVER_3);
    }

    bool NtpService::ApplySynchronizedTime(
        const struct tm& timeInfo)
    {
        SVEMS::Device::RTCDateTime rtcTime;

        rtcTime.year =
            static_cast<uint16_t>(
                timeInfo.tm_year + 1900);

        rtcTime.month =
            static_cast<uint8_t>(
                timeInfo.tm_mon + 1);

        rtcTime.day =
            static_cast<uint8_t>(
                timeInfo.tm_mday);

        rtcTime.hour =
            static_cast<uint8_t>(
                timeInfo.tm_hour);

        rtcTime.minute =
            static_cast<uint8_t>(
                timeInfo.tm_min);

        rtcTime.second =
            static_cast<uint8_t>(
                timeInfo.tm_sec);

        // tm_wday: 일요일 0 ~ 토요일 6
        // 현재 RTCDateTime 규칙: 월요일 1 ~ 일요일 7
        if (timeInfo.tm_wday == 0)
        {
            rtcTime.dayOfWeek = 7U;
        }
        else
        {
            rtcTime.dayOfWeek =
                static_cast<uint8_t>(
                    timeInfo.tm_wday);
        }

        if (!TimeService::Synchronize(
                rtcTime))
        {
            Logger::Error(
                "NTP",
                "RTC update failed");

            return false;
        }

        Logger::Info(
            "NTP",
            "RTC updated");

        return true;
    }

    volatile bool
        NtpService::SyncNotificationPending = false;

    uint32_t
        NtpService::LastSuccessfulSyncMs = 0U;
}