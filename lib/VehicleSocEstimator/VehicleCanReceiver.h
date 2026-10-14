//-------------------------------------------------------------
// File : VehicleCanReceiver.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Read-only vehicle CAN receiver
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "driver/twai.h"

class VehicleCanReceiver
{
public:
    bool Begin();

    bool Receive(
        twai_message_t& message);

    bool IsReady() const;

private:
    bool m_ready = false;
};