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

// ===========================
// TFT SPI Configuration
// ===========================
constexpr uint8_t PIN_TFT_CS   = 10;
constexpr uint8_t PIN_TFT_MOSI = 11;
constexpr uint8_t PIN_TFT_SCK  = 12;
constexpr uint8_t PIN_TFT_MISO = 13;
constexpr uint8_t PIN_TFT_DC   = 14;
constexpr uint8_t PIN_TFT_RST  = 15;
constexpr uint8_t PIN_TFT_BL   = 21;

// ===========================
// Capacitive Touch Configuration
// ===========================
constexpr uint8_t PIN_CTP_RST  = 6;
constexpr uint8_t PIN_CTP_INT  = 7;

// ===========================
// SD Card Configuration
// ===========================
constexpr uint8_t PIN_SD_CS = 5;

// ===========================
// BMS Sniffer
// ===========================
constexpr uint8_t PIN_BMS_RX = 4;
// constexpr uint8_t PIN_BMS_TX = 3;

#endif
