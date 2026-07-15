//-----------------------------------------------------------
// File    : Modbus.h
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-06
//-----------------------------------------------------------

#ifndef MODBUS_H
#define MODBUS_H

#include <Arduino.h>

constexpr uint8_t MODBUS_READ_HOLDING_REGISTERS = 0x03;
constexpr uint8_t MODBUS_READ_INPUT_REGISTERS   = 0x04;

constexpr uint8_t MODBUS_WRITE_SINGLE_REGISTER  = 0x06;
constexpr uint8_t MODBUS_WRITE_MULTI_REGISTER   = 0x10;

#endif