
 //-------------------------------------------------------------
 // File : GNSSService.cpp
 // Author : JongOh Kim + ChatGPT
 // Date : 2026-11-13
 // Project : SVEMS
 // Version : 0.9.0
 // Description : GNSS positioning service implementation
 //-------------------------------------------------------------

#include "GNSSService.h"
#include "Pins.h"
#include "DFRobot_GNSS.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

namespace
{
    constexpr uint32_t GNSS_BAUD = 9600U;
    constexpr size_t NMEA_BUFFER_SIZE = 128U;

    // Protect the shared GNSSData snapshot.
    SemaphoreHandle_t gnssDataMutex = nullptr;

    HardwareSerial GNSSSerial(1);

    DFRobot_GNSS_UART gnss(
        &GNSSSerial,
        GNSS_BAUD,
        PIN_GNSS_RX,
        PIN_GNSS_TX
    );

    char nmeaBuffer[NMEA_BUFFER_SIZE] = {};
    size_t nmeaLength = 0U;

    uint32_t nmeaValidCount = 0U;
    uint32_t nmeaInvalidCount = 0U;
    uint32_t nmeaOverflowCount = 0U;

    uint32_t nmeaChecksumErrorCount = 0U;
    uint32_t nmeaIncompleteCount = 0U;

    // NMEA error diagnostics
    char nmeaLastChecksumError[NMEA_BUFFER_SIZE] = {};
    char nmeaLastIncomplete[NMEA_BUFFER_SIZE] = {};

    int HexValue(char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';

        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;

        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;

        return -1;
    }

    bool ValidateNmea(const char* sentence, size_t length)
    {
        if (length < 4U || sentence[0] != '$')
            return false;

        size_t starPos = length;

        for (size_t i = 1U; i < length; ++i)
        {
            if (sentence[i] == '*')
            {
                starPos = i;
                break;
            }
        }

        if (starPos == length ||
            starPos + 3U != length)
        {
            return false;
        }

        const int high = HexValue(sentence[starPos + 1U]);
        const int low  = HexValue(sentence[starPos + 2U]);

        if (high < 0 || low < 0)
            return false;

        uint8_t checksum = 0U;

        for (size_t i = 1U; i < starPos; ++i)
        {
            checksum ^= static_cast<uint8_t>(sentence[i]);
        }

        const uint8_t received =
            static_cast<uint8_t>((high << 4) | low);

        return checksum == received;
    }

    //---------------------------------------------------------
    // GGA Parsing
    //---------------------------------------------------------

    struct GGAResult
    {
        bool parsed = false;

        double latitude = 0.0;
        double longitude = 0.0;

        float altitude = 0.0f;
        float hdop = 0.0f;

        uint8_t satellites = 0U;
        uint8_t fixQuality = 0U;

        uint32_t receivedMs = 0U;
    };

    GGAResult latestGGA{};

    uint32_t ggaParseCount = 0U;
    uint32_t ggaErrorCount = 0U;

    //---------------------------------------------------------
    // Extract NMEA Field
    //---------------------------------------------------------

    bool GetNmeaField(
        const char* sentence,
        uint8_t fieldIndex,
        char* output,
        size_t outputSize
    )
    {
        if (sentence == nullptr ||
            output == nullptr ||
            outputSize == 0U)
        {
            return false;
        }

        output[0] = '\0';

        const char* start = sentence;
        uint8_t currentField = 0U;

        while (*start != '\0' &&
               *start != '*')
        {
            if (currentField == fieldIndex)
            {
                size_t length = 0U;

                while (start[length] != '\0' &&
                       start[length] != ',' &&
                       start[length] != '*')
                {
                    ++length;
                }

                if (length == 0U ||
                    length >= outputSize)
                {
                    return false;
                }

                memcpy(output, start, length);
                output[length] = '\0';

                return true;
            }

            if (*start == ',')
                ++currentField;

            ++start;
        }

        return false;
    }

    //---------------------------------------------------------
    // Parse Numeric Field
    //---------------------------------------------------------

    bool ParseNumber(
        const char* sentence,
        uint8_t index,
        double& value
    )
    {
        char field[24] = {};

        if (!GetNmeaField(
                sentence,
                index,
                field,
                sizeof(field)))
        {
            return false;
        }

        char* end = nullptr;

        value = strtod(field, &end);

        return end != field &&
               *end == '\0' &&
               isfinite(value);
    }

