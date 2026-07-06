constexpr const char* SVEMS_VERSION = "0.1.3";

#include <Arduino.h>
#include "StatusLED.h"
#include "Logger.h"
#include "RS485.h"
#include "CRC16.h"
#include "ModbusRTU.h"

void setup()
{
  Serial.begin(115200);
  delay(5000);
  
  Logger::Begin();
  Logger::Info("SYSTEM","SVEMS v0.1.5a");
  Logger::Info("LOGGER","Logger OK");
  RS485::Begin();
  Logger::Info("RS485", "Ready");

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
  Logger::Debug("CRC", buffer);

  ModbusRTU::Begin();
  delay(1000);

  ModbusRTU::ReadHoldingRegisters(
    0x01,
    0x3100,
    2);
}

void loop()
{

}
