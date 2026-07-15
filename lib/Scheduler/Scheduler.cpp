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

namespace
{
    uint32_t Timer100ms = 0;
    uint32_t Timer1sec  = 0;
    uint32_t Timer5sec  = 0;
    uint32_t Timer30sec = 0;
    uint32_t Timer60sec = 0;
}

bool Scheduler::Begin()
{
    uint32_t now = millis();
    Timer100ms = now;
    Timer1sec  = now;
    Timer5sec  = now;
    Timer30sec = now;
    Timer60sec = now;

    return true;
}

void Scheduler::Run()
{
    uint32_t now = millis();

    if (now - Timer100ms >= 100)
    {
        Timer100ms = now;
        Run100ms();
    }

    if (now - Timer1sec >= 1000)
    {
        Timer1sec = now;
        Run1sec();
    }

    if (now - Timer5sec >= 5000)
    {
        Timer5sec = now;
        Run5sec();
    }

    if (now - Timer30sec >= 30000)
    {
        Timer30sec = now;
        Run30sec();
    }

    if (now - Timer60sec >= 60000)
    {
        Timer60sec = now;
        Run60sec();
    }
}

//------------------------------
// 0.1 Second Tasks
//------------------------------
void Scheduler::Run100ms()
{

}

//------------------------------
// 1 Second Tasks
//------------------------------
void Scheduler::Run1sec()
{
    PollSolar();
}

//------------------------------
// 5 Second Tasks
//------------------------------
void Scheduler::Run5sec()
{
    PollBattery();
}

//------------------------------
// 30 Second Tasks
//------------------------------
void Scheduler::Run30sec()
{
    // Epever::ReadTemperature();
}

//------------------------------
// 60 Second Tasks
//------------------------------
void Scheduler::Run60sec()
{
    // Epever::ReadSOC();
}

//------------------------------
// Poll Solar Information
//------------------------------
void Scheduler::PollSolar()
{
    Epever::ReadSolar();
}

//------------------------------
// Poll Battery Information
//------------------------------
void Scheduler::PollBattery()
{
    Epever::ReadBattery();
}

//------------------------------
// Poll Temperature Information
//------------------------------
void Scheduler::PollTemperature()
{
    // Epever::ReadTemperature();
}

//------------------------------
// Poll SOC Information
//------------------------------
void Scheduler::PollSOC()
{
    // Epever::ReadSOC();
}