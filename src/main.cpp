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
    RS485::Begin();
    ModbusRTU::Begin();
    DeviceManager::Begin();
    Display::Begin();
    Scheduler::Begin();
    // delay(1000);
    TestTFT();
    delay(2000);
    Tests::RunDisplayTests();
    Tests::RunDisplayThemeTests();
    Tests::RunDisplayModelTests();
    Tests::RunDisplayRendererTests(tftTarget);

    Serial.println();
    Serial.println("SVEMS Display Test");

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
    lcd.setCursor(130, 130);
    lcd.printf("v0.4.7");
    lcd.setCursor(0, 160);
    lcd.printf("ABCDEFGHIJKLMNOPQRSTUVWXYZ"); //fontSize(2) 12 x 14
    lcd.setCursor(0, 176);
    lcd.printf("abcdefghijklmnopqrstuvwxyz");
    lcd.setTextSize(1);
    lcd.setCursor(0, 194);
    lcd.printf("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"); //fontSize(1) 6 x 7
    lcd.setTextSize(1);
    lcd.setCursor(110,220);
    lcd.printf("ykstudio & ChatGPT");

    Serial.print("LCD width  : ");

    Serial.println(lcd.width());

    Serial.print("LCD height : ");

    Serial.println(lcd.height());

    Serial.println("========== TFT READY ==========");
}