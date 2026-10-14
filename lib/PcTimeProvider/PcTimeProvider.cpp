//-------------------------------------------------------------
// File : PcTimeProvider.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS PcTimeProvider
//-------------------------------------------------------------

#include "PcTimeProvider.h"

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>


//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void PcTimeProvider::Begin()
{
    m_timeValid = false;
    m_inputBuffer.reserve(48);
}


//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void PcTimeProvider::Update()
{
    while (Serial.available() > 0)
    {
        const char ch = (char)Serial.read();

        if (ch == '\n')
        {
            m_inputBuffer.trim();

            ProcessCommand(m_inputBuffer);

            m_inputBuffer = "";
        }
        else if (ch != '\r')
        {
            if (m_inputBuffer.length() < 47)
            {
                m_inputBuffer += ch;
            }
            else
            {
                m_inputBuffer = "";
                Serial.println("[TIME] Input too long");
            }
        }
    }
}


//-------------------------------------------------------------
// Process Command
//-------------------------------------------------------------
void PcTimeProvider::ProcessCommand(
    const String& command)
{
    if (!command.startsWith("TIME "))
        return;

    String value = command.substring(5);
    value.trim();

    if (value.length() == 0)
    {
        Serial.println("[TIME] Invalid input");
        return;
    }

    // Accept decimal digits only
    for (size_t i = 0; i < value.length(); ++i)
    {
        if (value[i] < '0' || value[i] > '9')
        {
            Serial.println("[TIME] Invalid timestamp");
            return;
        }
    }

    errno = 0;
    char* endPtr = nullptr;

    const unsigned long parsed =
        strtoul(value.c_str(), &endPtr, 10);

    if (errno == ERANGE ||
        endPtr == value.c_str() ||
        *endPtr != '\0' ||
        parsed > UINT32_MAX)
    {
        Serial.println("[TIME] Invalid timestamp");
        return;
    }

    // Valid range for current development testing
    // 2024-01-01 to 2035-01-01 (UTC)
    if (parsed < 1704067200UL ||
        parsed >= 2051222400UL)
    {
        Serial.println("[TIME] Timestamp out of range");
        return;
    }

    struct timeval tv = {};
    tv.tv_sec = (time_t)parsed;
    tv.tv_usec = 0;

    if (settimeofday(&tv, nullptr) != 0)
    {
        Serial.println("[TIME] Synchronization failed");
        return;
    }

    m_timeValid = true;

    Serial.printf(
        "[TIME] Synchronized: %lu\n",
        parsed);
}


//-------------------------------------------------------------
// IsTimeValid
//-------------------------------------------------------------
bool PcTimeProvider::IsTimeValid() const
{
    return m_timeValid;
}


//-------------------------------------------------------------
// GetTimestamp
//-------------------------------------------------------------
uint32_t PcTimeProvider::GetTimestamp() const
{
    if (!m_timeValid)
        return 0;

    const time_t now = time(nullptr);

    if (now < 1704067200LL ||
        now >= 2051222400LL)
    {
        return 0;
    }

    return (uint32_t)now;
}