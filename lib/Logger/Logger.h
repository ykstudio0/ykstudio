//-----------------------------------------------------
//File    : Logger.h
//Project : SVEMS
//Author  : JongOh Kim + ChatGPT
//Created : 2026-07-04
//-----------------------------------------------------

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
public:
    static void Begin();
    static void Info(
        const String& tag,
        const String& message);
    static void Warning(
        const String& tag,
        const String& message);
    static void Error(
        const String& tag,
        const String& message);
    static void Debug(
        const String& tag,
        const String& message);
    static void Hex(
        const String& tag,
        const uint8_t* data,
        size_t length);

private:
    static void PrintHeader(
        const String& level,
        const String& tag = "");
};

#endif