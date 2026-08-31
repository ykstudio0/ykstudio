//-------------------------------------------------------------
// File : HttpTransport.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "HttpTransport.h"
#include "Logger.h"
#include "WiFiService.h"
#include "Config.h"
#include "Secrets.h"
#include "Scheduler.h"

namespace
{
    // struct HttpMessage
    // {
    //     char payload[
    //         SVEMS::Transport::HttpTransport::
    //             MAX_PAYLOAD_SIZE];
    // };
}

namespace SVEMS::Transport
{
    HttpTransport::State
        HttpTransport::CurrentState =
            HttpTransport::State::Ready;

    uint32_t
        HttpTransport::SuccessCount = 0U;

    uint32_t
        HttpTransport::FailureCount = 0U;

    uint32_t
        HttpTransport::ConsecutiveFailures = 0U;

    uint32_t
        HttpTransport::LastFailureMs = 0U;

    uint32_t HttpTransport::SendingSinceMs = 0U;
    
    int HttpTransport::LastErrorCode = 0;

    uint32_t HttpTransport::MaxConsecutiveFailures = 0U;

    bool HttpTransport::Ready = false;

    QueueHandle_t HttpTransport::Queue = nullptr;

    TaskHandle_t HttpTransport::WorkerTaskHandle = nullptr;

    portMUX_TYPE HttpTransport::StateMux =
        portMUX_INITIALIZER_UNLOCKED;

    TaskHandle_t
        HttpTransport::CommandWorkerTaskHandle =
            nullptr;

    bool HttpTransport::Begin()
    {
        //---------------------------------------------------------
        // Already initialized
        //---------------------------------------------------------

        if (Ready)
        {
            return true;
        }

        //---------------------------------------------------------
        // Initialize state
        //---------------------------------------------------------

        portENTER_CRITICAL(
            &StateMux);

        CurrentState =
            State::Ready;

        SuccessCount = 0U;
        FailureCount = 0U;
        ConsecutiveFailures = 0U;
        LastFailureMs = 0U;

        portEXIT_CRITICAL(
            &StateMux);

        //---------------------------------------------------------
        // Create Queue
        //---------------------------------------------------------

        Queue =
            xQueueCreate(
                QUEUE_LENGTH,
                sizeof(HttpMessage));

        if (Queue == nullptr)
        {
            Logger::Error(
                "HTTP",
                "Queue Create Failed");

            return false;
        }

        //---------------------------------------------------------
        // Create HTTP Worker
        //---------------------------------------------------------

        const BaseType_t result =
            xTaskCreatePinnedToCore(
                WorkerTask,
                "HttpWorker",
                10240,
                nullptr,
                1,
                &WorkerTaskHandle,
                0);

        if (result != pdPASS)
        {
            Logger::Error(
                "HTTP",
                "Task Create Failed");

            vQueueDelete(
                Queue);

            Queue = nullptr;

            return false;
        }

        //---------------------------------------------------------
        // Create Command Worker
        //---------------------------------------------------------

        const BaseType_t commandResult =
            xTaskCreatePinnedToCore(
                CommandWorkerTask,
                "CommandWorker",
                8192,
                nullptr,
                1,
                &CommandWorkerTaskHandle,
                0);

        if (commandResult != pdPASS)
        {
            Logger::Error(
                "HTTP CMD",
                "Task Create Failed");

            vTaskDelete(
                WorkerTaskHandle);

            WorkerTaskHandle =
                nullptr;

            vQueueDelete(
                Queue);

            Queue =
                nullptr;

            return false;
        }

        Ready = true;

        Logger::Info(
            "HTTP",
            "Ready");

        return true;
    }

    bool HttpTransport::IsReady()
    {
        return Ready;
    }

    bool HttpTransport::IsOnline()
    {
        portENTER_CRITICAL(
            &StateMux);

        const bool online =
            CurrentState !=
                State::RetryWaiting;

        portEXIT_CRITICAL(
            &StateMux);

        return online;
    }

    bool canSend = false;
    bool sendingTimedOut = false;

