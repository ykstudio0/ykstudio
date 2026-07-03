#include "RS485.h"

HardwareSerial RS485Serial(1);

bool RS485::Begin()
{
    pinMode(DIR_PIN, OUTPUT);

    RxMode();

    RS485Serial.begin(
        115200,
        SERIAL_8N1,
        RX_PIN,
        TX_PIN
    );

    return true;
}

void RS485::TxMode()
{
    digitalWrite(DIR_PIN, HIGH);
}

void RS485::RxMode()
{
    digitalWrite(DIR_PIN, LOW);
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