    //---------------------------------------------------------
    // Convert NMEA Coordinate to Decimal Degrees
    //---------------------------------------------------------

    bool ParseCoordinate(
        const char* sentence,
        uint8_t valueIndex,
        uint8_t hemisphereIndex,
        bool isLatitude,
        double& result
    )
    {
        double raw = 0.0;

        if (!ParseNumber(sentence, valueIndex, raw))
            return false;

        char hemisphere[4] = {};

        if (!GetNmeaField(
                sentence,
                hemisphereIndex,
                hemisphere,
                sizeof(hemisphere)))
        {
            return false;
        }

        if (hemisphere[1] != '\0')
            return false;

        if (raw < 0.0)
            return false;

        const int degrees = static_cast<int>(raw / 100.0);
        const double minutes = raw - degrees * 100.0;

        if (minutes < 0.0 || minutes >= 60.0)
            return false;

        result = degrees + minutes / 60.0;

        if (isLatitude)
        {
            if (degrees > 90 ||
                (degrees == 90 && minutes > 0.0))
                return false;

            if (hemisphere[0] == 'S')
                result = -result;
            else if (hemisphere[0] != 'N')
                return false;
        }
        else
        {
            if (degrees > 180 ||
                (degrees == 180 && minutes > 0.0))
                return false;

            if (hemisphere[0] == 'W')
                result = -result;
            else if (hemisphere[0] != 'E')
                return false;
        }

        return true;
    }

    //---------------------------------------------------------
    // Parse GGA
    //---------------------------------------------------------

    bool ParseGGA(
        const char* sentence,
        GGAResult& result
    )
    {
        GGAResult parsed{};

        if (!ParseCoordinate(
                sentence, 2U, 3U, true,
                parsed.latitude))
            return false;

        if (!ParseCoordinate(
                sentence, 4U, 5U, false,
                parsed.longitude))
            return false;

        double value = 0.0;

        if (!ParseNumber(sentence, 6U, value) ||
            value < 0.0 || value > 255.0 ||
            value != static_cast<int>(value))
            return false;

        parsed.fixQuality = static_cast<uint8_t>(value);

        if (!ParseNumber(sentence, 7U, value) ||
            value < 0.0 || value > 255.0 ||
            value != static_cast<int>(value))
            return false;

        parsed.satellites = static_cast<uint8_t>(value);

        if (!ParseNumber(sentence, 8U, value) ||
            value < 0.0)
            return false;

        parsed.hdop = static_cast<float>(value);

        if (!ParseNumber(sentence, 9U, value))
            return false;

        parsed.altitude = static_cast<float>(value);

        parsed.parsed = true;
        result = parsed;

        return true;
    }


    //---------------------------------------------------------
    // RMC Parsing
    //---------------------------------------------------------

    struct RMCResult
    {
        bool parsed = false;
        bool valid = false;

        double latitude = 0.0;
        double longitude = 0.0;

        float speedKmh = 0.0f;

        char utcTime[16] = {};
        char utcDate[8] = {};

        uint32_t receivedMs = 0U;
    };

    RMCResult latestRMC{};

    uint32_t rmcParseCount = 0U;
    uint32_t rmcErrorCount = 0U;

    //---------------------------------------------------------
    // Validate UTC Time / Date
    //---------------------------------------------------------

    bool ParseRmcUtc(
        const char* sentence,
        RMCResult& result
    )
    {
        if (!GetNmeaField(
                sentence, 1U,
                result.utcTime,
                sizeof(result.utcTime)))
            return false;

        if (!GetNmeaField(
                sentence, 9U,
                result.utcDate,
                sizeof(result.utcDate)))
            return false;

        // hhmmss or hhmmss.sss
        const size_t timeLen = strlen(result.utcTime);

        if (timeLen < 6U)
            return false;

        for (size_t i = 0U; i < 6U; ++i)
        {
            if (result.utcTime[i] < '0' ||
                result.utcTime[i] > '9')
                return false;
        }

        if (timeLen > 6U)
        {
            if (result.utcTime[6] != '.' ||
                timeLen == 7U)
                return false;

            for (size_t i = 7U; i < timeLen; ++i)
            {
                if (result.utcTime[i] < '0' ||
                    result.utcTime[i] > '9')
                    return false;
            }
        }

        const int hour =
            (result.utcTime[0] - '0') * 10 +
            (result.utcTime[1] - '0');

        const int minute =
            (result.utcTime[2] - '0') * 10 +
            (result.utcTime[3] - '0');

        const int second =
            (result.utcTime[4] - '0') * 10 +
            (result.utcTime[5] - '0');

        if (hour > 23 ||
            minute > 59 ||
            second > 59)
            return false;

        // ddmmyy
        if (strlen(result.utcDate) != 6U)
            return false;

        for (size_t i = 0U; i < 6U; ++i)
        {
            if (result.utcDate[i] < '0' ||
                result.utcDate[i] > '9')
                return false;
        }

        const int day =
            (result.utcDate[0] - '0') * 10 +
            (result.utcDate[1] - '0');

        const int month =
            (result.utcDate[2] - '0') * 10 +
            (result.utcDate[3] - '0');

        return day >= 1 && day <= 31 &&
               month >= 1 && month <= 12;
    }

