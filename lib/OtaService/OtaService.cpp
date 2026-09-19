//-------------------------------------------------------------
// File : OtaService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-19
// Project : SVEMS
// Version : 0.8.3
// Description : OtaService
//-------------------------------------------------------------

#include <Update.h>
#include <mbedtls/sha256.h>
#include <esp_ota_ops.h>

#include "OtaService.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Logger.h"
#include "Version.h"

namespace SVEMS::Service
{
    static constexpr const char* OTA_VERSION_URL =
        "http://192.168.0.15:8080/firmware/main-test/latest.json";

    static constexpr const char* OTA_FIRMWARE_BASE_URL =
        "http://192.168.0.15:8080/firmware/main-test/";

    static int CompareVersion(
        const char* currentVersion,
        const char* latestVersion
    )
    {
        int currentMajor = 0;
        int currentMinor = 0;
        int currentPatch = 0;

        int latestMajor = 0;
        int latestMinor = 0;
        int latestPatch = 0;

        sscanf(
            currentVersion,
            "%d.%d.%d",
            &currentMajor,
            &currentMinor,
            &currentPatch
        );

        sscanf(
            latestVersion,
            "%d.%d.%d",
            &latestMajor,
            &latestMinor,
            &latestPatch
        );

        if (latestMajor != currentMajor)
        {
            return
                (latestMajor > currentMajor)
                ? 1
                : -1;
        }

        if (latestMinor != currentMinor)
        {
            return
                (latestMinor > currentMinor)
                ? 1
                : -1;
        }

        if (latestPatch != currentPatch)
        {
            return
                (latestPatch > currentPatch)
                ? 1
                : -1;
        }

        return 0;
    }

    bool OtaService::CheckForUpdate()
    {
        HTTPClient http;

        if (!http.begin(
                OTA_VERSION_URL))
        {
            Logger::Warning(
                "OTA",
                "HTTP Begin Failed"
            );

            return false;
        }

        const int httpCode =
            http.GET();

        if (httpCode != 200)
        {
            Logger::Warning(
                "OTA",
                "Version Check Failed"
            );

            http.end();

            return false;
        }

        const String payload =
            http.getString();

        http.end();

        JsonDocument doc;

        const DeserializationError error =
            deserializeJson(
                doc,
                payload
            );

        if (error)
        {
            Logger::Warning(
                "OTA",
                "JSON Parse Failed"
            );

            return false;
        }

        const char* latestVersion =
            doc["version"] |
            "";

        const char* firmwareFile =
            doc["file"] |
            "";

        const char* firmwareSha256 =
            doc["sha256"] |
            "";

        char message[160];

        snprintf(
            message,
            sizeof(message),
            "Current=%s Latest=%s File=%s",
            SVEMS_VERSION_STRING,
            latestVersion,
            firmwareFile
        );

        Logger::Info(
            "OTA",
            message
        );

        if (strlen(firmwareSha256) == 64U)
        {
            Logger::Info(
                "OTA",
                "SHA256 Ready"
            );
        }
        else
        {
            Logger::Warning(
                "OTA",
                "Invalid SHA256"
            );
        }

        const int versionCompare =
            CompareVersion(
                SVEMS_VERSION_STRING,
                latestVersion
            );

        if (versionCompare == 0)
        {
            Logger::Info(
                "OTA",
                "Firmware Up To Date"
            );

            return false;
        }

        if (versionCompare < 0)
        {
            Logger::Warning(
                "OTA",
                "Server Firmware Is Older"
            );

            return false;
        }

        if (versionCompare > 0)
        {
            Logger::Info(
                "OTA",
                "New Firmware Available"
            );

            const bool updateResult =
                PerformUpdate(
                    firmwareFile,
                    firmwareSha256
                );

            if (updateResult)
            {
                Logger::Info(
                    "OTA",
                    "Update Completed"
                );

                delay(
                    1000
                );

                ESP.restart();
            }

            return updateResult;
        }

        return true;
    }

