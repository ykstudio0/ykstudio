//-------------------------------------------------------------
// File : DeviceManger.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-13
// Project : SVEMS
// Version : 0.1.9
// Description : Device 관리
//-------------------------------------------------------------

#include "DeviceManager.h"
#include "Logger.h"
#include "Epever.h"
#include "Scheduler.h"

bool DeviceManager::Ready = false;

bool DeviceManager::Begin()
{
    bool ok = true;

    ok &= Epever::Begin();
    ok &= Scheduler::Begin();

    Ready = ok;

    if (ok)
        Logger::Info("DEVICES", "Ready");
    else
        Logger::Error("DEVICES", "Init Failed");
        
    return ok;
}

bool DeviceManager::Update()
{
    Scheduler::Run();

    return true;
}

bool DeviceManager::IsReady()
{
    return Ready;
}