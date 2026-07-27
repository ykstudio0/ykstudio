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

private:
    static void Run100ms();
    static void Run1Sec();
    static void Run5Sec();
    static void Run30Sec();
    static void Run60Sec();

    static void PollSolar();
    static void PollBattery();
    static void PollLoad();
    static void PollTemperature();
    static void PollSOC();

    static void Service();
    static void ServiceLogger();
    static void ServiceDisplay();
    static void ServiceIoT();

    static void PollChargingStatus();
};

#endif