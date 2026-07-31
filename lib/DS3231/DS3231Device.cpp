//-------------------------------------------------------------
// File : DS3231Device.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 5.0.0
// Description : DS3231
//-------------------------------------------------------------

#include "DS3231Device.h"

namespace SVEMS::Device
{
    bool DS3231Device::Begin()
    {
        SetState(DeviceState::Initializing);

        // 실제 RTC 초기화는 나중에 작성

        return true;
    }

    void DS3231Device::Update()
    {
        // 실제 시간 읽기는 나중에 작성
        
    }
}