//-------------------------------------------------------------
// File : BMSService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-06
// Project : SVEMS
// Version : 0.5.7
// Description : PowerBank BMS communication service framework
//-------------------------------------------------------------

#include <cstring>
#include <cmath>

#include "BMSService.h"
#include "Pins.h"
#include "Logger.h"
#include "Config.h"
#include "DataManager.h"

namespace
{
    HardwareSerial BMSSerial(2);

    constexpr uint32_t BMS_BAUDRATE =
        115200UL;

    constexpr uint8_t BMS_HEADER[] =
    {
        0x5A,
        0x5D,
        0x1A,
        0x01
    };

    constexpr size_t BMS_HEADER_SIZE =
        sizeof(BMS_HEADER);

    constexpr size_t BMS_FRAME_SIZE =
        96U;

    uint8_t FrameBuffer[BMS_FRAME_SIZE];

    size_t FrameLength = 0U;
    size_t HeaderMatched = 0U;

    uint32_t LastDecodeLogTime = 0U;

    constexpr uint32_t BMS_DECODE_LOG_INTERVAL_MS =
        1000UL;

    //---------------------------------------------------------
    // Big-endian 32-bit value
    //---------------------------------------------------------

    uint32_t ReadUInt32BE(
        const uint8_t* data)
    {
        return
            (static_cast<uint32_t>(data[0]) << 24) |
            (static_cast<uint32_t>(data[1]) << 16) |
            (static_cast<uint32_t>(data[2]) << 8) |
            static_cast<uint32_t>(data[3]);
    }

    //---------------------------------------------------------
    // Big-endian IEEE-754 float
    //---------------------------------------------------------

    float ReadFloatBE(
        const uint8_t* data)
    {
        const uint32_t raw =
            ReadUInt32BE(data);

        float value = 0.0f;

        std::memcpy(
            &value,
            &raw,
            sizeof(value));

        return value;
    }

    void DumpDecodedFrame(
        const uint8_t* frame,
        size_t length)
    {
        if (frame == nullptr ||
            length != BMS_FRAME_SIZE)
        {
            return;
        }

        static uint32_t frameNumber = 0U;

        Serial.println();
        Serial.printf(
            "[%08lu] [BMS DEC] FRAME #%lu\n",
            millis(),
            ++frameNumber);

        Serial.println(
            "OFFSET  RAW          UINT32       FLOAT");

        Serial.println(
            "------------------------------------------");

        // Offset 0x00~0x03은 프레임 헤더이므로 제외한다.
        for (size_t offset = 4U;
             offset + 3U < length;
             offset += 4U)
        {
            const uint32_t raw =
                ReadUInt32BE(
                    &frame[offset]);

            const float value =
                ReadFloatBE(
                    &frame[offset]);

            Serial.printf(
                "0x%02X    "
                "%02X %02X %02X %02X  "
                "%10lu  "
                "%12.5f\n",
                static_cast<unsigned>(offset),
                frame[offset],
                frame[offset + 1U],
                frame[offset + 2U],
                frame[offset + 3U],
                static_cast<unsigned long>(raw),
                value);
        }

        Serial.println(
            "------------------------------------------");
    }
}

namespace SVEMS::Service
{
    bool BMSService::Ready = false;
    bool BMSService::Receiving = false;
    bool BMSService::ValidData = false;

    uint32_t BMSService::LastReceiveTime = 0U;

    BMSData BMSService::Data;

    // Begin
    bool BMSService::Begin()
    {
        Ready = false;
        Receiving = false;
        ValidData = false;

        LastReceiveTime = 0U;

        Data = BMSData();

        FrameLength = 0U;
        HeaderMatched = 0U;

        BMSSerial.begin(
            BMS_BAUDRATE,
            SERIAL_8N1,
            PIN_BMS_RX,
            -1);

        pinMode(
            PIN_BMS_RX,
            INPUT_PULLUP);

        Ready = true;

        Logger::Info(
            "BMS",
            "Ready (115200bps)");

        return Ready;
    }