    bool HttpTransport::Send(
        const String& payload)
    {
        if (!Ready ||
            Queue == nullptr)
        {
            return false;
        }

        if (payload.length() == 0U)
        {
            return false;
        }

        if (payload.length() >=
            MAX_PAYLOAD_SIZE)
        {
            Logger::Warning(
                "HTTP",
                "Payload Too Large");

            return false;
        }

        const uint32_t now =
            millis();

        //---------------------------------------------------------
        // HTTP request already in progress.
        // Skip this telemetry snapshot.
        //---------------------------------------------------------

        bool canSend = false;

        portENTER_CRITICAL(
            &StateMux);

        if (CurrentState ==
            State::Ready)
        {
            CurrentState =
                State::Sending;

            SendingSinceMs =
                now;

            canSend = true;
        }
        else if (CurrentState ==
                State::RetryWaiting)
        {
            if (now - LastFailureMs >=
                RETRY_INTERVAL_MS)
            {
                CurrentState =
                    State::Sending;
                
                SendingSinceMs =
                    now;

                canSend = true;
            }
        }
        else if (CurrentState ==
                State::Sending)
        {
            if (now - SendingSinceMs >=
                SENDING_TIMEOUT_MS)
            {
                CurrentState =
                    State::RetryWaiting;

                LastFailureMs =
                    now;

                ++FailureCount;
                ++ConsecutiveFailures;

                if (ConsecutiveFailures >
                    MaxConsecutiveFailures)
                {
                    MaxConsecutiveFailures =
                        ConsecutiveFailures;
                }

                sendingTimedOut = true;
            }
        }

        portEXIT_CRITICAL(
            &StateMux);

        if (sendingTimedOut)
        {
            Logger::Warning(
                "HTTP",
                "Sending Timeout");
        }

            if (!canSend)
        {
            return true;
        }

        //---------------------------------------------------------
        // Prepare payload.
        //---------------------------------------------------------

        HttpMessage message{};

        payload.toCharArray(
            message.payload,
            MAX_PAYLOAD_SIZE);

        //---------------------------------------------------------
        // Mark as in-flight BEFORE queueing.
        //
        // This prevents another telemetry snapshot from being
        // queued while the worker is waiting for HTTP timeout.
        //---------------------------------------------------------

        const BaseType_t result =
            xQueueSend(
                Queue,
                &message,
                0);

        if (result != pdPASS)
        {
            portENTER_CRITICAL(
                &StateMux);

            CurrentState =
                State::Ready;

            portEXIT_CRITICAL(
                &StateMux);

            Logger::Warning(
                "HTTP",
                "Queue Full");

            return false;
        }

        return true;
    }

    void HttpTransport::WorkerTask(
        void* parameter)
    {
        (void)parameter;

        HttpMessage message{};

        for (;;)
        {
            if (xQueueReceive(
                    Queue,
                    &message,
                    portMAX_DELAY) == pdPASS)
            {
                SendHttp(
                    message.payload);
            }
        }
    }

    void HttpTransport::CommandWorkerTask(
        void* parameter
    )
    {
        (void)parameter;

        Logger::Info(
            "HTTP CMD",
            "Worker Started"
        );

        for (;;)
        {
            String response;

            if (
                FetchReverseChargeCommand(
                    response
                )
            )
            {
                if (
                    response.length() > 0
                )
                {
                    JsonDocument doc;

                    const DeserializationError error =
                        deserializeJson(
                            doc,
                            response
                        );

                    if (!error)
                    {
                        JsonVariant command =
                            doc["command"];

                        if (
                            !command.isNull()
                        )
                        {
                            Scheduler::
                                SetReverseChargePendingCommand(
                                    response
                                );
                        }
                    }
                }
            }

            vTaskDelay(
                pdMS_TO_TICKS(
                    COMMAND_POLL_INTERVAL_MS
                )
            );
        }
    }

    bool HttpTransport::SendHttp(
        const char* payload)
    {
        if (payload == nullptr ||
            payload[0] == '\0')
        {
            return false;
        }

        //---------------------------------------------------------
        // WiFi Check
        //---------------------------------------------------------

        if (!SVEMS::Service::WiFiService::IsConnected())
        {
            const uint32_t now =
                millis();

            portENTER_CRITICAL(
                &StateMux);

            ++FailureCount;
            ++ConsecutiveFailures;

            if (ConsecutiveFailures >
                MaxConsecutiveFailures)
            {
                MaxConsecutiveFailures =
                    ConsecutiveFailures;
            }

            LastFailureMs =
                now;

            CurrentState =
                State::RetryWaiting;

            portEXIT_CRITICAL(
                &StateMux);

            Logger::Warning(
                "HTTP",
                "WiFi Offline");

            return false;
        }

        //---------------------------------------------------------
        // HTTP Begin
        //---------------------------------------------------------

        HTTPClient http;

        http.setConnectTimeout(
            3000);

        http.setTimeout(
            3000);

        if (!http.begin(
                SVEMS::Config::TELEMETRY_URL))
        {
            const uint32_t now =
                millis();

            portENTER_CRITICAL(
                &StateMux);

            ++FailureCount;
            ++ConsecutiveFailures;

            if (ConsecutiveFailures >
                MaxConsecutiveFailures)
            {
                MaxConsecutiveFailures =
                    ConsecutiveFailures;
            }

            LastFailureMs =
                now;

            CurrentState =
                State::RetryWaiting;
            
            portEXIT_CRITICAL(
                &StateMux);

            Logger::Warning(
                "HTTP",
                "Begin Failed");

            return false;
        }

        http.addHeader(
            "Content-Type",
            "application/json");
        
        http.addHeader(
            "X-SVEMS-API-Key",
            SVEMS::Config::SVEMS_API_KEY);

        //---------------------------------------------------------
        // HTTP POST
        //
        // Blocking is allowed here because this function runs
        // only inside HttpWorker on Core 0.
        //---------------------------------------------------------

        const int httpCode =
            http.POST(
                String(payload));

        //---------------------------------------------------------
        // Success
        //---------------------------------------------------------

        if (httpCode >= 200 &&
            httpCode < 300)
        {
            portENTER_CRITICAL(
                &StateMux); 

            ++SuccessCount;

            ConsecutiveFailures = 0U;

            CurrentState =
                State::Ready;

            portEXIT_CRITICAL(
                &StateMux);

            char message[48];

            snprintf(
                message,
                sizeof(message),
                "Response = %d",
                httpCode);

            Logger::Info(
                "HTTP",
                message);

            http.end();

            return true;
        }

        //---------------------------------------------------------
        // Failure
        //---------------------------------------------------------
        const uint32_t now =
            millis();

        portENTER_CRITICAL(
            &StateMux);

        ++FailureCount;
        ++ConsecutiveFailures;

        if (ConsecutiveFailures >
            MaxConsecutiveFailures)
        {
            MaxConsecutiveFailures =
                ConsecutiveFailures;
        }

        LastErrorCode =
            httpCode;

        LastFailureMs =
            now;

        CurrentState =
            State::RetryWaiting;

        portEXIT_CRITICAL(
            &StateMux);
            
        char message[48];

        snprintf(
            message,
            sizeof(message),
            "POST Failed (%d)",
            httpCode);

        Logger::Warning(
            "HTTP",
            message);

        http.end();

        return false;
    }

