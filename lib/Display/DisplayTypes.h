//-------------------------------------------------------------
// File : DisplayTypes.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Common display geometry types
//-------------------------------------------------------------
 #pragma once

 #include <Arduino.h>

 namespace DisplayTypes
 {
    // 표시되는 값의 종류
    // Renderer는 이 타입을 보고
    // 단위 문자열과 기본 표시 형식을 결정한다.
    enum class ValueType : uint8_t
    {
        None = 0,

        Voltage,
        Current,
        Power,
        Energy,

        Temperature,
        Humidity,
        Percent,

        Time,
        Duration,

        Text
    };

    // 문자열 정렬
    enum class TextAlign : uint8_t
    {
        Left = 0,
        Center,
        Right
    };

    // 위젯 상태
    // Theme에서는 상태에 따라
    // 색상이나 강조 방식을 결정한다.
    enum class WidgetState : uint8_t
    {
        Normal = 0,
        Active,
        Warning,
        Alarm,
        Disabled,
        Unknown
    };

    // 아이콘 종류
    // 실제 아이콘 데이터와 그리기 코드는
    // DisplayIcons에서 관리한다.
    enum class IconType : uint8_t
    {
        None = 0,

        Overview,
        Solar,
        Battery,
        Load,
        Temperature,
        Humidity,

        WiFi,
        Bluetooth,
        LTE,
        RS485,
        SDCard,

        Charging,
        Warning,
        Error,
        Settings
    };

    // 화념에 표시할 하나의 수치 값
    // 값 자체뿐 아니라 표시 방식에 필요한 정보를
    // 하나의 구조체로 함께 관리한다.
    struct DisplayValue
    {
        float value = 0.0f;

        ValueType type =
            ValueType::None;

        WidgetState state =
            WidgetState::Normal;

        TextAlign align =
            TextAlign::Right;

        uint8_t decimals = 1;

        bool visible = true;

        // 기본 생성자
        constexpr DisplayValue() = default;

        // 값과 타입만 지정하는 생성자
        // 상태, 정렬, 소수점 자리수는 
        // 기본값을 사용한다.
        constexpr DisplayValue(
            float initialValue,
            ValueType valueType)
            : value(initialValue),
              type(valueType)
        {

        }

        // 전체 속성을 지정하는 생성자
        constexpr DisplayValue(
            float initialValue,
            ValueType valueType,
            WidgetState widgetState,
            TextAlign textAlign,
            uint8_t decimalPlaces,
            bool isVisible = true)
            : value(initialValue),
              type(valueType),
              state(widgetState),
              align(textAlign),
              decimals(decimalPlaces),
              visible(isVisible)
        {

        }

        // 표시 가능 상태인지 확인
        constexpr bool IsVisible() const
        {
            return visible &&
                state != WidgetState::Disabled;
        }

        // 값 유효 여부 확인
        // None 타입은 아직 표시할 값이
        // 설정되지 않은 상태로 판단한다.
        constexpr bool IsValid() const
        {
            return type != ValueType::None;
        }

        // 경고 또는 알람 상태인지 확인
        constexpr bool HasAlert() const
        {
            return
                state == WidgetState::Warning ||
                state == WidgetState::Alarm;
        }

        // 값을 갱신한다.
        void SetValue(float newValue)
        {
            value = newValue;
        }

        // 값과 상태를 함께 갱신한다.
        void Set(
            float newValue,
            WidgetState newState)
        {
            value = newValue;
            state = newState;
        }

        // 표시 여부 설정
        void SetVisible(bool isVisible)
        {
            visible = isVisible;
        }
    };

    // 값 타입별 기본 소소점 자리수
    constexpr uint8_t GetDefaultDecimals(
        ValueType type)
    {
        switch (type)
        {
            case ValueType::Voltage:
            case ValueType::Current:
                return 2;
            
            case ValueType::Power:
            case ValueType::Energy:
                return 1;

            case ValueType::Temperature:
            case ValueType::Humidity:
                return 1;

            case ValueType::Percent:
                return 0;

            case ValueType::Time:
            case ValueType::Duration:
            case ValueType::Text:
            case ValueType::None:
            default:
                return 0;
        }
    }

    // 값 타입별 단위 문자열
    inline const char* GetUnit(
        ValueType type)
    {
        switch (type)
        {
            case ValueType::Voltage:
                return "V";
            
            case ValueType::Current:
                return "A";

            case ValueType::Power:
                return "W";

            case ValueType::Energy:
                return "Wh";

            case ValueType::Temperature:
                return "C";

            case ValueType::Humidity:
                return "%";

            case ValueType::Percent:
                return "%";

            case ValueType::Duration:
                return "s";

            case ValueType::Time:
            case ValueType::Text:
            case ValueType::None:
            default:
                return "";
        }
    }

    // 타입을 지정해 DisplayValue를 생성한다.
    // 소수점 자리수는 타입별 기본값을 적용한다.
    constexpr DisplayValue AMakeValue(
        float value,
        ValueType type,
        WidgetState state =
            WidgetState::Normal,
        TextAlign align =
            TextAlign::Right)
    {
        return DisplayValue(
            value,
            type,
            state,
            align,
            GetDefaultDecimals(type),
            true);
    }
}
//  #include <stdint.h>

//  namespace DisplayTypes
//  {
//     // Point
//     struct Point
//     {
//         int16_t x;
//         int16_t y;
//     };

//     // size
//     struct Size
//     {
//         int16_t width;
//         int16_t height;
//     };

//     // Rectangle
//     struct Rect
//     {
//         int16_t x;
//         int16_t y;
//         int16_t width;
//         int16_t height;

//         constexpr int16_t Left() const
//         {
//             return x;
//         }

//         constexpr int16_t Top() const
//         {
//             return y;
//         }

//         constexpr int16_t Right() const
//         {
//             return x + width;
//         }

//         constexpr int16_t Bottom() const
//         {
//             return y + height;
//         }

//         constexpr Point Position() const
//         {
//             return { x, y };
//         }

//         constexpr Size Dimensions() const
//         {
//             return { width, height };
//         }
//     };
//  }