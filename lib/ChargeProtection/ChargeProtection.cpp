//-------------------------------------------------------------
// File : ChargeProtection.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-09
// Project : SVEMS
// Version : 0.8.6
// Description : ChargeProtection
//-------------------------------------------------------------

#include "ChargeProtection.h"

#include <Arduino.h>

#include "Pins.h"
#include "DataManager.h"


bool ChargeProtection::s_inhibit = false;


//-------------------------------------------------------------
// TEST thresholds
//-------------------------------------------------------------
static constexpr float INHIBIT_ON_CELL  = 3.40f;
static constexpr float INHIBIT_OFF_CELL = 3.30f;


//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void ChargeProtection::Begin()
{
    pinMode(
        PIN_CHARGE_INHIBIT,
        OUTPUT
    );

    s_inhibit = false;

    digitalWrite(
        PIN_CHARGE_INHIBIT,
        LOW
    );
}


//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void ChargeProtection::Update()
{
    if (!DataManager::Battery.status.online)
    {
        return;
    }

    float maxCell = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        const float cell =
            DataManager::Battery.cellVoltage[i];

        if (cell < 2.0f || cell > 4.5f)
        {
            s_inhibit = true;

            digitalWrite(
                PIN_CHARGE_INHIBIT,
                HIGH
            );

            return;
        }

        if (cell > maxCell)
        {
            maxCell = cell;
        }
    }

    if (!s_inhibit)
    {
        if (maxCell >= INHIBIT_ON_CELL)
        {
            s_inhibit = true;
        }
    }
    else
    {
        if (maxCell <= INHIBIT_OFF_CELL)
        {
            s_inhibit = false;
        }
    }

    digitalWrite(
        PIN_CHARGE_INHIBIT,
        s_inhibit ? HIGH : LOW
    );
}