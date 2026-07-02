#include <Arduino.h>
#include "SystemInfo.h"
#include <WiFi.h>

void SystemInfo::Print()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println(" SVEMS Hardware Diagnostic");
    Serial.println("========================================");

    Serial.printf("Board         : ESP32-S3 DevKitC\n");
    Serial.printf("Chip Model    : %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision : %d\n", ESP.getChipRevision());

    Serial.println();
    Serial.println("CPU");
    Serial.println("----------------------------------------");
    Serial.printf("Frequency     : %d MHz\n", ESP.getCpuFreqMHz());

    Serial.println();
    Serial.println("Memory");
    Serial.println("----------------------------------------");
    float flashMB = ESP.getFlashChipSize() /1024.0 / 1024.0;
    Serial.printf("Flash         : %.2f MB\n", flashMB);

#ifdef BOARD_HAS_PSRAM
    if (psramFound())
    {
        float psramMB = ESP.getPsramSize() /1024.0 / 1024.0;
        float psramfreeMB = ESP.getFreePsram() /1024.0 / 1024.0;
        Serial.printf("PSRAM        : %.2f MB\n", psramMB);
        //Serial.printf("PSRAM Total  : %u\n", ESP.getPsramSize());
        Serial.printf("PSRAM Free   : %.2f MB\n", psramfreeMB);
    }
    else
    {
        Serial.println("PSRAM       : Not Found");
    }
#else
    Serial.println("PSRAM       : Disabled");

#endif

    Serial.printf("Heap          : %u KB\n", ESP.getHeapSize() / 1024);
    Serial.printf("Heap Free     : %u KB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("Min Heap      : %u KB\n", ESP.getMinFreeHeap() / 1024);
    Serial.printf("SDK Version   : %s\n", ESP.getSdkVersion());

    Serial.printf("MAC Address   : %s\n", WiFi.macAddress().c_str());

    Serial.println("============================================");
}