    bool BMSService::ParseFrame(
        const uint8_t* frame,
        size_t length)
    {
        if (frame == nullptr ||
            length != BMS_FRAME_SIZE)
        {
            return false;
        }

        if (frame[0] != BMS_HEADER[0] ||
            frame[1] != BMS_HEADER[1] ||
            frame[2] != BMS_HEADER[2] ||
            frame[3] != BMS_HEADER[3])
        {
            return false;
        }

        BMSData parsed;

        parsed.socPercent =
            ReadFloatBE(
                &frame[0x04]);

        parsed.cellVoltage1 =
            ReadFloatBE(
                &frame[0x08]);

        parsed.cellVoltage2 =
            ReadFloatBE(
                &frame[0x0C]);

        parsed.cellVoltage3 =
            ReadFloatBE(
                &frame[0x10]);

        parsed.cellVoltage4 =
            ReadFloatBE(
                &frame[0x18]);

        parsed.packVoltage =
            ReadFloatBE(
                &frame[0x44]);

        parsed.packCurrent =
            ReadFloatBE(
                &frame[0x48]);

        parsed.batteryTemperature =
            ReadFloatBE(
                &frame[0x4C]);

        parsed.bmsTemperature =
            ReadFloatBE(
                &frame[0x50]);

        parsed.externalTemperature =
            ReadFloatBE(
                &frame[0x54]);

        parsed.cellCount =
            ReadUInt32BE(
                &frame[0x58]);

        parsed.remainingCapacity =
            ReadFloatBE(
                &frame[0x5C]);

        //-----------------------------------------------------
        // 최소 유효성 검사
        //-----------------------------------------------------

        if (parsed.cellCount == 0U ||
            parsed.cellCount > 16U)
        {
            return false;
        }

        if (parsed.socPercent < 0.0f ||
            parsed.socPercent > 100.0f)
        {
            return false;
        }

        if (parsed.packVoltage < 0.0f ||
            parsed.packVoltage > 100.0f)
        {
            return false;
        }

        if (!std::isfinite(parsed.socPercent) ||
            !std::isfinite(parsed.packVoltage) ||
            !std::isfinite(parsed.packCurrent) ||
            !std::isfinite(parsed.remainingCapacity) ||
            !std::isfinite(parsed.batteryTemperature) ||
            !std::isfinite(parsed.bmsTemperature) ||
            !std::isfinite(parsed.externalTemperature))
        {
            return false;
        }

        if (!std::isfinite(parsed.cellVoltage1) ||
            !std::isfinite(parsed.cellVoltage2) ||
            !std::isfinite(parsed.cellVoltage3) ||
            !std::isfinite(parsed.cellVoltage4))
        {
            return false;
        }

        if (parsed.cellVoltage1 < 0.0f ||
            parsed.cellVoltage1 > 5.0f ||
            parsed.cellVoltage2 < 0.0f ||
            parsed.cellVoltage2 > 5.0f ||
            parsed.cellVoltage3 < 0.0f ||
            parsed.cellVoltage3 > 5.0f ||
            parsed.cellVoltage4 < 0.0f ||
            parsed.cellVoltage4 > 5.0f)
        {
            return false;
        }

        Data = parsed;
        ValidData = true;

        const uint32_t now =
            millis();

        // Battery
        DataManager::Battery.voltage =
            parsed.packVoltage;

        DataManager::Battery.current =
            parsed.packCurrent;

        DataManager::Battery.power=
            parsed.packVoltage *
            parsed.packCurrent;

        DataManager::Battery.remainingCapacity =
            parsed.remainingCapacity;

        DataManager::Battery.cellVoltage[0] =
            parsed.cellVoltage1;

        DataManager::Battery.cellVoltage[1] =
            parsed.cellVoltage2;

        DataManager::Battery.cellVoltage[2] =
            parsed.cellVoltage3;

        DataManager::Battery.cellVoltage[3] =
            parsed.cellVoltage4;

        DataManager::Battery.cellCount =
            static_cast<uint8_t>(
                parsed.cellCount);

        DataManager::Battery.status.updated = true;
        DataManager::Battery.status.online = true;
        DataManager::Battery.status.lastUpdate = now;

        // SOC
        float soc =
            parsed.socPercent;

        if (soc < 0.0f)
        {
            soc = 0.0f;
        }
        else if (soc > 100.0f)
        {
            soc = 100.0f;
        }

        DataManager::Soc.value =
            static_cast<uint8_t>(
                soc + 0.5f);

        DataManager::Soc.status.updated = true;
        DataManager::Soc.status.online = true;
        DataManager::Soc.status.lastUpdate = now;

        // Temperature
        DataManager::Temperature.battery =
            parsed.batteryTemperature;

        DataManager::Temperature.bms =
            parsed.bmsTemperature;

        DataManager::Temperature.powerBankExternal =
            parsed.externalTemperature;

        DataManager::Temperature.powerBankStatus.updated = true;
        DataManager::Temperature.powerBankStatus.online = true;
        DataManager::Temperature.powerBankStatus.lastUpdate = now;

        return true;
    }

