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
#include <cmath>

#include "Pins.h"
#include "DataManager.h"
#include "Logger.h"


bool ChargeProtection::s_inhibit = false;
bool ChargeProtection::s_overVoltageLatched = false;


//-------------------------------------------------------------
// TEST thresholds
//-------------------------------------------------------------
static constexpr float INHIBIT_ON_CELL  = 3.585f;
static constexpr float INHIBIT_OFF_CELL = 3.45f;


//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void ChargeProtection::Begin()
{
    digitalWrite(PIN_CHARGE_INHIBIT, HIGH);
    pinMode(PIN_CHARGE_INHIBIT, OUTPUT);

    s_inhibit = true;
    s_overVoltageLatched = false;
}


//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void ChargeProtection::Update()
{
    // BMS Offline: 충전 차단
    if (!DataManager::Battery.status.online)
    {
        s_inhibit = true;
        digitalWrite(PIN_CHARGE_INHIBIT, HIGH);

        static uint32_t lastLog = 0;
        const uint32_t now = millis();

        if (now - lastLog >= 5000U)
        {
            lastLog = now;

            Logger::Info(
                "CHG PROTECT",
                "Inhibit=" + String(s_inhibit) +
                " GPIO42=" + String(digitalRead(PIN_CHARGE_INHIBIT)) +
                " BMS=" + String(DataManager::Battery.status.online)
            );
        }

        return;
    }

    float maxCell = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        const float cell =
            DataManager::Battery.cellVoltage[i];

        // 비정상 셀 전압: 충전 차단
        if (!std::isfinite(cell) ||
            cell < 2.0f || cell > 4.5f)
        {
            s_inhibit = true;
            digitalWrite(PIN_CHARGE_INHIBIT, HIGH);
            return;
        }

        if (cell > maxCell)
        {
            maxCell = cell;
        }
    }

    // 과전압 감지
    if (maxCell >= INHIBIT_ON_CELL)
    {
        s_overVoltageLatched = true;
    }

    // 과전압 이력이 있다면 복구 기준 적용
    if (s_overVoltageLatched)
    {
        if (maxCell <= INHIBIT_OFF_CELL)
        {
            s_overVoltageLatched = false;
        }
    }

    // 최초 정상 수신 또는 과전압 해제 후 충전 허용
    s_inhibit = s_overVoltageLatched;

    digitalWrite(
        PIN_CHARGE_INHIBIT,
        s_inhibit ? HIGH : LOW
    );
}