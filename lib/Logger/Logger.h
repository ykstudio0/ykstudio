#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
public:
    static void Begin();
    static void Info(
        const String& module,
        const String& msg);
    static void Warn(
        const String& module,
        const String& msg);
    static void Error(
        const String& module,
        const String& msg);
    static void Debug(
        const String& module,
        const String& msg);
    static void Hex(
        const String& tag,
        const uint8_t* data,
        size_t length);
};

#endif