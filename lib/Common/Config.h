//-------------------------------------------------------------
// File : Config.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Descripton : 
//-------------------------------------------------------------

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//-------------------------------
// Project
//-------------------------------
constexpr char SVEMS_VERSION[] = "0.1.9";
constexpr char DEVICE_NAME[] = "SVEMS";
constexpr uint16_t BOOT_DELAY_MS = 5000;

//-------------------------------
// Modbus Configuration
//-------------------------------
constexpr uint8_t  MODBUS_SLAVE_ID = 1;
constexpr uint32_t MODBUS_BAUDRATE = 115200;
constexpr uint16_t MODBUS_TIMEOUT = 500;
constexpr uint8_t MODBUS_RETRY = 2;

//-------------------------------
// Logger
//-------------------------------
constexpr bool LOGGER_ENABLE_DEBUG = true;

//-------------------------------
// STATUS LED Configuration
//-------------------------------
constexpr uint8_t STATUS_LED_COUNT = 1;
constexpr uint8_t STATUS_LED_BRIGHTNESS = 30;

#endif