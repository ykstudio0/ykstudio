//-------------------------------------------------------------
// File : main.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-01
// Project : SVEMS
// Version : 0.1.0
// Description : Solar Vehicle Energy Management System(SVEMS) Main Application
//-------------------------------------------------------------

#include <Arduino.h>

#include "StatusLED.h"
#include "Logger.h"
#include "RS485.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "DeviceManager.h"
#include "Display.h"
#include "DisplayModel.h"
#include "IRenderTarget.h"
#include "DisplayRenderer.h"
#include "SerialRenderTarget.h"
#include "Tests.h"
#include "LGFX_Config.h"
#include "TFTRenderTarget.h"
#include "Scheduler.h"
#include "WifiService.h"
#include "NtpService.h"
#include "BMSService.h"
#include "Version.h"
#include "VehicleInput.h"
#include "SystemRuntimeService.h"
#include "VehicleVoltageService.h"
#include "ChargeRelayDriver.h"
#include "ChargeControlService.h"
#include "DisplayPowerManager.h"
#include "OtaService.h"
#include "DisplayConfig.h"

namespace
{
    LGFX_SVEMS display;
    TFTRenderTarget tftTarget(display);
}

extern "C" bool verifyRollbackLater(void)
{
    return true;
}

DisplayModel::Model displayModel;
SerialRenderTarget serialRenderTarget(Serial);
DisplayRenderer::Renderer displayRenderer;

LGFX_SVEMS lcd;

void TestTFT();

void setup()
{
    Serial.begin(MODBUS_BAUDRATE);
    delay(BOOT_DELAY_MS);
    Logger::Begin();

    if (!SVEMS::Service::SystemRuntimeService::Begin())
    {
        Logger::Error(
            "SYSTEM",
            "Runtime Init Failed"
        );
    }

    pinMode(6, OUTPUT);
    digitalWrite(6, LOW);
    delay(10);
    digitalWrite(6, HIGH);
    delay(200);

    Wire.begin(8, 9);

    Logger::Info("I2C", "Scanning...");

    uint8_t foundCount = 0;

    for (uint8_t address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);

        const uint8_t error = Wire.endTransmission();

        if (error == 0)
        {
            char message[24];

            snprintf(
                message,
                sizeof(message),
                "Found 0x%02X",
                address);

            Logger::Info("I2C", message);

            ++foundCount;
        }
    }

    char result[24];

    snprintf(
        result,
        sizeof(result),
        "Scan complete: %u",
        foundCount);

    Logger::Info("I2C", result);
    StatusLED::Begin();

    RS485::Begin();
    ModbusRTU::Begin();
    DeviceManager::Begin();
    Display::Begin();
    DisplayPowerManager::Begin();
    lcd.setBrightness(
        DisplayPowerManager::GetBrightness()
    );
    SVEMS::Service::WiFiService::Begin();
    SVEMS::Service::NtpService::Begin();
    TestTFT();
    VehicleInput::Begin();
    SVEMS::Vehicle::VehicleVoltageService::Begin();
    SVEMS::Vehicle::ChargeRelayDriver::Begin();
    SVEMS::Vehicle::ChargeControlService::Begin();
    delay(2000);

    if constexpr (!ENABLE_BMS_SERVICE)
    {
        Tests::RunDisplayTests();
        Tests::RunDisplayThemeTests();
        Tests::RunDisplayModelTests();
        Tests::RunDisplayRendererTests(tftTarget);

        Serial.println();
        Serial.println("SVEMS Display Test");
    }

    if (!serialRenderTarget.Begin())
    {
        Serial.println("SerialRenderTarget Begin failed");
        return;
    }

    if (!displayRenderer.Begin(serialRenderTarget))
    {
        Serial.println("DisplayRenderer Begin failed");
        return;
    }

    Serial.println("DisplayRenderer Ready");

    Logger::Info(
        "SYSTEM",
        SVEMS_DEVICE_ID);

    pinMode(PIN_HEART_LED, OUTPUT);
    digitalWrite(PIN_HEART_LED, LOW);
}

