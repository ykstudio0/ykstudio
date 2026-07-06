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
  Logger::Info("SYSTEM","SVEMS v0.1.3");
  Logger::Info("LOGGER", "Logger OK");
  RS485::Begin();
  Logger::Info("RS485","RS485 Ready");

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
  Serial.printf("CRC = %04X\r\n", crc);

  ModbusRTU::Begin();
  delay(1000);

  ModbusRTU::ReadHoldingRegisters(
    0x01,
    0x3100,
    2);
}

void loop()
{
  // StatusLED::Task();

  // static uint32_t lastTick = 0;
  // static uint8_t state = 0;

  // if (millis() - lastTick >= 2000)
  // {
  //   lastTick = millis();

  //   switch(state)
  //   {
  //     case 0:
  //       StatusLED::SetState(LedState::Boot);
  //       Serial.println("Boot");
  //       break;

  //     case 1:
  //       StatusLED::SetState(LedState::Ready);
  //       Serial.println("Ready");
  //       break;

  //     case 2:
  //       StatusLED::SetState(LedState::Charging);
  //       Serial.println("Charging");
  //       break;

  //     case 3:
  //       StatusLED::SetState(LedState::WiFiConnecting);
  //       Serial.println("WiFiConnecting");
  //       break;

  //     case 4:
  //       StatusLED::SetState(LedState::RS485);
  //       Serial.println("RS485");
  //       break;

  //     case 5:
  //       StatusLED::SetState(LedState::Warning);
  //       Serial.println("Warning");
  //       break;

  //     case 6:
  //       StatusLED::SetState(LedState::Error);
  //       Serial.println("Error");
  //       break;
  //   }

  //   state++;

  //   if(state > 6)
  //   {
  //     state = 0;
  //   }
  // }
}
