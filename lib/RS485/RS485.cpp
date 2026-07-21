//-------------------------------------------------------------
// File : RS485.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-03
// Project : SVEMS
// Version : 0.1.4
// Description : RS485 통신을 위한 클래스 구현
//-------------------------------------------------------------

#include "RS485.h"
#include "Config.h"
#include "Logger.h"
#include "Pins.h"

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

// // Begin
// bool RS485::Begin()
// {
//     pinMode(PIN_RS485_DE, OUTPUT);

//     digitalWrite(PIN_RS485_DE, LOW);

//     RS485Serial.begin(
//         MODBUS_BAUDRATE,
//         SERIAL_8N1,
//         PIN_RS485_RX,
//         PIN_RS485_TX);

//         ClearReceiveBuffer();

//         Logger::Info("RS485", "Ready");

//         return true;
// }

// // Send
// bool RS485::Send(
//     const uint8_t* data,
//     size_t length)
// {
//     if (data == nullptr || length == 0)
//     {
//         Logger::Error("RS485", "Invalid TX Buffer");
//         return false;
//     }

//     // 이전 통신에서 남은 수신 데이터를 제거
//     ClearReceiveBuffer();

//     // 송신 모드
//     digitalWrite(PIN_RS485_DE, HIGH);

//     // 트랜시버 전환 안정화
//     delayMicroseconds(100);

//     size_t written = 
//         RS485Serial.write(data, length);

//     // UART 송신 완료까지 대기
//     RS485Serial.flush();

//     // 마지막 Stop Bit가 실제 선로에 나가는 시간을 고려
//     delayMicroseconds(100);

//     // 수신 모드
//     digitalWrite(PIN_RS485_DE, LOW);

//     Logger::Info("RS485", "SEND");

//     if (written != length)
//     {
//         Logger::Error(
//             "RS485", 
//             "TX Incomplete (" +
//             String(written) +
//             "/" +
//             String(length) +
//             ")");

//         return false;
//     }

//     return true;
// }    

// ReceiveFrame
bool RS485::ReceiveFrame(
    uint8_t* buffer,
    size_t& length,
    size_t maxLength,
    uint32_t timeout)
{
    length = 0;

    if (buffer == nullptr || maxLength < 5)
    {
        Logger::Error("RS485", "Invalid RX Buffer");
        return false;
    }

    // 이전 데이터가 새 프레임에 섞이지 않도록 초기화
    memset(buffer, 0, maxLength);

    const uint32_t startTime = millis();

    // // 첫 바이트 대기
    // if (!WaitForData(startTime, timeout))
    // {
    //     return false;
    // }

    // 프레임 수신
    size_t expectedLength = 0;
    while (true)
    {
        // 전체 타임아웃 검사
        if (static_cast<uint32_t>(
            millis() - startTime) >= timeout)
        {
            if (length > 0)
            {
                Logger::Hex(
                    "RX PARTIAL",
                    buffer,
                    length);

                Logger::Warning(
                    "RS485",
                    "RX Timeout (" +
                    String(length) +
                    "/" +
                    String(expectedLength) +
                    ")");
            }

            return false;
        }
        // 현재 UART에 들어온 데이터를 모두 읽음
        while (RS485Serial.available() > 0)
        {
            if (length >= maxLength)
            {
                Logger::Error(
                    "RS485",
                    "RX Buffer Overflow");

                return false;
            }

            const int value = RS485Serial.read();

            if (value < 0)
            {
                continue;
            }

            buffer[length++] =
                static_cast<uint8_t>(value);

            // 현재 프레임의 헤더로 전체 길이 계산
            if (expectedLength == 0)
            {
                DetermineFrameLength(
                    buffer,
                    length,
                    expectedLength);
                
                if (expectedLength > maxLength)
                {
                    Logger::Error(
                        "RS485",
                        "Frame Too Large (" +
                        String(expectedLength) +
                        "/" +
                        String(maxLength) +
                        ")");

                    return false;
                }
            }

            // 정상 프레임 수신 완료
            if (expectedLength > 0 &&
                length == expectedLength)
            {
                return true;
            }

            if (expectedLength > 0 &&
                length > expectedLength)
            {
                Logger::Error(
                    "RS485",
                    "RX Length Overflow");
                
                return false;
            }
        }
        
        // 지나친 Busy Loop 방지
        yield();
    }
}

// // WaitForData
// bool RS485::WaitForData(
//     uint32_t startTime,
//     uint32_t timeout)
// {
//     while (RS485Serial.available() == 0)
//     {
//         if (IsTimeOut(startTime, timeout))
//         {
//             return false;
//         }

//         yield();
//     }

//     return true;
// }