void loop()
{
    StatusLED::Task();
    
    Scheduler::Run();

    //---------------------------------------------------------
    // OTA Firmware Confirmation
    //---------------------------------------------------------

    static bool otaConfirmChecked =
        false;

    if (
        !otaConfirmChecked &&
        millis() >= 30000U
    )
    {
        otaConfirmChecked =
            true;

        if (
            SVEMS::Service::OtaService::
                IsPendingVerification()
        )
        {
            if (
                SVEMS::Service::WiFiService::
                    IsConnected()
            )
            {
                SVEMS::Service::OtaService::
                    ConfirmRunningFirmware();
            }
            else
            {
                Logger::Warning(
                    "OTA",
                    "Firmware Confirm Skipped"
                );
            }
        }
    }


    //---------------------------------------------------------
    // PWM Backlight
    //---------------------------------------------------------

   static uint8_t lastBrightness =
        0xFFU;

    const uint8_t brightness =
        DisplayPowerManager::GetBrightness();

    if (brightness != lastBrightness)
    {
        lastBrightness =
            brightness;

        lcd.setBrightness(
            brightness
        );
    }
}

void TestTFT()
{
    Serial.println();
    Serial.println("========== TFT TEST ==========");

    lcd.init();

    // 320 x 240 가로 방향
    lcd.setRotation(DISPLAY_ROTATION);
    lcd.setBrightness(180);
    lcd.fillScreen(TFT_BLACK);

    lcd.fillRect(11, 10, 67, 40, TFT_RED);
    lcd.fillRect(88, 10, 67, 40, TFT_GREEN);
    lcd.fillRect(165, 10, 67, 40, TFT_BLUE);
    lcd.fillRect(242, 10, 67, 40, TFT_WHITE);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(2);
    // lcd.setFont(&fonts::efontKR_16);
    lcd.setCursor(100, 100);
    lcd.println("Hello SVEMS");
    lcd.setCursor(98, 130);
    lcd.printf("Ver : %s", SVEMS_VERSION_STRING);
    // lcd.setCursor(0, 160);
    // lcd.printf("ABCDEFGHIJKLMNOPQRSTUVWXYZ"); //fontSize(2) 12 x 14
    // lcd.setCursor(0, 176);
    // lcd.printf("abcdefghijklmnopqrstuvwxyz");
    // lcd.setTextSize(1);
    // lcd.setCursor(0, 194);
    // lcd.printf("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"); //fontSize(1) 6 x 7
    lcd.setTextSize(1);
    lcd.setCursor(110,220);
    lcd.printf("ykstudio & ChatGPT");

    Serial.print("LCD width  : ");

    Serial.println(lcd.width());

    Serial.print("LCD height : ");

    Serial.println(lcd.height());

    Serial.println("========== TFT READY ==========");
}


// 20260923 test

// #include <Arduino.h>

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(1, INPUT);
// }

// void loop()
// {
//     delay(1000);
// }

// #include <Arduino.h>
// #include "driver/twai.h"

// constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_42; // 물리적으로 CTX와 연결 안 함
// constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_39;

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println();
//     Serial.println("=== TWAI LISTEN ONLY TEST ===");

//     twai_general_config_t g_config =
//         TWAI_GENERAL_CONFIG_DEFAULT(
//             CAN_TX_PIN,
//             CAN_RX_PIN,
//             TWAI_MODE_LISTEN_ONLY
//         );

//     twai_timing_config_t t_config =
//         TWAI_TIMING_CONFIG_500KBITS();

//     twai_filter_config_t f_config =
//         TWAI_FILTER_CONFIG_ACCEPT_ALL();

//     esp_err_t result =
//         twai_driver_install(&g_config, &t_config, &f_config);

//     if (result != ESP_OK)
//     {
//         Serial.printf("TWAI install failed: %d\n", result);
//         return;
//     }

//     result = twai_start();

//     if (result != ESP_OK)
//     {
//         Serial.printf("TWAI start failed: %d\n", result);
//         return;
//     }

//     Serial.println("TWAI started: Listen Only / 500 kbps");
// }

// void loop()
// {
//     twai_message_t message;

//     if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
//     {
//         Serial.printf(
//             "ID=%03lX DLC=%d DATA=",
//             message.identifier,
//             message.data_length_code
//         );

//         for (int i = 0; i < message.data_length_code; i++)
//         {
//             Serial.printf("%02X ", message.data[i]);
//         }

//         Serial.println();
//     }
// }