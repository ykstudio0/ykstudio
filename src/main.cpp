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
  RS485::Begin();
  ModbusRTU::Begin();
  DeviceManager::Begin();
  delay(1000);
}

void loop()
{
  DeviceManager::Update();
  
  delay(1);
}
