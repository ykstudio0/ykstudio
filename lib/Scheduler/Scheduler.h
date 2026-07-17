//-------------------------------------------------------------
// File : Scheduler.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Descripton : 시간을 관리
//-------------------------------------------------------------

#ifndef SCHEDULER_H
#define SCHEDULER_H

# include <Arduino.h>

class Scheduler
{
public:
    static bool Begin();
    static void Run();

private:
    static void Run100ms();
    static void Run1sec();
    static void Run5sec();
    static void Run30sec();
    static void Run60sec();

    static void PollSolar();
    static void PollBattery();
    static void PollTemperature();
    static void PollSOC();

    static void Service();
    static void ServiceLogger();
};

#endif