    //---------------------------------------------------------
    // Parse RMC
    //---------------------------------------------------------

    bool ParseRMC(
        const char* sentence,
        RMCResult& result
    )
    {
        RMCResult parsed{};

        if (!ParseRmcUtc(sentence, parsed))
            return false;

        char status[4] = {};

        if (!GetNmeaField(
                sentence, 2U,
                status, sizeof(status)))
            return false;

        if (status[1] != '\0')
            return false;

        if (status[0] != 'A' &&
            status[0] != 'V')
            return false;

        parsed.valid = (status[0] == 'A');

        // A = valid navigation data
        // V = navigation data invalid
        if (parsed.valid)
        {
            if (!ParseCoordinate(
                    sentence, 3U, 4U, true,
                    parsed.latitude))
                return false;

            if (!ParseCoordinate(
                    sentence, 5U, 6U, false,
                    parsed.longitude))
                return false;

            double speedKnots = 0.0;

            if (!ParseNumber(
                    sentence, 7U, speedKnots))
                return false;

            if (speedKnots < 0.0)
                return false;

            // 1 knot = 1.852 km/h
            parsed.speedKmh =
                static_cast<float>(
                    speedKnots * 1.852
                );
        }

        parsed.parsed = true;
        result = parsed;

        return true;
    }

    //---------------------------------------------------------
    // RMC UTC -> Unix Timestamp (seconds)
    // Supported years: 2000 - 2099
    //---------------------------------------------------------

    bool ConvertRmcUtcToUnix(
        const RMCResult& rmc,
        uint64_t& timestamp)
    {
        timestamp = 0ULL;

        if (!rmc.parsed)
            return false;

        const char* d = rmc.utcDate;
        const char* t = rmc.utcTime;

        const int year = 2000 +
            (d[4] - '0') * 10 + (d[5] - '0');

        const int month =
            (d[2] - '0') * 10 + (d[3] - '0');

        const int day =
            (d[0] - '0') * 10 + (d[1] - '0');

        const int hour =
            (t[0] - '0') * 10 + (t[1] - '0');

        const int minute =
            (t[2] - '0') * 10 + (t[3] - '0');

        const int second =
            (t[4] - '0') * 10 + (t[5] - '0');

        if (month < 1 || month > 12 ||
            hour > 23 || minute > 59 || second > 59)
        {
            return false;
        }

        const auto isLeapYear = [](int y) -> bool
        {
            return (y % 4 == 0) &&
                   ((y % 100 != 0) || (y % 400 == 0));
        };

        const int daysInMonth[12] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        int maxDay = daysInMonth[month - 1];

        if (month == 2 && isLeapYear(year))
            maxDay = 29;

        if (day < 1 || day > maxDay)
            return false;

        // Days from 1970-01-01 to 2000-01-01.
        uint64_t days = 10957ULL;

        for (int y = 2000; y < year; ++y)
            days += isLeapYear(y) ? 366ULL : 365ULL;

        for (int m = 1; m < month; ++m)
        {
            days += static_cast<uint64_t>(
                daysInMonth[m - 1]
            );

            if (m == 2 && isLeapYear(year))
                ++days;
        }

        days += static_cast<uint64_t>(day - 1);

        timestamp =
            days * 86400ULL +
            static_cast<uint64_t>(hour) * 3600ULL +
            static_cast<uint64_t>(minute) * 60ULL +
            static_cast<uint64_t>(second);

        return true;
    }

