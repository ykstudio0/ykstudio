//-------------------------------------------------------------
// File : RS485.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-03
// Project : SVEMS
// Version : 0.1.4
// Descripton : RS485 통신을 위한 클래스 구현
//-------------------------------------------------------------

#include "RS485.h"
#include "Config.h"

HardwareSerial RS485Serial(1);

bool RS485::Begin()
{
    pinMode(PIN_RS485_DE, OUTPUT);

    RxMode();

    RS485Serial.begin(
        MODBUS_BAUDRATE,
        SERIAL_8N1,
        PIN_RS485_RX,
        PIN_RS485_TX
    );

    return true;
}

void RS485::TxMode()
{
    digitalWrite(PIN_RS485_DE, HIGH);
}

void RS485::RxMode()
{
    digitalWrite(PIN_RS485_DE, LOW);
}

void RS485::Send(
    const uint8_t* data, 
    size_t length)
{
    // RX 버퍼 비우기
    while (RS485Serial.available())
    {
        RS485Serial.read();
    }

    TxMode();
    delayMicroseconds(100);
    RS485Serial.write(data, length);
    RS485Serial.flush();
    delayMicroseconds(100);
    // delay(1);
    RxMode();
}

    size_t RS485::Receive(
    uint8_t* buffer,
    size_t maxLength,
    uint32_t timeout)
{
    size_t length = 0;
    uint32_t start = millis();

    while ((millis() - start) < timeout)
    {
        while (RS485Serial.available())
        {
            if (length < maxLength)
            {
                buffer[length++] = RS485Serial.read();
            }
            start = millis();
        }
    }

    return length;
}

bool RS485::Available()
{
    return RS485Serial.available();
}

// uint8_t RS485::Read()
// {
//     return RS485Serial.read();
// }