//-------------------------------------------------------------
// File : Scheduler.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Descripton : 시간을 관리
//-------------------------------------------------------------

#include "Scheduler.h"
#include "Epever.h"

static uint32_t LastPoll = 0;

bool Scheduler::Begin()
{
    return true;
}

void Scheduler::Run()
{
    uint32_t now = millis();

    if (now - LastPoll >= 1000)
    {
        LastPoll = now;
        
        Epever::Update();
    }
}