    //---------------------------------------------------------
    // GNSS Fix Validation
    //---------------------------------------------------------

    constexpr uint32_t GNSS_DATA_TIMEOUT_MS = 15000U;
    constexpr uint32_t GNSS_PAIR_MAX_DIFF_MS = 3000U;
    constexpr uint32_t GNSS_STALE_TIMEOUT_MS = 30000U;

    bool IsGnssFixValid(uint32_t nowMs)
    {
        // Both sentences must be parsed.
        if (!latestGGA.parsed || !latestRMC.parsed)
            return false;

        // GGA must report a valid fix.
        if (latestGGA.fixQuality == 0U)
            return false;

        // RMC must report valid navigation data.
        if (!latestRMC.valid)
            return false;

        const uint32_t ggaAge =
            nowMs - latestGGA.receivedMs;

        const uint32_t rmcAge =
            nowMs - latestRMC.receivedMs;

        // Reject stale data.
        if (ggaAge > GNSS_DATA_TIMEOUT_MS ||
            rmcAge > GNSS_DATA_TIMEOUT_MS)
        {
            return false;
        }

        // Compare the arrival times.
        const uint32_t timeDifference =
            (ggaAge >= rmcAge)
                ? (ggaAge - rmcAge)
                : (rmcAge - ggaAge);

        if (timeDifference > GNSS_PAIR_MAX_DIFF_MS)
            return false;

        return true;
    }

    void CompleteNmeaSentence(bool interrupted = false)
    {
        if (nmeaLength == 0U)
            return;

        nmeaBuffer[nmeaLength] = '\0';

        if (interrupted)
        {
            ++nmeaIncompleteCount;
            ++nmeaInvalidCount;

            memcpy(
                nmeaLastIncomplete,
                nmeaBuffer,
                nmeaLength + 1U
            );
        }
        else if (ValidateNmea(nmeaBuffer, nmeaLength))
        {
            ++nmeaValidCount;

            // Accept GNGGA, GPGGA and other talker IDs.
            if (nmeaLength >= 7U &&
                nmeaBuffer[0] == '$' &&
                nmeaBuffer[3] == 'G' &&
                nmeaBuffer[4] == 'G' &&
                nmeaBuffer[5] == 'A' &&
                nmeaBuffer[6] == ',')
            {
                GGAResult parsed{};

                if (ParseGGA(nmeaBuffer, parsed))
                {
                    parsed.receivedMs = millis();

                    latestGGA = parsed;
                    ++ggaParseCount;
                }
                else
                {
                    ++ggaErrorCount;
                }
            }
            // Accept GNRMC, GPRMC and other talker IDs.
            if (nmeaLength >= 7U &&
                nmeaBuffer[0] == '$' &&
                nmeaBuffer[3] == 'R' &&
                nmeaBuffer[4] == 'M' &&
                nmeaBuffer[5] == 'C' &&
                nmeaBuffer[6] == ',')
            {
                RMCResult parsed{};

                if (ParseRMC(nmeaBuffer, parsed))
                {
                    parsed.receivedMs = millis();

                    latestRMC = parsed;
                    ++rmcParseCount;
                }
                else
                {
                    ++rmcErrorCount;
                }
            }
        }
        else
        {
            ++nmeaChecksumErrorCount;
            ++nmeaInvalidCount;

            memcpy(
                nmeaLastChecksumError,
                nmeaBuffer,
                nmeaLength + 1U
            );
        }

        nmeaLength = 0U;
    }

    void GnssRawCallback(char* data, uint8_t len)
    {
        if (data == nullptr || len == 0U)
            return;

        for (uint16_t i = 0U; i < len; ++i)
        {
            const char c = data[i];

            if (c == '$')
            {
                // A new sentence can arrive without CR/LF.
                // Accept the previous sentence if checksum is valid.
                if (nmeaLength > 0U)
                {
                    nmeaBuffer[nmeaLength] = '\0';

                    if (ValidateNmea(nmeaBuffer, nmeaLength))
                    {
                        CompleteNmeaSentence(false);
                    }
                    else
                    {
                        CompleteNmeaSentence(true);
                    }
                }

                nmeaLength = 0U;
                nmeaBuffer[nmeaLength++] = c;
                continue;
            }

            if (c == '\r' || c == '\n')
            {
                CompleteNmeaSentence();
                continue;
            }

            // Ignore data before the start marker.
            if (nmeaLength == 0U)
                continue;

            if (nmeaLength >= NMEA_BUFFER_SIZE - 1U)
            {
                ++nmeaOverflowCount;
                nmeaLength = 0U;
                continue;
            }

            nmeaBuffer[nmeaLength++] = c;
        }
    }
}


