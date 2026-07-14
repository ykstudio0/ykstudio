//-------------------------------------------------------------
// File : main.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-01
// Project : SVEMS
// Version : 0.1.6
// Descripton : Solar Vehicle Energy Management System(SVEMS) Main Application
//-------------------------------------------------------------

#include <Arduino.h>
#include "Version.h"
#include "StatusLED.h"
#include "Logger.h"
#include "RS485.h"
#include "CRC16.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "EpeverRegisters.h"
#include "Epever.h"
#include "DeviceManager.h"

void setup()
{
  Serial.begin(MODBUS_BAUDRATE);
  delay(BOOT_DELAY_MS);
  
  Logger::Begin();
  Logger::Info(DEVICE_NAME, SVEMS_VERSION_STRING);
  Logger::Info("LOGGER","Logger OK");
  RS485::Begin();
  Logger::Info("RS485", "Ready");
  ModbusRTU::Begin();
  DeviceManager::Begin();
  // Logger::Info("DEVICES", "Ready");
  delay(1000);
}

void loop()
{
  if (DeviceManager::Update())
  {
    Logger::Info(
      "PV",
      String(Epever::Data.pvVoltage, 2) + " V");

    Logger::Info(
      "PV",
      String(Epever::Data.pvCurrent, 2) + " A");

    Logger::Info(
      "PV",
      String(Epever::Data.pvPower, 1) + " W");

    Logger::Info(
      "BATTERY",
      String(Epever::Data.batteryVoltage, 2) + " V");
    Logger::Info(
      "BATTERY",
      String(Epever::Data.batteryCurrent, 2) + " A");
  }
  delay(1000);
}
