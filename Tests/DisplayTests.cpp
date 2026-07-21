//-------------------------------------------------------------
// File : DisplayTests.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-21
// Project : SVEMS
// Version : 0.3.0
// Description : 빌드 검증용 Test cpp
//-------------------------------------------------------------
#include <Arduino.h>

#include "DisplayLayout.h"
#include "DisplayPages.h"
#include "DisplayTypes.h"

using namespace DisplayPages;
using namespace DisplayLayout;
using namespace DisplayTypes;

// DisplayPage 검증
static_assert(
    PAGE_COUNT == 6,
    "Unexpected display page count");

static_assert(
    DEFAULT_PAGE == Page::Overview,
    "Default page must be Overview");

static_assert(
    IsValid(Page::Overview),
    "Overview page must be valid");

static_assert(
    IsValid(Page::System),
    "System page must be valid");

static_assert(
    !IsValid(Page::Count),
    "Page::Count must not be treated as a real page");



static_assert(
    DisplayLayout::IsValid(),
    "Invalid display layout");

static_assert(
    DisplayLayout::SCREEN_WIDTH == 320,
    "Unexpected screen width");

static_assert(
    DisplayLayout::GetRowY(1) >
    DisplayLayout::GetRowY(0),
    "Invalid row layout");

static_assert(
    GetDefaultDecimals(ValueType::Voltage) == 2,
    "Voltage decimals must be 2");

static_assert(
    GetDefaultDecimals(ValueType::Precent) == 0,
    "Percent decimals must be 0");

constexpr DisplayValue testValue =
    MakeValue(
        14.25f,
        ValueType::Voltage);

static_assert(
    testValue.IsValid(),
    "DisplayValue must be valid");

static_assert(
    testValue.decimals == 2,
    "Voltage must use 2 decimals");