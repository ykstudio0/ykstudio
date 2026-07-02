#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

enum class LedState
{
  Off,
  Boot,
  Ready,
  Charging,
  WiFiConnecting,
  RS485,
  Warning,
  Error
};

class StatusLED
{
public:
  static void Begin();
  static void Task();
  static void SetState(LedState state);
  static LedState GetState();

private:
  static LedState currentState;
  static void ShowColor(uint8_t r,
                        uint8_t g,
                        uint8_t b);
};

#endif