    bool OtaService::PerformUpdate(
        const char* firmwareFile,
        const char* expectedSha256
    )
    {
        if (
            firmwareFile == nullptr ||
            firmwareFile[0] == '\0'
        )
        {
            Logger::Warning(
                "OTA",
                "Firmware File Missing"
            );

            return false;
        }

        if (
            expectedSha256 == nullptr ||
            strlen(expectedSha256) != 64U
        )
        {
            Logger::Warning(
                "OTA",
                "Invalid SHA256"
            );

            return false;
        }

        //---------------------------------------------------------
        // Build Firmware URL
        //---------------------------------------------------------

        String firmwareUrl =
            OTA_FIRMWARE_BASE_URL;

        firmwareUrl +=
            firmwareFile;

        Logger::Info(
            "OTA",
            "Download Starting"
        );

        //---------------------------------------------------------
        // HTTP GET
        //---------------------------------------------------------

        HTTPClient http;

        if (!http.begin(
                firmwareUrl))
        {
            Logger::Warning(
                "OTA",
                "HTTP Begin Failed"
            );

            return false;
        }

        const int httpCode =
            http.GET();

        if (httpCode != HTTP_CODE_OK)
        {
            Logger::Warning(
                "OTA",
                "Firmware GET Failed"
            );

            http.end();

            return false;
        }

        const int contentLength =
            http.getSize();

        if (contentLength <= 0)
        {
            Logger::Warning(
                "OTA",
                "Invalid Content Length"
            );

            http.end();

            return false;
        }

        char message[64];

        snprintf(
            message,
            sizeof(message),
            "Size=%d bytes",
            contentLength
        );

        Logger::Info(
            "OTA",
            message
        );

        //---------------------------------------------------------
        // Open OTA Partition
        //---------------------------------------------------------

        if (!Update.begin(
                static_cast<size_t>(
                    contentLength
                )))
        {
            Logger::Warning(
                "OTA",
                "Update Begin Failed"
            );

            http.end();

            return false;
        }

        //---------------------------------------------------------
        // SHA256 Context
        //---------------------------------------------------------

        mbedtls_sha256_context sha256Context;

        mbedtls_sha256_init(
            &sha256Context
        );

        if (
            mbedtls_sha256_starts_ret(
                &sha256Context,
                0
            ) != 0
        )
        {
            Logger::Warning(
                "OTA",
                "SHA256 Init Failed"
            );

            mbedtls_sha256_free(
                &sha256Context
            );

            Update.abort();

            http.end();

            return false;
        }

        //---------------------------------------------------------
        // Download + Write
        //---------------------------------------------------------

        WiFiClient* stream =
            http.getStreamPtr();

        uint8_t buffer[1024];

        size_t totalWritten = 0U;

        uint32_t lastDataMs =
            millis();

        bool failed =
            false;

        while (
            totalWritten <
            static_cast<size_t>(
                contentLength
            )
        )
        {
            const size_t available =
                stream->available();

            if (available > 0U)
            {
                const size_t remaining =
                    static_cast<size_t>(
                        contentLength
                    ) -
                    totalWritten;

                const size_t readSize =
                    min(
                        available,
                        min(
                            sizeof(buffer),
                            remaining
                        )
                    );

                const int bytesRead =
                    stream->readBytes(
                        buffer,
                        readSize
                    );

                if (bytesRead <= 0)
                {
                    failed = true;
                    break;
                }

                if (
                    mbedtls_sha256_update_ret(
                        &sha256Context,
                        buffer,
                        bytesRead
                    ) != 0
                )
                {
                    failed = true;
                    break;
                }

                const size_t written =
                    Update.write(
                        buffer,
                        bytesRead
                    );

                if (
                    written !=
                    static_cast<size_t>(
                        bytesRead
                    )
                )
                {
                    failed = true;
                    break;
                }

                totalWritten +=
                    written;

                lastDataMs =
                    millis();
            }
            else
            {
                if (
                    millis() -
                    lastDataMs >
                    10000U
                )
                {
                    Logger::Warning(
                        "OTA",
                        "Download Timeout"
                    );

                    failed = true;
                    break;
                }

                delay(
                    1
                );
            }
        }

        //---------------------------------------------------------
        // Finish SHA256
        //---------------------------------------------------------

        uint8_t calculatedHash[32];

        if (
            !failed &&
            mbedtls_sha256_finish_ret(
                &sha256Context,
                calculatedHash
            ) != 0
        )
        {
            failed = true;
        }

        mbedtls_sha256_free(
            &sha256Context
        );

        if (failed)
        {
            Logger::Warning(
                "OTA",
                "Download/Write Failed"
            );

            Update.abort();

            http.end();

            return false;
        }

        http.end();

        //---------------------------------------------------------
        // SHA256 To Hex
        //---------------------------------------------------------

        char calculatedSha256[65];

        for (
            int i = 0;
            i < 32;
            ++i
        )
        {
            snprintf(
                &calculatedSha256[i * 2],
                3,
                "%02x",
                calculatedHash[i]
            );
        }

        calculatedSha256[64] =
            '\0';

        //---------------------------------------------------------
        // SHA256 Verify
        //---------------------------------------------------------

        if (
            strcasecmp(
                calculatedSha256,
                expectedSha256
            ) != 0
        )
        {
            Logger::Warning(
                "OTA",
                "SHA256 Mismatch"
            );

            Update.abort();

            return false;
        }

        Logger::Info(
            "OTA",
            "SHA256 Verified"
        );

        //---------------------------------------------------------
        // Commit OTA Image
        //---------------------------------------------------------

        if (!Update.end())
        {
            Logger::Warning(
                "OTA",
                "Update End Failed"
            );

            return false;
        }

        if (!Update.isFinished())
        {
            Logger::Warning(
                "OTA",
                "Update Incomplete"
            );

            return false;
        }

        Logger::Info(
            "OTA",
            "Firmware Update Ready"
        );

        return true;
    }

    bool OtaService::ConfirmRunningFirmware()
    {
        const esp_err_t result =
            esp_ota_mark_app_valid_cancel_rollback();

        if (result == ESP_OK)
        {
            Logger::Info(
                "OTA",
                "Firmware Confirmed"
            );

            return true;
        }

        Logger::Warning(
            "OTA",
            "Firmware Confirm Failed"
        );

        return false;
    }

    bool OtaService::IsPendingVerification()
    {
        const esp_partition_t* runningPartition =
            esp_ota_get_running_partition();

        if (runningPartition == nullptr)
        {
            return false;
        }

        esp_ota_img_states_t state;

        const esp_err_t result =
            esp_ota_get_state_partition(
                runningPartition,
                &state
            );

        if (result != ESP_OK)
        {
            return false;
        }

        char message[48];

        snprintf(
            message,
            sizeof(message),
            "Image State=%d",
            static_cast<int>(
                state
            )
        );

        Logger::Info(
            "OTA",
            message
        );

        return
            state ==
            ESP_OTA_IMG_PENDING_VERIFY;
    }
}