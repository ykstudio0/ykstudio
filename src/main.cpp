constexpr const char* SVEMS_VERSION = "0.1.2";

#include <Arduino.h>
#include "StatusLED.h"

void setup()
{
  Serial.begin(115200);
  StatusLED::Begin();

  Serial.println();
  Serial.println("SVEMS v0.1.2");
}

void loop()
{
  StatusLED::Task();

  static uint32_t lastTick = 0;
  static uint8_t state = 0;

  if (millis() - lastTick >= 2000)
  {
    lastTick = millis();

    switch(state)
    {
      case 0:
        StatusLED::SetState(LedState::Boot);
        Serial.println("Boot");
        break;

      case 1:
        StatusLED::SetState(LedState::Ready);
        Serial.println("Ready");
        break;

      case 2:
        StatusLED::SetState(LedState::Charging);
        Serial.println("Charging");
        break;

      case 3:
        StatusLED::SetState(LedState::WiFiConnecting);
        Serial.println("WiFiConnecting");
        break;

      case 4:
        StatusLED::SetState(LedState::RS485);
        Serial.println("RS485");
        break;

      case 5:
        StatusLED::SetState(LedState::Warning);
        Serial.println("Warning");
        break;

      case 6:
        StatusLED::SetState(LedState::Error);
        Serial.println("Error");
        break;
    }

    state++;

    if(state > 6)
    {
      state = 0;
    }
  }
}
