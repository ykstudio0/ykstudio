//-------------------------------------------------------------
// File : RS485.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-03
// Project : SVEMS
// Version : 0.1.3
// Descripton : RS485 통신을 위한 클래스 구현
//-------------------------------------------------------------

#include "RS485.h"

HardwareSerial RS485Serial(1);

bool RS485::Begin()
{
    pinMode(PIN_RS485_DIR, OUTPUT);

    RxMode();

    RS485Serial.begin(
        115200,
        SERIAL_8N1,
        PIN_RS485_RX,
        PIN_RS485_TX
    );

    return true;
}

void RS485::TxMode()
{
    digitalWrite(PIN_RS485_DIR, HIGH);
}

void RS485::RxMode()
{
    digitalWrite(PIN_RS485_DIR, LOW);
}

void RS485::Send(const uint8_t* data, size_t length)
{
    TxMode();
    delayMicroseconds(100);
    RS485Serial.write(data, length);
    RS485Serial.flush();
    delayMicroseconds(100);
    RxMode();
}

bool RS485::Available()
{
    return RS485Serial.available();
}

uint8_t RS485::Read()
{
    return RS485Serial.read();
}