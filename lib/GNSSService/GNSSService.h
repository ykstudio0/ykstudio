
 //-------------------------------------------------------------
 // File : GNSSService.h
 // Author : JongOh Kim + ChatGPT
 // Date : 2026-11-13
 // Project : SVEMS
 // Version : 0.9.0
 // Description : GNSS positioning service framework
 //-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Service
{
    struct GNSSData
    {
        bool valid = false;

        double latitude = 0.0;
        double longitude = 0.0;

        float altitude = 0.0f;
        float speedKmh = 0.0f;

        uint8_t satellites = 0U;
        uint8_t fixQuality = 0U;

        float hdop = 0.0f;

        uint64_t utcTimestamp = 0ULL;
        uint32_t lastUpdateMs = 0U;
    };

    class GNSSService
    {
    public:
        static bool Begin();

        // Returns the latest GNSS data snapshot.
        static GNSSData GetData();

        static bool IsReady();
        static bool HasValidData();

    private:
        static void WorkerTask(
            void* parameter
        );

        static bool Ready;
        static GNSSData Data;
    };
}
