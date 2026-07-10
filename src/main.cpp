//-------------------------------------------------------------
// File : main.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-01
// Project : SVEMS
// Version : 0.1.6
// Descripton : Solar Vehicle Energy Management System(SVEMS) Main Application
//-------------------------------------------------------------

#include <Arduino.h>
#include "StatusLED.h"
#include "Logger.h"
#include "RS485.h"
#include "CRC16.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "EpeverRegisters.h"
#include "Epever.h"

void setup()
{
  Serial.begin(MODBUS_BAUDRATE);
  delay(BOOT_DELAY_MS);
  
  Logger::Begin();
  Logger::Info("SYSTEM", SVEMS_VERSION);
  Logger::Info("LOGGER","Logger OK");
  RS485::Begin();
  Logger::Info("RS485", "Ready");
  ModbusRTU::Begin();
  delay(1000);

  uint8_t frame[] =
  {
    0x01,
    0x03,
    0x31,
    0x00,
    0x00,
    0x02
  };

  
  uint16_t crc = CRC16::Calculate(frame, sizeof(frame));
  char buffer[8];
  sprintf(buffer, "%04X", crc);

  // Logger::Debug("CRC", buffer);
}

void loop()
{
  if (Epever::Update())
  {
    Logger::Info(
      "BATTERY",
      String(Epever::Data.batteryVoltage, 2) + " V");

    Logger::Info(
      "PV",
      String(Epever::Data.pvVoltage, 2) + " V");
  }
  delay(1000);
}
