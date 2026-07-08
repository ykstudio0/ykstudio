//-----------------------------------------------------------
// File    : Pins.h
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-08
//-----------------------------------------------------------

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// ===========================
// STATUS LED Configuration
// ===========================
constexpr uint8_t PIN_STATUS_LED = 38;

// ===========================
// RS485 Configuration
// ===========================
constexpr uint8_t PIN_RS485_TX = 17;
constexpr uint8_t PIN_RS485_RX = 18;
constexpr uint8_t PIN_RS485_DE = 16;

// ===========================
// I2C Configuration
// ===========================
constexpr uint8_t PIN_I2C_SDA = 8;
constexpr uint8_t PIN_I2C_SCL = 9;

#endif
