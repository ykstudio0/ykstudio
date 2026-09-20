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
#include <WiFiClientSecure.h>
#include <esp_partition.h>
#include "Certificates.h"
#include "OtaService.h"
#include "Secrets.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Logger.h"
#include "Version.h"
#include "StatusLED.h"

namespace SVEMS::Service
{

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

    static bool GetRunningImageSha256(
        char* output,
        size_t outputSize
    )
    {
        //---------------------------------------------------------
        // Validate Output Buffer
        //---------------------------------------------------------

        if (
            output == nullptr ||
            outputSize < 65U
        )
        {
            return false;
        }

        //---------------------------------------------------------
        // Get Running OTA Partition
        //---------------------------------------------------------

        const esp_partition_t* runningPartition =
            esp_ota_get_running_partition();

        if (runningPartition == nullptr)
        {
            return false;
        }

        //---------------------------------------------------------
        // Read Image SHA256
        //---------------------------------------------------------

        uint8_t hash[32];

        const esp_err_t result =
            esp_partition_get_sha256(
                runningPartition,
                hash
            );

        if (result != ESP_OK)
        {
            return false;
        }

        //---------------------------------------------------------
        // Convert To Hex String
        //---------------------------------------------------------

        for (
            int i = 0;
            i < 32;
            ++i
        )
        {
            snprintf(
                &output[i * 2],
                3,
                "%02x",
                hash[i]
            );
        }

        output[64] =
            '\0';

        return true;
    }

    bool OtaService::CheckForUpdate()
    {
        WiFiClientSecure secureClient;

        secureClient.setCACert(
            SVEMS::Config::OTA_ROOT_CA
        );

        HTTPClient http;

        if (
            !http.begin(
                secureClient,
                SVEMS::Config::OTA_VERSION_URL
            )
        )
        {
            Logger::Warning(
                "OTA",
                "HTTPS Begin Failed"
            );

            StatusLED::SetOtaState(
                OtaLedState::Idle
            );

            return false;
        }

        http.setAuthorization(
            SVEMS::Config::OTA_USERNAME,
            SVEMS::Config::OTA_PASSWORD
        );

        const int httpCode =
            http.GET();

        if (httpCode != HTTP_CODE_OK)
        {
            char message[128];

            snprintf(
                message,
                sizeof(message),
                "Version Check Failed Code=%d Error=%s",
                httpCode,
                HTTPClient::errorToString(
                    httpCode
                ).c_str()
            );

            Logger::Warning(
                "OTA",
                message
            );

            http.end();

            StatusLED::SetOtaState(
                OtaLedState::Idle
            );

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

            StatusLED::SetOtaState(
                OtaLedState::Idle
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

        const char* imageSha256 =
            doc["imageSha256"] |
            "";

        char runningImageSha256[65] =
            { 0 };

        const bool runningImageShaReady =
            GetRunningImageSha256(
                runningImageSha256,
                sizeof(runningImageSha256)
            );

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

        //---------------------------------------------------------
        // Older Server Firmware
        //---------------------------------------------------------

        if (versionCompare < 0)
        {
            Logger::Warning(
                "OTA",
                "Server Firmware Is Older"
            );
            
            StatusLED::SetOtaState(
                OtaLedState::Idle
            );
            return false;
        }

        //---------------------------------------------------------
        // Validate Image SHA256
        //---------------------------------------------------------

        if (
            strlen(imageSha256) != 64U ||
            !runningImageShaReady
        )
        {
            Logger::Warning(
                "OTA",
                "Image SHA256 Not Available"
            );

            StatusLED::SetOtaState(
                OtaLedState::Idle
            );

            return false;
        }

        const bool imageShaDifferent =
            strcasecmp(
                runningImageSha256,
                imageSha256
            ) != 0;

        //---------------------------------------------------------
        // Firmware Up To Date
        //---------------------------------------------------------

        if (
            versionCompare == 0 &&
            !imageShaDifferent
        )
        {
            Logger::Info(
                "OTA",
                "Firmware Up To Date"
            );

            StatusLED::SetOtaState(
                OtaLedState::Idle
            );

            return false;
        }

        //---------------------------------------------------------
        // Update Required
        //---------------------------------------------------------

        if (
            versionCompare > 0 ||
            imageShaDifferent
        )
        {
            if (
                versionCompare == 0 &&
                imageShaDifferent
            )
            {
                Logger::Info(
                    "OTA",
                    "Same Version, Different Image"
                );
            }

            Logger::Info(
                "OTA",
                "New Firmware Available"
            );

            StatusLED::SetOtaState(
                OtaLedState::Updating
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

                StatusLED::SetOtaState(
                    OtaLedState::Success
                );

                delay(
                    1000
                );

                ESP.restart();
            }

            StatusLED::SetOtaState(
                OtaLedState::Idle
            );

            return updateResult;
        }

        StatusLED::SetOtaState(
            OtaLedState::Idle
         );

        return false;
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
            SVEMS::Config::OTA_FIRMWARE_BASE_URL;

        firmwareUrl +=
            firmwareFile;

        Logger::Info(
            "OTA",
            "Download Starting"
        );

        //---------------------------------------------------------
        // HTTP GET
        //---------------------------------------------------------

        WiFiClientSecure secureClient;

        secureClient.setCACert(
            SVEMS::Config::OTA_ROOT_CA
        );

        HTTPClient http;

        if (
            !http.begin(
                secureClient,
                firmwareUrl
            )
        )
        {
            Logger::Warning(
                "OTA",
                "HTTPS Begin Failed"
            );

            return false;
        }

        http.setAuthorization(
            SVEMS::Config::OTA_USERNAME,
            SVEMS::Config::OTA_PASSWORD
        );

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
            StatusLED::Task();
            
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

    static bool GetRunningFirmwareSha256(
        char* output,
        size_t outputSize
    )
    {
        if (
            output == nullptr ||
            outputSize < 65U
        )
        {
            return false;
        }

        const esp_partition_t* runningPartition =
            esp_ota_get_running_partition();

        if (runningPartition == nullptr)
        {
            return false;
        }

        uint8_t hash[32];

        if (
            esp_partition_get_sha256(
                runningPartition,
                hash
            ) != ESP_OK
        )
        {
            return false;
        }

        for (
            int i = 0;
            i < 32;
            ++i
        )
        {
            snprintf(
                &output[i * 2],
                3,
                "%02x",
                hash[i]
            );
        }

        output[64] =
            '\0';

        return true;
    }
}