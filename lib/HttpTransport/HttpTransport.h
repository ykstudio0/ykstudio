//-------------------------------------------------------------
// File : HttpTransport.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : HTTP Transport Layer
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

namespace SVEMS::Transport
{
    class HttpTransport
    {
    public:

        static bool Begin();

        static bool IsReady();

        // Non-blocking
        // Payload is copied into the queue.
        static bool Send(
            const String& payload);

    public:

        enum class State : uint8_t
        {
            Ready,
            Sending,
            RetryWaiting
        };

        static State GetState();

        static uint32_t GetSuccessCount();
        static uint32_t GetFailureCount();
        static uint32_t GetConsecutiveFailures();

    private:

        static void WorkerTask(
            void* parameter);

        static bool SendHttp(
            const char* payload);

    private:

        static bool Ready;

        static QueueHandle_t Queue;

        static TaskHandle_t WorkerTaskHandle;

        static constexpr size_t
            MAX_PAYLOAD_SIZE = 1024U;

        static constexpr uint8_t
            QUEUE_LENGTH = 2U;

        struct HttpMessage
        {
            char payload[
                MAX_PAYLOAD_SIZE];
        };

    private:

        static State CurrentState;

        static uint32_t SuccessCount;
        static uint32_t FailureCount;
        static uint32_t ConsecutiveFailures;

        static uint32_t LastFailureMs;

        static constexpr uint32_t
            RETRY_INTERVAL_MS = 30000UL;
    };

}