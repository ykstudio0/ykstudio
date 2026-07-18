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
#include "Logger.h"
#include "DataManager.h"

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

    while (now - Timer100ms >= 100)
    {
        Timer100ms += 100;
        Run100ms();
    }

    while (now - Timer1sec >= 1000)
    {
        Timer1sec += 1000;
        Run1sec();
    }

    while (now - Timer5sec >= 5000)
    {
        Timer5sec += 5000;
        Run5sec();
    }

    while (now - Timer30sec >= 30000)
    {
        Timer30sec += 30000;
        Run30sec();
    }

    while (now - Timer60sec >= 60000)
    {
        Timer60sec += 60000;
        Run60sec();
    }

    Service();
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

// void Scheduler::UpdateDisplay()
// {

// }

void Scheduler::Service()
{
    DataManager::UpdateOnlineStatus(millis());
    
    ServiceLogger();
    ServiceDisplay();
    ServiceIoT();

    DataManager::ClearUpdates();
}

void Scheduler::ServiceLogger()
{
    if (DataManager::Solar.status.updated)
    {
        char buffer[32];
        sprintf(buffer, "%.1f V", DataManager::Solar.voltage);
        Logger::Info("PV", buffer);
        
        sprintf(buffer, "%.1f A", DataManager::Solar.current);
        Logger::Info("PV", buffer);

        sprintf(buffer, "%.1f W", DataManager::Solar.power);
        Logger::Info("PV", buffer);

        // DataManager::Solar.status.updated = false;
    }

    if (DataManager::Battery.status.updated)
    {
        char buffer[32];

        sprintf(buffer, "%.1f V", DataManager::Battery.voltage);
        Logger::Info("BATTERY", buffer);

        sprintf(buffer, "%.1f A", DataManager::Battery.current);
        Logger::Info("BATTERY", buffer);

        // DataManager::Battery.status.updated = false;
    }
    // DataManager::ClearUpdates();
}

void Scheduler::ServiceDisplay()
{
    // Display 모듈 구현 후 연결
}

void Scheduler::ServiceIoT()
{
    // Wi-Fi / MQTT / Web 모듈 구현 후 연결
}