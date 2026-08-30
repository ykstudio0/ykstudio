//-------------------------------------------------------------
// File : ReverseChargeTypes.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-30
// Project : SVEMS
// Version : 0.7.0
// Description : ReverseCharge Management
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace ReverseCharge
{

    enum class Mode : uint8_t
    {
        Normal = 0,
        Soft,
        Hard
    };

    enum class State : uint8_t
    {
        Idle = 0,

        // Normal / Soft 자동충전 판단
        WaitAfterIg2Off,
        LowVoltageConfirm,

        // 실제 충전
        Charging,

        // 충전 종료 후 휴지전압 확인
        Resting,

        // 안전조건으로 정지
        SafetyStop
    };

    enum class SafetyReason : uint8_t
    {
        None = 0,

        InvalidVoltage,
        OverVoltage
    };

    struct Config
    {
        // ---------------------------------------------------------
        // Normal Mode
        // ---------------------------------------------------------

        static constexpr float NORMAL_START_VOLTAGE =
            12.10f;

        // ---------------------------------------------------------
        // Soft Mode
        // ---------------------------------------------------------

        static constexpr float SOFT_START_VOLTAGE =
            11.80f;

        #if defined(SVEMS_REVERSE_CHARGE_TEST_MODE)

            // ---------------------------------------------------------
            // Test MAIN - accelerated timing
            // ---------------------------------------------------------

            static constexpr uint32_t AFTER_IG2_OFF_DELAY_MS =
                5UL * 1000UL;

            static constexpr uint32_t LOW_VOLTAGE_CONFIRM_MS =
                3UL * 1000UL;

            static constexpr uint32_t MIN_CHARGE_TIME_MS =
                10UL * 1000UL;

            static constexpr uint32_t REST_TIME_MS =
                5UL * 1000UL;

            static constexpr uint32_t VALID_VOLTAGE_RECOVERY_MS =
                2UL * 1000UL;

            static constexpr uint32_t HARD_CHARGE_TIME_MS =
                15UL * 1000UL;

        #else
            // ---------------------------------------------------------
            // Real vehicle
            // ---------------------------------------------------------

            // SafetyStop에서 빠져나오는 처리
            static constexpr uint32_t
                VALID_VOLTAGE_RECOVERY_MS =
                    10UL * 1000UL;

            // ---------------------------------------------------------
            // Common
            // ---------------------------------------------------------

            // IG2 OFF 후 차량 Sleep 대기
            static constexpr uint32_t AFTER_IG2_OFF_DELAY_MS =
                30UL * 60UL * 1000UL;

            // 저전압 지속 확인
            static constexpr uint32_t LOW_VOLTAGE_CONFIRM_MS =
                3UL * 60UL * 1000UL;

            // 최소 충전 유지시간
            static constexpr uint32_t MIN_CHARGE_TIME_MS =
                10UL * 60UL * 1000UL;

            // 충전 OFF 후 휴지전압 확인 대기
            static constexpr uint32_t REST_TIME_MS =
                60UL * 1000UL;

            // Hard Mode 충전 지속시간
            static constexpr uint32_t HARD_CHARGE_TIME_MS =
                30UL * 60UL * 1000UL;

        #endif

        // 1차 안전 상한값
        // 실제 차량 테스트 후 조정
        static constexpr float MAX_CHARGE_VOLTAGE =
            14.40f;

        // 비정상 ADC 판단용
        static constexpr float MIN_VALID_VOLTAGE =
            6.0f;

        static constexpr float MAX_VALID_VOLTAGE =
            16.0f;

        // Normal Mode 충전 종료 전압
        static constexpr float NORMAL_STOP_VOLTAGE =
            12.60f;
    };

    struct Input
    {
        bool ig2Active = false;

        bool voltageValid = false;

        float vehicleVoltage = 0.0f;

        bool manualStart = false;

        bool manualStop = false;
    };

    struct Output
    {
        bool chargeEnabled = false;

        Mode mode =
            Mode::Normal;

        State state =
            State::Idle;
    };

}