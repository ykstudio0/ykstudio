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

        static bool AppliedToRtc;
        
        static bool Configured;

        static bool Synchronized;

        static uint32_t LastAttemptMs;

        static uint32_t SyncStartedMs;
    };
}