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
#include "Logger.h"

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

    Logger::Info("RS485", "Ready");

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
    Logger::Info("RS485","SEND");
    
    // RX 버퍼 비우기
    while (RS485Serial.available())
    {
        RS485Serial.read();
    }

    TxMode();
    delayMicroseconds(300);
    RS485Serial.write(data, length);
    RS485Serial.flush();
    delayMicroseconds(300);
    RxMode();
}

static bool ReceiveFrame(
    uint8_t* buffer,
    size_t& length,
    size_t maxLength,
    uint32_t timeout = MODBUS_TIMEOUT);

bool RS485::ReceiveFrame(
    uint8_t* buffer,
    size_t& length,
    size_t maxLength,
    uint32_t timeout)
{
    length = Receive(
        buffer,
        maxLength,
        timeout);
    
    if (length == 0)
    {
        return false;
    }
    return true;
}
    
    size_t RS485::Receive(
    uint8_t* buffer,
    size_t maxLength,
    uint32_t timeout)
{
    size_t length = 0;
    uint32_t start = millis();

    while (!RS485Serial.available())
    {
        if (millis() - start > timeout)
            return 0;
    }

    uint32_t lastByte = micros();

    while (true)
    {
        while (RS485Serial.available())
        {
            buffer[length++] = RS485Serial.read();
            lastByte = micros();
        }

        if ((micros() - lastByte)> 2500)
            break;
    }

    return length;
    // while ((millis() - start) < timeout)
    // {
    //     if (RS485Serial.available())
    //     {
    //         break;
    //     }
    // }

    // if (!RS485Serial.available())
    // {
    //     return 0;
    // }

    // delay(3);   // 프레임 수신 완료 대기

    // while (RS485Serial.available())
    // {
    //     if (length < maxLength)
    //     {
    //         buffer[length++] = RS485Serial.read();
    //     }
    //     start = millis();
    // }

    // return length;
}

bool RS485::Available()
{
    return RS485Serial.available();
}
