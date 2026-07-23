//-------------------------------------------------------------
// File : main.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-01
// Project : SVEMS
// Version : 0.1.6
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

DisplayModel::Model displayModel;
SerialRenderTarget serialRenderTarget(Serial);
DisplayRenderer::Renderer displayRenderer;

void setup()
{
  Serial.begin(MODBUS_BAUDRATE);
  delay(BOOT_DELAY_MS);
  Logger::Begin();
  RS485::Begin();
  ModbusRTU::Begin();
  DeviceManager::Begin();
  Display::Begin();
  delay(1000);
  Tests::RunDisplayTests();
  Tests::RunDisplayThemeTests();
  Tests::RunDisplayModelTests();
  Tests::RunDisplayRendererTests();

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
  // displayRenderer.RenderPage(
  //   DisplayPages::Page::Overview,
  //   displayModel);

  delay(5000);
  // DeviceManager::Update();
  // delay(1);
}
