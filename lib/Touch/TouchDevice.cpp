//-------------------------------------------------------------
// File : TouchDevice.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : FT6336G capacitive touch controller driver
//-------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>

#include "TouchDevice.h"
#include "Logger.h"

namespace
{
    constexpr uint8_t TOUCH_I2C_ADDRESS =
        0x38U;

    constexpr uint8_t TOUCH_RESET_PIN =
        6U;

    constexpr uint8_t TOUCH_INTERRUPT_PIN =
        7U;

    //---------------------------------------------------------
    // FT6336G Registers
    //---------------------------------------------------------

    constexpr uint8_t REG_TOUCH_STATUS =
        0x02U;

    constexpr uint8_t REG_POINT1_X_HIGH =
        0x03U;

    constexpr uint8_t TOUCH_COUNT_MASK =
        0x0FU;

    constexpr uint8_t COORDINATE_HIGH_MASK =
        0x0FU;

    constexpr size_t TOUCH_DATA_LENGTH =
        5U;

    constexpr uint32_t RESET_LOW_TIME_MS =
        10UL;

    constexpr uint32_t RESET_STARTUP_TIME_MS =
        200UL;
}

namespace SVEMS::Device
{
    //---------------------------------------------------------
    // Begin
    //---------------------------------------------------------

    bool TouchDevice::Begin()
    {
        m_ready = false;
        m_touched = false;
        m_x = 0U;
        m_y = 0U;

        pinMode(
            TOUCH_INTERRUPT_PIN,
            INPUT_PULLUP);

        if (!ResetController())
        {
            Logger::Error(
                "TOUCH",
                "Reset failed");

            return false;
        }

        uint8_t touchStatus = 0U;

        if (!ReadRegister(
                REG_TOUCH_STATUS,
                touchStatus))
        {
            Logger::Error(
                "TOUCH",
                "Device not found");

            return false;
        }

        m_ready = true;

        Logger::Info(
            "TOUCH",
            "Ready");

        return true;
    }

    //---------------------------------------------------------
    // Update
    //---------------------------------------------------------

    bool TouchDevice::Update()
    {
        if (!m_ready)
        {
            return false;
        }

        uint8_t data[TOUCH_DATA_LENGTH] = {};

        if (!ReadRegisters(
                REG_TOUCH_STATUS,
                data,
                sizeof(data)))
        {
            m_touched = false;

            return false;
        }

        const uint8_t touchCount =
            data[0] & TOUCH_COUNT_MASK;

        if (touchCount == 0U)
        {
            m_touched = false;

            return true;
        }

        m_x =
            static_cast<uint16_t>(
                data[1] &
                COORDINATE_HIGH_MASK)
            << 8;

        m_x |=
            static_cast<uint16_t>(
                data[2]);

        m_y =
            static_cast<uint16_t>(
                data[3] &
                COORDINATE_HIGH_MASK)
            << 8;

        m_y |=
            static_cast<uint16_t>(
                data[4]);

        m_touched = true;

        return true;
    }

    //---------------------------------------------------------
    // IsReady
    //---------------------------------------------------------

    bool TouchDevice::IsReady() const
    {
        return m_ready;
    }

    //---------------------------------------------------------
    // IsTouched
    //---------------------------------------------------------

    bool TouchDevice::IsTouched() const
    {
        return
            m_ready &&
            m_touched;
    }

    //---------------------------------------------------------
    // ReadPoint
    //---------------------------------------------------------

    bool TouchDevice::ReadPoint(
        uint16_t& x,
        uint16_t& y) const
    {
        if (!IsTouched())
        {
            return false;
        }

        x = m_x;
        y = m_y;

        return true;
    }

    //---------------------------------------------------------
    // ResetController
    //---------------------------------------------------------

    bool TouchDevice::ResetController()
    {
        pinMode(
            TOUCH_RESET_PIN,
            OUTPUT);

        digitalWrite(
            TOUCH_RESET_PIN,
            LOW);

        delay(
            RESET_LOW_TIME_MS);

        digitalWrite(
            TOUCH_RESET_PIN,
            HIGH);

        delay(
            RESET_STARTUP_TIME_MS);

        return true;
    }

    //---------------------------------------------------------
    // ReadRegister
    //---------------------------------------------------------

    bool TouchDevice::ReadRegister(
        uint8_t registerAddress,
        uint8_t& value)
    {
        return ReadRegisters(
            registerAddress,
            &value,
            1U);
    }

    //---------------------------------------------------------
    // ReadRegisters
    //---------------------------------------------------------

    bool TouchDevice::ReadRegisters(
        uint8_t registerAddress,
        uint8_t* buffer,
        size_t length)
    {
        if (buffer == nullptr ||
            length == 0U)
        {
            return false;
        }

        Wire.beginTransmission(
            TOUCH_I2C_ADDRESS);

        Wire.write(
            registerAddress);

        const uint8_t transmissionResult =
            Wire.endTransmission(
                false);

        if (transmissionResult != 0U)
        {
            return false;
        }

        const size_t receivedLength =
            Wire.requestFrom(
                TOUCH_I2C_ADDRESS,
                length,
                true);

        if (receivedLength != length)
        {
            while (Wire.available())
            {
                Wire.read();
            }

            return false;
        }

        for (size_t index = 0U;
            index < length;
            ++index)
        {
            if (!Wire.available())
            {
                return false;
            }

            buffer[index] =
                static_cast<uint8_t>(
                    Wire.read());
        }

        return true;
    }
}
