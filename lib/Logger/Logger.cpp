//-----------------------------------------------------
//File    : Logger.cpp
//Project : SVEMS
//Author  : JongOh Kim + ChatGPT
//Created : 2026-07-04
//-----------------------------------------------------

#include "Logger.h"
#include "Config.h"
#include "Version.h"

void Logger::Begin()
{
    Serial.println();
    Serial.println("========== LOGGER READY ==========");
    Logger::Info(DEVICE_NAME, SVEMS_VERSION_STRING);
}

void Logger::Info(const String& tag, const String& message)
{
    PrintHeader("INFO", tag);
    Serial.println(message);
}

void Logger::Warning(const String& tag, const String& message)
{
    PrintHeader("WARN", tag);
    Serial.println(message);
}

void Logger::Error(const String& tag, const String& message)
{
    PrintHeader("ERROR", tag);
    Serial.println(message);
}

void Logger::Debug(const String& tag, const String& message)
{
    PrintHeader("DEBUG", tag);
    Serial.println(message);
}

void Logger::Hex(const String& tag, const uint8_t* data, size_t length)
{
    Serial.printf("HEX FUNC\r\n");
    Serial.printf("[%08lu] ", millis());

    Serial.print("[");
    Serial.print(tag);
    Serial.print("] ");

    Serial.printf("LEN = %u\n", length);

    for (size_t i = 0; i < length; i++)
    {
        if (data[i] < 0x10)
            Serial.print("0");
        
        Serial.print(data[i], HEX);
        Serial.print(" ");
        
    }
    Serial.println();
}

void Logger::PrintHeader(
    const String& level,
    const String& tag)
{
    Serial.printf("[%08lu] [%-5s] [%-8s] ", millis(), level.c_str(), tag.c_str());
}
