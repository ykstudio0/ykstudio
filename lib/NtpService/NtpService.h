//-------------------------------------------------------------
// File : NtpService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.5.5
// Description : Non-blocking NTP time synchronization service
//-------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <time.h>

namespace SVEMS::Service
{
    class NtpService
    {
    public:
        static void Begin();

        static void Update();

        static bool IsSynchronized();

        static bool GetLocalTime(
            struct tm& timeInfo);

    private:
        static void StartSynchronization();

        static bool ApplySynchronizedTime(
            const struct tm& timeInfo);

        static void OnTimeSynchronized(
            struct timeval* timeValue);

        static bool Configured;
        static bool Synchronized;
        static bool AppliedToRtc;

        static volatile bool SyncNotificationPending;
        
        static uint32_t LastAttemptMs;
        static uint32_t SyncStartedMs;
        static uint32_t LastSuccessfulSyncMs;
    };
}