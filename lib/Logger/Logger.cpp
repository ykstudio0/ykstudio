//-----------------------------------------------------
//File    : Logger.cpp
//Project : SVEMS
//Author  : JongOh Kim + ChatGPT
//Created : 2026-07-04
//-----------------------------------------------------

#include "Logger.h"
#include "Config.h"
#include "Version.h"

SemaphoreHandle_t Logger::Mutex =
    nullptr;

void Logger::Begin()
{
    if (Mutex == nullptr)
    {
        Mutex =
            xSemaphoreCreateMutex();
    }

    Serial.println();
    Serial.println("========== LOGGER READY ==========");
    Logger::Info(DEVICE_NAME, SVEMS_VERSION_STRING);
}

void Logger::PrintHeader(
    const String& level,
    const String& tag)
{
    Serial.printf("[%08lu] [%-5s] [%-8s] ", millis(), level.c_str(), tag.c_str());
}

bool Logger::Lock()
{
    if (Mutex == nullptr)
    {
        return true;
    }

    return
        xSemaphoreTake(
            Mutex,
            pdMS_TO_TICKS(50)) == pdTRUE;
}

void Logger::Unlock()
{
    if (Mutex != nullptr)
    {
        xSemaphoreGive(
            Mutex);
    }
}

void Logger::Print(
    const String& level,
    const String& tag,
    const String& message)
{
    if (!Lock())
    {
        return;
    }

    PrintHeader(
        level,
        tag);

    Serial.println(
        message);

    Unlock();
}

void Logger::Info(
    const String& tag,
    const String& message)
{
    Print(
        "INFO",
        tag,
        message);
}

void Logger::Warning(
    const String& tag,
    const String& message)
{
    Print(
        "WARN",
        tag,
        message);
}

void Logger::Error(
    const String& tag,
    const String& message)
{
    Print(
        "ERROR",
        tag,
        message);
}

void Logger::Debug(
    const String& tag,
    const String& message)
{
    Print(
        "DEBUG",
        tag,
        message);
}

void Logger::Hex(
    const String& tag,
    const uint8_t* data,
    size_t length)
{
    if (!Lock())
    {
        return;
    }

    Serial.printf(
        "[%08lu] ",
        millis());

    Serial.print("[");
    Serial.print(tag);
    Serial.print("] ");

    Serial.printf(
        "LEN = %u\n",
        length);

    for (size_t i = 0;
         i < length;
         i++)
    {
        if (data[i] < 0x10)
        {
            Serial.print("0");
        }

        Serial.print(
            data[i],
            HEX);

        Serial.print(" ");
    }

    Serial.println();
    Serial.println();

    Unlock();
}