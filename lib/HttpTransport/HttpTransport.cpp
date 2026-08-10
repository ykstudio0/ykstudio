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

    bool HttpTransport::Ready = false;

    QueueHandle_t HttpTransport::Queue = nullptr;

    TaskHandle_t HttpTransport::WorkerTaskHandle = nullptr;

    bool HttpTransport::Begin()
    {
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

        HttpMessage message{};

        payload.toCharArray(
            message.payload,
            MAX_PAYLOAD_SIZE);

        //---------------------------------------------------------
        // Do not block Scheduler.
        //---------------------------------------------------------

        const BaseType_t result =
            xQueueSend(
                Queue,
                &message,
                0);

        if (result != pdPASS)
        {
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

        // char coreMessage[32];

        // snprintf(
        //     coreMessage,
        //     sizeof(coreMessage),
        //     "Worker Core = %d",
        //     xPortGetCoreID());

        // Logger::Info(
        //     "HTTP",
        //     coreMessage);

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

        if (!SVEMS::Service::WiFiService::IsConnected())
        {
            Logger::Warning(
                "HTTP",
                "WiFi Offline");

            return false;
        }

        HTTPClient http;

        if (!http.begin(
                TELEMETRY_URL))
        {
            Logger::Warning(
                "HTTP",
                "Begin Failed");

            return false;
        }

        http.addHeader(
            "Content-Type",
            "application/json");

        const int httpCode =
            http.POST(
                String(payload));

        if (httpCode > 0)
        {
            char message[48];

            snprintf(
                message,
                sizeof(message),
                "Response = %d",
                httpCode);

            Logger::Info(
                "HTTP",
                message);
        }
        else
        {
            Logger::Warning(
                "HTTP",
                "POST Failed");
        }

        http.end();

        return
            httpCode >= 200 &&
            httpCode < 300;
    }
}