    // Update
    void BMSService::Update()
    {
        if (!Ready)
        {
            return;
        }

        while (BMSSerial.available() > 0)
        {
            const int value =
                BMSSerial.read();

            if (value < 0)
            {
                break;
            }

            const uint8_t byte =
                static_cast<uint8_t>(value);

            LastReceiveTime = millis();
            Receiving = true;

            //-------------------------------------------------
            // 프레임을 아직 수집하지 않는 상태
            // 5A 5D 1A 01 헤더를 찾는다.
            //-------------------------------------------------

            if (FrameLength == 0U)
            {
                if (byte ==
                    BMS_HEADER[HeaderMatched])
                {
                    FrameBuffer[HeaderMatched] =
                        byte;

                    ++HeaderMatched;

                    if (HeaderMatched >=
                        BMS_HEADER_SIZE)
                    {
                        FrameLength =
                            BMS_HEADER_SIZE;

                        HeaderMatched = 0U;
                    }
                }
                else
                {
                    //-------------------------------------------------
                    // 현재 바이트가 다시 헤더 첫 바이트라면
                    // 새 헤더 후보로 유지한다.
                    //-------------------------------------------------

                    if (byte == BMS_HEADER[0])
                    {
                        FrameBuffer[0] = byte;
                        HeaderMatched = 1U;
                    }
                    else
                    {
                        HeaderMatched = 0U;
                    }
                }

                continue;
            }

            //-------------------------------------------------
            // 헤더 이후 프레임 데이터 수집
            //-------------------------------------------------

            if (FrameLength < BMS_FRAME_SIZE)
            {
                FrameBuffer[FrameLength++] =
                    byte;
            }

            //-------------------------------------------------
            // 99바이트 프레임 완성
            //-------------------------------------------------

            if (FrameLength >= BMS_FRAME_SIZE)
            {
                if (ParseFrame(
                        FrameBuffer,
                        FrameLength))
                {
                    static uint32_t lastLogTime = 0U;

                    const uint32_t now =
                        millis();

                    if (now - lastLogTime >= 5000U)
                    {
                        lastLogTime = now;

                        Serial.printf(
                            "[%08lu] [BMS] "
                            "SOC=%.2f%% "
                            "PACK=%.3fV "
                            "CURRENT=%.3fA "
                            "CAP=%.2fAh\n",
                            now,
                            Data.socPercent,
                            Data.packVoltage,
                            Data.packCurrent,
                            Data.remainingCapacity);

                        Serial.printf(
                            "[%08lu] [BMS] "
                            "CELL=%.4f/%.4f/%.4f/%.4fV\n",
                            now,
                            Data.cellVoltage1,
                            Data.cellVoltage2,
                            Data.cellVoltage3,
                            Data.cellVoltage4);

                        Serial.printf(
                            "[%08lu] [BMS] "
                            "TEMP=%.2f/%.2f/%.2fC "
                            "CELLS=%lu\n",
                            now,
                            Data.batteryTemperature,
                            Data.bmsTemperature,
                            Data.externalTemperature,
                            static_cast<unsigned long>(
                                Data.cellCount));

                        Serial.printf(
                            "[%08lu] [DATA] "
                            "BAT=%.3fV %.3fA %.1fW "
                            "SOC=%u%% TEMP=%.2fC\n",
                            now,
                            DataManager::Battery.voltage,
                            DataManager::Battery.current,
                            DataManager::Battery.power,
                            DataManager::Soc.value,
                            DataManager::Temperature.battery);
                    }
                }
                else
                {
                    Logger::Warning(
                        "BMS",
                        "Invalid Frame");
                }

                FrameLength = 0U;
                HeaderMatched = 0U;
            }
        }

        //-----------------------------------------------------
        // 일정 시간 수신이 없으면 Receiving 해제
        //-----------------------------------------------------

        if (Receiving &&
            millis() - LastReceiveTime > 2000U)
        {
            Receiving = false;
        }
    }

    bool BMSService::HasValidData()
    {
        return ValidData;
    }

    const BMSData& BMSService::GetData()
    {
        return Data;
    }

    // IsReady
    bool BMSService::IsReady()
    {
        return Ready;
    }

    // IsReceiving
    bool BMSService::IsReceiving()
    {
        return Receiving;
    }

    // GetLastReceiveTime
    uint32_t BMSService::GetLastReceiveTime()
    {
        return LastReceiveTime;
    }
}