//-------------------------------------------------------------
// File : DeviceManger.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-13
// Project : SVEMS
// Version : 0.1.9
// Descripton : Device 관리
//-------------------------------------------------------------

#include "DeviceManager.h"
#include "Epever.h"

bool DeviceManager::Begin()
{
    bool ok = true;
    ok &= Epever::Begin();

    return ok;
}

bool DeviceManager::Update()
{
    bool ok = true;
    ok &= Epever::Update();

    return ok;
}