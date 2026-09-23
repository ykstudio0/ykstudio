//-------------------------------------------------------------
// File : DisplayConfig.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-23
// Project : SVEMS
// Version : 0.8.5
// Description : Display Configuration
//-------------------------------------------------------------

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <Arduino.h>

//-----------------------------------------------------------
// Display Rotation
//-----------------------------------------------------------
//
// 0 : Portrait
// 1 : Landscape
// 2 : Portrait 180°
// 3 : Landscape 180°
//
// Current hardware orientation
//
constexpr uint8_t DISPLAY_ROTATION = 1U;

//-----------------------------------------------------------
// Touch native coordinate size
//-----------------------------------------------------------
//
// Touch controller coordinate reference:
// Rotation 3 = X 0~319, Y 0~239
//
constexpr uint16_t TOUCH_NATIVE_WIDTH  = 320U;
constexpr uint16_t TOUCH_NATIVE_HEIGHT = 240U;

#endif