    HttpTransport::State
    HttpTransport::GetState()
    {
        portENTER_CRITICAL(
            &StateMux);

        const State value =
            CurrentState;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    uint32_t
    HttpTransport::GetSuccessCount()
    {
        portENTER_CRITICAL(
            &StateMux);

        const uint32_t value =
            SuccessCount;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    uint32_t
    HttpTransport::GetFailureCount()
    {
        portENTER_CRITICAL(
            &StateMux);

        const uint32_t value =
            FailureCount;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    uint32_t
    HttpTransport::GetConsecutiveFailures()
    {
        portENTER_CRITICAL(
            &StateMux);

        const uint32_t value =
            ConsecutiveFailures;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    uint32_t
    HttpTransport::GetLastFailureMs()
    {
        portENTER_CRITICAL(
            &StateMux);

        const uint32_t value =
            LastFailureMs;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    const char*
    HttpTransport::StateToString(
        State state)
    {
        switch (state)
        {
            case State::Ready:
                return "Ready";

            case State::Sending:
                return "Sending";

            case State::RetryWaiting:
                return "Retry";

            default:
                return "Unknown";
        }
    }

    int HttpTransport::GetLastErrorCode()
    {
        portENTER_CRITICAL(
            &StateMux);

        const int value =
            LastErrorCode;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    uint32_t HttpTransport::GetMaxConsecutiveFailures()
    {
        portENTER_CRITICAL(
            &StateMux);

        const uint32_t value =
            MaxConsecutiveFailures;

        portEXIT_CRITICAL(
            &StateMux);

        return value;
    }

    bool HttpTransport::FetchReverseChargeCommand(
        String& response)
    {
        response = "";

        //---------------------------------------------------------
        // WiFi Check
        //---------------------------------------------------------

        if (!SVEMS::Service::WiFiService::IsConnected())
        {
            Logger::Warning(
                "HTTP",
                "WiFi Offline");

            return false;
        }

        //---------------------------------------------------------
        // HTTP Begin
        //---------------------------------------------------------

        HTTPClient http;

        http.setConnectTimeout(
            3000);

        http.setTimeout(
            3000);

        String url =
            SVEMS::Config::REVERSE_CHARGE_COMMAND_URL;

        url += "?deviceId=";
        url += SVEMS_DEVICE_ID;

        if (!http.begin(
                url))
        {
            Logger::Warning(
                "HTTP",
                "Command Begin Failed");

            return false;
        }

        http.addHeader(
            "X-SVEMS-API-Key",
            SVEMS::Config::SVEMS_API_KEY);

        //---------------------------------------------------------
        // HTTP GET
        //---------------------------------------------------------

        const int httpCode =
            http.GET();

        if (httpCode < 200 ||
            httpCode >= 300)
        {
            char message[48];

            snprintf(
                message,
                sizeof(message),
                "Command GET Failed (%d)",
                httpCode);

            Logger::Warning(
                "HTTP",
                message);

            const String errorBody =
                http.getString();

            Logger::Warning(
                "HTTP CMD",
                errorBody.c_str()
            );

            http.end();

            return false;
        }

        //---------------------------------------------------------
        // Response
        //---------------------------------------------------------

        response =
            http.getString();

        http.end();

        return true;
    }
}