namespace SVEMS::Service
{
    //---------------------------------------------------------
    // Static Members
    //---------------------------------------------------------

    bool GNSSService::Ready = false;

    GNSSData GNSSService::Data{};

    // Step 9-3B: Last validated GNSS position
    GNSSData GNSSService::lastValidData{};
    bool GNSSService::hasLastValidData = false;

    //---------------------------------------------------------
    // Begin
    //---------------------------------------------------------

    bool GNSSService::Begin()
    {
        if (Ready)
        {
            return true;
        }

        Serial.println("[GNSS] Initializing...");

        // Create the GNSS data mutex before starting the worker.
        if (gnssDataMutex == nullptr)
        {
            gnssDataMutex = xSemaphoreCreateMutex();

            if (gnssDataMutex == nullptr)
            {
                Serial.println("[GNSS] Mutex Create Failed");
                return false;
            }
        }

        Serial.println("[GNSS] Mutex Ready");

        GNSSSerial.setRxBufferSize(8192);
        
        if (!gnss.begin())
        {
            Serial.println("[GNSS] Device NOT detected!");

            return false;
        }

        gnss.enablePower();
        gnss.setGnss(eGPS_BeiDou_GLONASS);

        gnss.setCallback(GnssRawCallback);

        Serial.println("[GNSS] Device detected!");
        Serial.println("[GNSS] Power enabled");

        TaskHandle_t taskHandle = nullptr;

        const BaseType_t result =
            xTaskCreatePinnedToCore(
                WorkerTask,
                "GNSSWorker",
                6144,
                nullptr,
                1,
                &taskHandle,
                0
            );

        if (result != pdPASS)
        {
            Serial.println("[GNSS] Task Create Failed");
            return false;
        }

        Ready = true;

        Serial.println("[GNSS] Task Created");

        return true;
    }

    //---------------------------------------------------------
    // GetData
    //---------------------------------------------------------

    GNSSData GNSSService::GetData()
    {
        GNSSData snapshot{};

        if (gnssDataMutex == nullptr)
            return snapshot;

        if (xSemaphoreTake(
                gnssDataMutex,
                pdMS_TO_TICKS(20)) == pdTRUE)
        {
            snapshot = Data;
            xSemaphoreGive(gnssDataMutex);
        }
        else
        {
            return snapshot;
        }

        // Step 9-3B-2B: Validate published data freshness.
        constexpr uint32_t DATA_MAX_AGE_MS = 15000U;
        constexpr uint32_t STALE_MAX_AGE_MS = 30000U;

        if (snapshot.state != GNSSDataState::Invalid)
        {
            const uint32_t dataAge =
                millis() - snapshot.lastUpdateMs;

            if (dataAge > STALE_MAX_AGE_MS)
            {
                // Last validated position has expired.
                snapshot = GNSSData{};
            }
            else if (snapshot.state == GNSSDataState::Valid &&
                    dataAge > DATA_MAX_AGE_MS)
            {
                // Worker has not refreshed the published data.
                // Preserve the old position, but mark it as stale.
                snapshot.valid = false;
                snapshot.state = GNSSDataState::Stale;
            }
        }

        return snapshot;
    }

    //---------------------------------------------------------
    // IsReady
    //---------------------------------------------------------

    bool GNSSService::IsReady()
    {
        return Ready;
    }

    //---------------------------------------------------------
    // HasValidData
    //---------------------------------------------------------

    bool GNSSService::HasValidData()
    {
        return GetData().valid;
    }

    //---------------------------------------------------------
    // WorkerTask
    //---------------------------------------------------------

