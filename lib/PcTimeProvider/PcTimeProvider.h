//-------------------------------------------------------------
// File : PcTimeProvider.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS PcTimeProvider
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

class PcTimeProvider
{
public:
    void Begin();
    void Update();

    bool IsTimeValid() const;
    uint32_t GetTimestamp() const;

private:
    bool m_timeValid = false;
    String m_inputBuffer;

    void ProcessCommand(const String& command);
};