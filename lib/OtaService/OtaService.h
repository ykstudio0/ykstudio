//-------------------------------------------------------------
// File : OtaService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-19
// Project : SVEMS
// Version : 0.8.3
// Description : OtaService
//-------------------------------------------------------------

#pragma once

namespace SVEMS::Service
{
    class OtaService
    {
    public:
        static bool CheckForUpdate();

        static bool PerformUpdate(
            const char* firmwareFile,
            const char* expectedSha256
        );

        static bool ConfirmRunningFirmware();

        static bool IsPendingVerification();
    };
}