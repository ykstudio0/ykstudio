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

namespace
{
    LGFX_SVEMS display;
    TFTRenderTarget tftTarget(display);
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
    
    if constexpr (ENABLE_BMS_SERVICE)
    {
        if (!SVEMS::Service::BMSService::Begin())
        {
            Logger::Error("BMS", "Init Failed");
        }
    }

    RS485::Begin();
    ModbusRTU::Begin();
    DeviceManager::Begin();
    Display::Begin();
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

    // SVEMS::Service::WiFiService::StartSetupMode();
    // delay(5000);
    // ESP.restart();
}

void loop()
{
    Scheduler::Run();
}

void TestTFT()
{
    Serial.println();
    Serial.println("========== TFT TEST ==========");

    lcd.init();

    // 320 x 240 가로 방향
    lcd.setRotation(3);

    // PWM 백라이트
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