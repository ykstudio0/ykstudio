#include "Logger.h"

void Logger::Begin()
{
    Serial.println();
    Serial.println("========== LOGGER READY ==========");
}

void Logger::Info(const String& module, const String& msg)
{
    Serial.print("[INFO ] [");
    Serial.print(module);
    Serial.print("] ");
    Serial.println(msg);
}

void Logger::Warn(const String& module, const String& msg)
{
    Serial.print("[WARN ] [");
    Serial.print(module);
    Serial.print("] ");
    Serial.println(msg);
}

void Logger::Error(const String& module, const String& msg)
{
    Serial.print("[ERROR] [");
    Serial.print(module);
    Serial.print("] ");
    Serial.println(msg);
}

void Logger::Debug(const String& module, const String& msg)
{
    Serial.print("[DEBUG] [");
    Serial.print(module);
    Serial.print("] ");
    Serial.println(msg);
}