// ReadByte
// bool RS485::ReadByte(
//     uint8_t& value,
//     uint32_t startTime,
//     uint32_t timeout)
// {
//     while (RS485Serial.available() == 0)
//     {
//         if (IsTimeOut(startTime, timeout))
//         {
//             return false;
//         }

//         yield();
//     }

//     int received = RS485Serial.read();

//     if (received < 0)
//     {
//         return false;
//     }

//     value = static_cast<uint8_t>(received);

//     return true;
// }

// DetermineFrameLength
bool RS485::DetermineFrameLength(
    const uint8_t* buffer,
    size_t receivedLength,
    size_t& expectedLength)
{
    expectedLength = 0;

    // Slave + Funtion 필요
    if (receivedLength < 2)
    {
        return false;
    }

    const uint8_t function = buffer[1];

    // Modbus Exception 응답
    //
    // Slave     1
    // Function  1
    // Exception 1
    // CRC       2
    if ((function & 0x80U) != 0)
    {
        expectedLength = 5;
        return true;
    }

    // Byte Count 기반 응답
    //
    // 01 : Read Coils
    // 02 : Read Discrete Inputs
    // 03 : Read Holding Registers
    // 04 : Read Input Registers
    switch (function)
    {
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        {
            if (receivedLength < 3)
            {
                return false;
            }

            const size_t byteCount =
                static_cast<size_t>(buffer[2]);

            expectedLength =
                3U +
                byteCount +
                2U;

            return true;
        }

        // 고정 8바이트 응답
        //
        // 05 : Write Single Coil
        // 06 : Write Single Register
        // 0F : Write Multiple Coils
        // 10 : Write Multiple Registers
        case 0x05:
        case 0x06:
        case 0x0F:
        case 0x10:
        {
            expectedLength = 8;
            return true;
        }

        default:
        {
            return false;
        }
    }
}

// ClearReceiveBuffer
void RS485::ClearReceiveBuffer()
{
    while (RS485Serial.available() > 0)
    {
        RS485Serial.read();
    }
}

// IsTimeOut
bool RS485::IsTimeOut(
    uint32_t startTime,
    uint32_t timeout)
{
    return
        static_cast<uint32_t>(
            millis() - startTime) >= timeout;
}
// HardwareSerial RS485Serial(1);

// bool RS485::Begin()
// {
//     pinMode(PIN_RS485_DE, OUTPUT);

//     RxMode();

//     RS485Serial.begin(
//         MODBUS_BAUDRATE,
//         SERIAL_8N1,
//         PIN_RS485_RX,
//         PIN_RS485_TX
//     );

//     Logger::Info("RS485", "Ready");

//     return true;
// }

// void RS485::TxMode()
// {
//     digitalWrite(PIN_RS485_DE, HIGH);
// }

// void RS485::RxMode()
// {
//     digitalWrite(PIN_RS485_DE, LOW);
// }

// void RS485::Send(
//     const uint8_t* data, 
//     size_t length)
// {
//     Logger::Info("RS485","SEND");
    
//     // RX 버퍼 비우기
//     while (RS485Serial.available())
//     {
//         RS485Serial.read();
//     }

//     TxMode();
//     delayMicroseconds(300);
//     RS485Serial.write(data, length);
//     RS485Serial.flush();
//     delayMicroseconds(300);
//     RxMode();
// }

// static bool ReceiveFrame(
//     uint8_t* buffer,
//     size_t& length,
//     size_t maxLength,
//     uint32_t timeout = MODBUS_TIMEOUT);

// bool RS485::ReceiveFrame(
//     uint8_t* buffer,
//     size_t& length,
//     size_t maxLength,
//     uint32_t timeout)
// {
//     length = Receive(
//         buffer,
//         maxLength,
//         timeout);
    
//     if (length == 0)
//     {
//         return false;
//     }
//     return true;
// }
    
//     size_t RS485::Receive(
//     uint8_t* buffer,
//     size_t maxLength,
//     uint32_t timeout)
// {
//     size_t length = 0;
//     uint32_t start = millis();

//     // 첫 바이트 대기
//     while (!RS485Serial.available())
//     {
//         if (millis() - start > timeout)
//             return 0;
//     }

//     uint32_t lastByte = micros();

//     // 수신
//     while (true)
//     {
//         while (RS485Serial.available())
//         {
//             if (length >= maxLength)
//             {
//                 return length;
//             }

//             buffer[length++] = RS485Serial.read();
//             lastByte = micros();
//         }

//         if ((micros() - lastByte)> 3500)
//         {
//             break;
//         }

//         if ((millis() - start) >= timeout)
//         {
//             break;
//         }
//     }

//     return length;
// }

// bool RS485::Available()
// {
//     return RS485Serial.available();
// }
