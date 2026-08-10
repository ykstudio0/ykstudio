//-------------------------------------------------------------
// File : HttpTransport.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------

#include <HTTPClient.h>

#include "HttpTransport.h"
#include "Logger.h"
#include "WiFiService.h"
// #include <HTTPClient.h>

namespace
{
    constexpr const char* TELEMETRY_URL =
        "http://192.168.0.10:8080/telemetry";

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
    
    bool HttpTransport::Ready = false;

    QueueHandle_t HttpTransport::Queue = nullptr;

    TaskHandle_t HttpTransport::WorkerTaskHandle = nullptr;

    bool HttpTransport::Begin()
    {
        CurrentState =
            State::Ready;

        SuccessCount = 0U;
        FailureCount = 0U;
        ConsecutiveFailures = 0U;
        LastFailureMs = 0U;

        if (Ready)
        {
            return true;
        }

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

        const BaseType_t result =
            xTaskCreatePinnedToCore(
                WorkerTask,
                "HttpWorker",
                6144,
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

        if (CurrentState ==
            State::Sending)
        {
            return true;
        }

        //---------------------------------------------------------
        // Retry backoff.
        //---------------------------------------------------------

        if (CurrentState ==
            State::RetryWaiting)
        {
            if (now - LastFailureMs <
                RETRY_INTERVAL_MS)
            {
                return true;
            }

            CurrentState =
                State::Ready;
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

        CurrentState =
            State::Sending;

        const BaseType_t result =
            xQueueSend(
                Queue,
                &message,
                0);

        if (result != pdPASS)
        {
            CurrentState =
                State::Ready;

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
            ++FailureCount;
            ++ConsecutiveFailures;

            LastFailureMs =
                millis();

            CurrentState =
                State::RetryWaiting;

            Logger::Warning(
                "HTTP",
                "WiFi Offline");

            return false;
        }

        //---------------------------------------------------------
        // HTTP Begin
        //---------------------------------------------------------

        HTTPClient http;

        if (!http.begin(
                TELEMETRY_URL))
        {
            ++FailureCount;
            ++ConsecutiveFailures;

            LastFailureMs =
                millis();

            CurrentState =
                State::RetryWaiting;

            Logger::Warning(
                "HTTP",
                "Begin Failed");

            return false;
        }

        http.addHeader(
            "Content-Type",
            "application/json");

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
            ++SuccessCount;

            ConsecutiveFailures = 0U;

            CurrentState =
                State::Ready;

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

        ++FailureCount;
        ++ConsecutiveFailures;

        LastFailureMs =
            millis();

        CurrentState =
            State::RetryWaiting;

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
}