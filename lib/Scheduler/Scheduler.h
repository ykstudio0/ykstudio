//-------------------------------------------------------------
// File : Scheduler.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Description : 시간을 관리
//-------------------------------------------------------------

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

class Scheduler
{
public:
    static bool Begin();
    static void Run();
    static void RunInitial();

    static void SetReverseChargePendingCommand(
        const String& command);

private:
    static void Run100ms();
    static void Run1Sec();
    static void Run5Sec();
    static void Run30Sec();
    static void Run60Sec();

    static bool PollSolar();
    static bool PollBattery();
    static bool PollLoad();
    static bool PollTemperature();
    static bool PollSOC();
    static bool PollEnergy();

    static void Service();
    static void ServiceLogger();
    static void ServiceDisplay();
    static void ServiceIoT();

    static bool PollChargingStatus();

    static void PollEpeverDistributed();

    //---------------------------------------------------------
    // Asynchronous EPEVER Solar Polling
    //---------------------------------------------------------
    static void PollSolarAsync();

    static void PollBatteryAsync();
    
    static void PollLoadAsync();

    static void PollChargingAsync();

    static void PollTemperatureAsync();

    static void PollSocAsync();

    static void PollEnergyAsync();

    //---------------------------------------------------------
    // Start asynchronous Solar polling
    //---------------------------------------------------------
    static void StartSolarAsyncPoll();

    static void StartBatteryAsyncPoll();

    static void StartLoadAsyncPoll();

    static void StartChargingAsyncPoll();

    static void StartTemperatureAsyncPoll();

    static void StartSocAsyncPoll();

    static void StartEnergyAsyncPoll();
};

#endif