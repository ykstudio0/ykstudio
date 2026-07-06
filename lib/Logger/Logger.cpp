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

void Logger::Hex(const String& tag, const uint8_t* data, size_t length)
{
    Serial.print("[");
    Serial.print(tag);
    Serial.print("] ");

    for (size_t i = 0; i < length; i++)
    {
        if (data[i] < 0x10)
            Serial.print("0");
        
        Serial.print(data[i], HEX);
        Serial.print(" ");
        
    }
    Serial.println();
}