    void GNSSService::WorkerTask(
        void* parameter
    )
    {
        (void)parameter;

        Serial.printf(
            "[GNSS] Worker Started (Core %d)\n",
            xPortGetCoreID()
        );

        for (;;)
        {
            const uint32_t startMs = millis();

            Serial.println("[GNSS] NMEA Request");

            gnss.getAllGnss();

            Serial.printf(
                "[GNSS] GGA Parsed=%lu Error=%lu\n",
                static_cast<unsigned long>(ggaParseCount),
                static_cast<unsigned long>(ggaErrorCount)
            );

            if (latestGGA.parsed)
            {
                Serial.printf(
                    "[GNSS] GGA Lat=%.7f Lon=%.7f\n",
                    latestGGA.latitude,
                    latestGGA.longitude
                );

                Serial.printf(
                    "[GNSS] GGA Fix=%u SAT=%u HDOP=%.2f ALT=%.1fm\n",
                    static_cast<unsigned>(latestGGA.fixQuality),
                    static_cast<unsigned>(latestGGA.satellites),
                    latestGGA.hdop,
                    latestGGA.altitude
                );
            }

            Serial.printf(
                "[GNSS] RMC Parsed=%lu Error=%lu\n",
                static_cast<unsigned long>(rmcParseCount),
                static_cast<unsigned long>(rmcErrorCount)
            );

            if (latestRMC.parsed)
            {
                Serial.printf(
                    "[GNSS] RMC Valid=%s Speed=%.2f km/h\n",
                    latestRMC.valid ? "YES" : "NO",
                    latestRMC.speedKmh
                );

                Serial.printf(
                    "[GNSS] RMC UTC Date=%s Time=%s\n",
                    latestRMC.utcDate,
                    latestRMC.utcTime
                );

                if (latestRMC.valid)
                {
                    Serial.printf(
                        "[GNSS] RMC Lat=%.7f Lon=%.7f\n",
                        latestRMC.latitude,
                        latestRMC.longitude
                    );
                }
            }

            const uint32_t nowMs = millis();

            const bool fixValid = IsGnssFixValid(nowMs);

            //---------------------------------------------------------
            // GNSSData Snapshot
            //---------------------------------------------------------

            GNSSData snapshot{};

            uint64_t unixTimestamp = 0ULL;

            if (fixValid &&
                ConvertRmcUtcToUnix(latestRMC, unixTimestamp))
            {
                // Position and speed from RMC
                snapshot.latitude = latestRMC.latitude;
                snapshot.longitude = latestRMC.longitude;
                snapshot.speedKmh = latestRMC.speedKmh;

                // Altitude and fix information from GGA
                snapshot.altitude = latestGGA.altitude;
                snapshot.satellites = latestGGA.satellites;
                snapshot.fixQuality = latestGGA.fixQuality;
                snapshot.hdop = latestGGA.hdop;

                // UTC conversion will be added in Step 9-2D-3.
                snapshot.utcTimestamp = unixTimestamp;

                snapshot.lastUpdateMs = nowMs;
                snapshot.valid = true;
            }

            // Publish a complete snapshot under the mutex.
            bool published = false;

            if (gnssDataMutex != nullptr &&
                xSemaphoreTake(
                    gnssDataMutex,
                    pdMS_TO_TICKS(20)) == pdTRUE)
            {
                if (snapshot.valid)
                {
                    snapshot.state = GNSSDataState::Valid;

                    // Preserve the last validated GNSS position.
                    lastValidData = snapshot;
                    hasLastValidData = true;
                }
                else if (hasLastValidData &&
                        (nowMs - lastValidData.lastUpdateMs) <=
                            GNSS_STALE_TIMEOUT_MS)
                {
                    // Preserve the last known position.
                    snapshot = lastValidData;

                    snapshot.valid = false;
                    snapshot.state = GNSSDataState::Stale;
                }
                else
                {
                    // No usable GNSS position.
                    snapshot = GNSSData{};
                    snapshot.state = GNSSDataState::Invalid;
                }

                // Publish GNSS data.
                Data = snapshot;

                xSemaphoreGive(gnssDataMutex);
                published = true;
            }

            if (!published)
            {
                Serial.println("[GNSS] Data Publish Failed");
            }

            if (published)
            {
                GNSSData stateSnapshot{};

                if (gnssDataMutex != nullptr &&
                    xSemaphoreTake(gnssDataMutex, pdMS_TO_TICKS(20)) == pdTRUE)
                {
                    stateSnapshot = Data;
                    xSemaphoreGive(gnssDataMutex);

                    const char* stateName = "Invalid";

                    switch (stateSnapshot.state)
                    {
                        case GNSSDataState::Valid:
                            stateName = "Valid";
                            break;

                        case GNSSDataState::Stale:
                            stateName = "Stale";
                            break;

                        default:
                            break;
                    }

                    const uint32_t dataAge =
                        stateSnapshot.state == GNSSDataState::Invalid
                            ? 0U
                            : (nowMs - stateSnapshot.lastUpdateMs);

                    Serial.printf(
                        "[GNSS STATE] %s Valid=%s Age=%lu ms\n",
                        stateName,
                        stateSnapshot.valid ? "YES" : "NO",
                        static_cast<unsigned long>(dataAge)
                    );
                }
            }

            if (published)
            {
                Serial.printf(
                    "[GNSS] DATA Valid=%s Lat=%.7f Lon=%.7f\n",
                    snapshot.valid ? "YES" : "NO",
                    snapshot.latitude,
                    snapshot.longitude
                );

                Serial.printf(
                    "[GNSS] DATA Speed=%.2f ALT=%.1f SAT=%u\n",
                    snapshot.speedKmh,
                    snapshot.altitude,
                    static_cast<unsigned>(snapshot.satellites)
                );

                Serial.printf(
                    "[GNSS] DATA UTC=%llu\n",
                    static_cast<unsigned long long>(
                        snapshot.utcTimestamp
                    )
                );
            }

            const uint32_t ggaAge =
                latestGGA.parsed
                    ? (nowMs - latestGGA.receivedMs)
                    : 0U;

            const uint32_t rmcAge =
                latestRMC.parsed
                    ? (nowMs - latestRMC.receivedMs)
                    : 0U;

            Serial.printf(
                "[GNSS] FIX Valid=%s GGA_Age=%lu RMC_Age=%lu\n",
                fixValid ? "YES" : "NO",
                static_cast<unsigned long>(ggaAge),
                static_cast<unsigned long>(rmcAge)
            );

            // Print NMEA error diagnostics
            static uint32_t lastChecksumErrorCount = 0U;
            static uint32_t lastIncompleteCount = 0U;

            if (nmeaChecksumErrorCount != lastChecksumErrorCount)
            {
                const char* sentence = nmeaLastChecksumError;
                const size_t length = strlen(sentence);

                uint8_t calculated = 0U;
                size_t starPos = length;
                unsigned starCount = 0U;

                for (size_t i = 1U; i < length; ++i)
                {
                    if (sentence[i] == '*')
                    {
                        ++starCount;

                        if (starPos == length)
                            starPos = i;
                    }

                    if (starPos == length)
                    {
                        calculated ^=
                            static_cast<uint8_t>(sentence[i]);
                    }
                }

                int received = -1;

                if (starPos + 3U == length)
                {
                    const int high = HexValue(sentence[starPos + 1U]);
                    const int low  = HexValue(sentence[starPos + 2U]);

                    if (high >= 0 && low >= 0)
                        received = (high << 4) | low;
                }

                Serial.printf(
                    "[GNSS CRC] Len=%u Calc=%02X Rx=%02X Stars=%u Raw=%s\n",
                    static_cast<unsigned>(length),
                    static_cast<unsigned>(calculated),
                    static_cast<unsigned>(
                        received >= 0 ? received : 0xFFFF
                    ),
                    starCount,
                    sentence
                );

                lastChecksumErrorCount = nmeaChecksumErrorCount;
            }

            if (nmeaIncompleteCount != lastIncompleteCount)
            {
                Serial.printf(
                    "[GNSS] INCOMPLETE: %s\n",
                    nmeaLastIncomplete
                );

                lastIncompleteCount = nmeaIncompleteCount;
            }

            Serial.printf(
                "[GNSS] NMEA Valid=%lu Invalid=%lu ChecksumError=%lu Incomplete=%lu Overflow=%lu\n",
                static_cast<unsigned long>(nmeaValidCount),
                static_cast<unsigned long>(nmeaInvalidCount),
                static_cast<unsigned long>(nmeaChecksumErrorCount),
                static_cast<unsigned long>(nmeaIncompleteCount),
                static_cast<unsigned long>(nmeaOverflowCount)
            );

            Serial.printf(
                "[GNSS] NMEA Processing=%lu ms\n",
                static_cast<unsigned long>(
                    millis() - startMs
                )
            );

            vTaskDelay(
                pdMS_TO_TICKS(5000)
            );
        }
    }
}
