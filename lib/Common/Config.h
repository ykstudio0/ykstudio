//-------------------------------------------------------------
// File : Config.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Description : 
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
constexpr uint32_t MODBUS_BAUDRATE = 9600;
constexpr uint16_t MODBUS_TIMEOUT = 200;
constexpr uint8_t MODBUS_RETRY = 0U;

//-------------------------------
// Logger
//-------------------------------
constexpr bool LOGGER_ENABLE_DEBUG = true;

//-------------------------------
// STATUS LED Configuration
//-------------------------------
constexpr uint8_t STATUS_LED_COUNT = 1;
constexpr uint8_t STATUS_LED_BRIGHTNESS = 30;

//-------------------------------
// 테스트 기기
//-------------------------------
constexpr bool ENABLE_EPEVER_POLLING = true;   
constexpr bool ENABLE_TOUCH = true;
constexpr bool ENABLE_WIFI = true;
constexpr bool ENABLE_NTP = true;
constexpr bool ENABLE_DISPLAY = true;

//-------------------------------
// BMS Configuration
//-------------------------------
constexpr bool ENABLE_BMS_SERVICE = false;
// BMS 로그 관리
constexpr bool ENABLE_BMS_TRACE = true;  
   
#endif