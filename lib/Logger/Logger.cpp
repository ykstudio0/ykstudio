#include "Logger.h"

void Logger::Begin()
{
    Serial.println();
    Serial.println("========== LOGGER READY ==========");
}

void Logger::Info(const String& msg)
{
    Serial.print("[INFO ] ");
    Serial.println(msg);
}

void Logger::Warn(const String& msg)
{
    Serial.print("[WARN ] ");
    Serial.println(msg);
}

void Logger::Error(const String& msg)
{
    Serial.print("[ERROR] ");
    Serial.println(msg);
}

void Logger::Debug(const String& msg)
{
    Serial.print("[DEBUB] ");
    Serial.println(msg);
}