//-------------------------------------------------------------
// File : DeviceManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-13
// Project : SVEMS
// Version : 0.1.9
// Description : Device 관리
//-------------------------------------------------------------

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

class DeviceManager
{
public:
    static bool Begin();
    static bool Update();
    static bool IsReady();

private:
    static bool Ready;
};

#endif