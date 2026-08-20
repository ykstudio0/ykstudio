//-------------------------------------------------------------
// File : VehicleInput.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-20
// Project : SVEMS
// Version : 0.6.3
// Description : VehicleInput
//-------------------------------------------------------------

#include "VehicleInput.h"
#include "DataManager.h"

bool VehicleInput::Active = false;

bool VehicleInput::Begin()
{
    Active = false;
    return true;
}

void VehicleInput::Update()
{
    // PC817 도착 후 GPIO 입력 처리 추가
    DataManager::Vehicle.active = Active;
}

bool VehicleInput::IsActive()
{
    return Active;
}