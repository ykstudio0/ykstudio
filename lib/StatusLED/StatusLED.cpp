#include "StatusLED.h"
#include "Config.h"
#include <Adafruit_NeoPixel.h>

namespace
{
    

    Adafruit_NeoPixel pixel(
        STATUS_LED_COUNT,
        PIN_STATUS_LED,
        NEO_GRB + NEO_KHZ800);
}

LedState StatusLED::currentState = LedState::Off;

void StatusLED::Begin()
{
    pixel.begin();
    pixel.setBrightness(STATUS_LED_BRIGHTNESS);
    pixel.clear();
    pixel.show();
    SetState(LedState::Boot);
}

void StatusLED::Task()
{
    // v0.1.3부터 상태 애니메이션 구현
}

LedState StatusLED::GetState()
{
    return currentState;
}

void StatusLED::SetState(LedState state)
{
  currentState = state;

  switch(state)
  {
    case LedState::Off:
      ShowColor(0,0,0);
      break;

    case LedState::Boot:
      ShowColor(0,0,255);
      break;

    case LedState::Ready:
      ShowColor(0,255,0);
      break;

    case LedState::Charging:
      ShowColor(0,100,0);
      break;

    case LedState::WiFiConnecting:
      ShowColor(255,255,0);
      break;

    case LedState::RS485:
      ShowColor(255,0,255);
      break;

    case LedState::Warning:
      ShowColor(255,120,0);
      break;

    case LedState::Error:
      ShowColor(255,0,0);
      break;
  }
}

void StatusLED::ShowColor(uint8_t r,
                          uint8_t g,
                          uint8_t b)
{
    pixel.setPixelColor(0, pixel.Color(r,g,b));
    pixel.show();
}