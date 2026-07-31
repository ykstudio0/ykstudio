//-------------------------------------------------------------
// File : SHT40Device.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.0
// Description : SHT40
//-------------------------------------------------------------

#include "SHT40Device.h"

namespace SVEMS::Device
{
    bool SHT40Device::Begin()
    {
        SetState(DeviceState::Initializing);

        // 실제 SHT40 초기화는 저녁 하드웨어 테스트 때 작성
        SetState(DeviceState::Online);

        return true;
    }

    void SHT40Device::Update()
    {
        // 실제 온도/습도 읽기는 나중에 작성
    }
}