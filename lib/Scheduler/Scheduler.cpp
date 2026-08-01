//-------------------------------------------------------------
// File : Scheduler.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Description : 시간을 관리
//-------------------------------------------------------------

#include "Scheduler.h"
#include "Epever.h"
#include "Logger.h"
#include "DataManager.h"
#include "Display.h"
#include "DeviceManager.h"

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

void Scheduler::RunInitial()
{

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
        Run1Sec();
    }

    while (now - Timer5sec >= 5000)
    {
        Timer5sec += 5000;
        Run5Sec();
    }

    while (now - Timer30sec >= 30000)
    {
        Timer30sec += 30000;
        Run30Sec();
    }

    while (now - Timer60sec >= 60000)
    {
        Timer60sec += 60000;
        Run60Sec();
    }

    Service();
}

// 0.1 Second Tasks
void Scheduler::Run100ms()
{

}

// 1 Second Tasks
void Scheduler::Run1Sec()
{
    DeviceManager::Update();
    // PollSolar();
}

// 5 Second Tasks
void Scheduler::Run5Sec()
{
    // PollBattery();
    // PollLoad();
    // PollChargingStatus();
}

// 30 Second Tasks
void Scheduler::Run30Sec()
{
    // PollTemperature();
    // PollSOC();
}

// 60 Second Tasks
void Scheduler::Run60Sec()
{
    
}

// Poll Solar Information
void Scheduler::PollSolar()
{
    Epever::ReadSolar();
}

// Poll Battery Information
void Scheduler::PollBattery()
{
    Epever::ReadBattery();
}

void Scheduler::PollLoad()
{
    Epever::ReadLoad();
}

// Poll Temperature Information
void Scheduler::PollTemperature()
{
    Epever::ReadTemperature();
}

// Poll SOC Information
void Scheduler::PollSOC()
{
    Epever::ReadSOC();
}

void Scheduler::PollChargingStatus()
{
    Epever::ReadChargingStatus();
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
    char buffer[32];

    if (DataManager::Solar.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Solar.voltage);
        Logger::Info("PV", buffer);
        
        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Solar.current);
        Logger::Info("PV", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Solar.power);
        Logger::Info("PV", buffer);
    }

    if (DataManager::Battery.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Battery.voltage);
        Logger::Info("BATTERY", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Battery.current);
        Logger::Info("BATTERY", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Battery.power);
        Logger::Info("BATTERY", buffer);
    }
    
    if (DataManager::Load.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f V", 
            DataManager::Load.voltage);
        Logger::Info("LOAD", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f A", 
            DataManager::Load.current);
        Logger::Info("LOAD", buffer);

        snprintf(buffer, sizeof(buffer), "%.1f W", 
            DataManager::Load.power);
        Logger::Info("LOAD", buffer);
    }

    if (DataManager::Temperature.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%.1f ℃", 
            DataManager::Temperature.battery);
        Logger::Info("BAT TEMP", buffer);
        
        snprintf(buffer, sizeof(buffer), "%.1f ℃", 
            DataManager::Temperature.device);
        Logger::Info("DEV TEMP", buffer);
    }

    if (DataManager::Soc.status.updated)
    {
        snprintf(buffer, sizeof(buffer), "%u %%", 
            static_cast<unsigned int>(DataManager::Soc.value));
        Logger::Info("SOC", buffer);
    }
}

void Scheduler::ServiceDisplay()
{
    static uint32_t previousMs = 0U;

    const uint32_t now = millis();

    if (now - previousMs < 500U)
    {
        return;
    }

    previousMs = now;
    
    Display::Update();
}

void Scheduler::ServiceIoT()
{
    // Wi-Fi / MQTT / Web 모듈 구현 후 연결
}