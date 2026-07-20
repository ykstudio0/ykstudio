//-------------------------------------------------------------
// File : Display.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Service
//-------------------------------------------------------------

#include "Display.h"
#include "DataManager.h"
#include "Logger.h"

namespace
{
    bool Initialized = false;
}

bool Display::Begin()
{
    // LCD 도착 후 이곳에서 하드웨어 초기화
    // 예:
    // - SPI 초기화
    // - ILI9341 초기화
    // - 화면 방향 설정
    // - 밝기 설정
    // - 초기 화면 출력

    if (!InitializeLCD())
    {
        Initialized = false;

        Logger::Error("DISPLAY", "Init failed");
        return false;
    }

    Initialized = true;

    Logger::Info("DISPLAY", "Ready");

    return true;
}

bool Display::InitializeLCD()
{
    // LCD 도착 후 실제 초기화 코드 추가
    //
    // 예정 작업:
    //
    // 1. SPI 버스 설정
    // 2. ILI9341 초기화
    // 3. 화면 방향 설정
    // 4. 화면 배경 지우기
    // 5. 백라이트 설정
    //
    // LCD가 없는 현재 개발 단계에서는 성공으로 처리
    return true;
}

void Display::Update()
{
    if (!Initialized)
    {
        return;
    }

    DrawMainScreen();
}

void Display::DrawMainScreen()
{
    // LCD 도착 후 화면 출력 구현
    //
    // DataManager::Solar
    // DataManager::Battery
    // DataManager::Load
    // DataManager::Temperature
    // DataManager:: SOC
    DrawHeader();

    DrawSolarBlock();
    DrawBatteryBlock();
    DrawLoadBlock();
    DrawSocBlock();
    DrawTemperatureBlock();

    DrawFooter();
}

void Display::DrawHeader()
{
    
}

void Display::DrawSolarBlock()
{

}

void Display::DrawBatteryBlock()
{

}

void Display::DrawLoadBlock()
{

}

void Display::DrawSocBlock()
{

}

void Display::DrawTemperatureBlock()
{

}

void Display::DrawFooter()
{

}