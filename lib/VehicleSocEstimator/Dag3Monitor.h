//-------------------------------------------------------------
// File : Dag3Monitor.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : DAG3 diagnostic request activity monitor
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

enum class Dag3State
{
    Unknown,
    Active,
    Inactive
};

class Dag3Monitor
{
public:
    void Begin();

    void OnRequestReceived(uint32_t nowMillis);

    void Update(uint32_t nowMillis);

    Dag3State GetState() const;

    bool OnCanFrame(
        uint32_t canId,
        const uint8_t* data,
        uint8_t length,
        uint32_t nowMillis);

private:
    static constexpr uint32_t REQUEST_TIMEOUT_MS = 10000UL;

    Dag3State m_state = Dag3State::Unknown;

    uint32_t m_lastRequestMillis = 0;
    bool m_hasReceivedRequest = false;
};