#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
public:
    static void Begin();
    static void Info(const String& msg);
    static void Warn(const String& msg);
    static void Error(const String& msg);
    static void Debug(const String& msg);
};

#endif