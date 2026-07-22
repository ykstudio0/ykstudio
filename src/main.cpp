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
#include "Tests.h"

DisplayModel::Model displayModel;

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
}

void loop()
{
  DeviceManager::Update();
  delay(1);
}
