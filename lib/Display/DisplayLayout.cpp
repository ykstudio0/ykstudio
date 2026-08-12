//-------------------------------------------------------------
// File : DisplayLayout.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-29
// Project : SVEMS
// Version : 0.4.7
// Description : Display Layout
//-------------------------------------------------------------

#include "DisplayLayout.h"

namespace
{
    using namespace DisplayLayout;

    const DisplayLayout::StaticRow g_overviewRows[] =
    {
        {0, "Solar"},
        {1, "Battery"},
        {2, "SOC"},
        {3, "Load"},
        {4, "Cabin"},
        {5, "Humidity"}
    };
    
    const DisplayLayout::StaticRow g_solarRows[] =
    {
        {0, "PV Voltage"},
        {1, "PV Current"},
        {2, "PV Power"},
        {3, "Stage"},
        {4, "Input"},
        {5, "Daily Eng"}
    };

    const DisplayLayout::StaticRow g_batteryRows[] =
    {
        {0, "Voltage"},
        {1, "Current"},
        {2, "Power"},
        {3, "SOC"},
        {4, "Temp"}
        // {5, "Status"}
    };

    constexpr StaticRow g_batteryDetailRows[] =
    {
        { 0U, "Cell 1",   true },
        { 1U, "Cell 2",   true },
        { 2U, "Cell 3",   true },
        { 3U, "Cell 4",   true },
        { 4U, "Delta",    true },
        { 5U, "Capacity", true }
    };

    static constexpr DisplayLayout::StaticRow g_systemDetailRows[] =
    {
        { 0U, "SOLAR OFF",  true },
        { 1U, "CHARGE OFF", true },
        { 2U, "BMS OFF",    true },
        { 3U, "LOAD OFF",   true },
        { 4U, "CTRL OFF",   true },
        { 5U, "SOC OFF",    true }
    };

    const DisplayLayout::StaticRow g_loadRows[] =
    {
        {0, "Voltage"},
        {1, "Current"},
        {2, "Power"}
    };

    const DisplayLayout::StaticRow g_temperatureRows[] =
    {
        {0, "Cabin"},
        {1, "Battery"},
        {2, "BMS"},
        {3, "Controller"}
    };

    const DisplayLayout::StaticRow g_systemRows[] =
    {
        {0, "RSSI"},
        {1, "EPEVER"},
        {2, "BMS"},
        {3, "DEVICES"},
        {4, "HEAP"},
        {5, "UPTIME"}
    };
} // anonymous namespace

const DisplayLayout::StaticRow*
    DisplayLayout::GetStaticRows(
        DisplayPages::Page page,
        uint8_t subPage,
        size_t& count)
    {
        switch(page)
        {
            case DisplayPages::Page::Overview:
                count =
                    sizeof(g_overviewRows) /
                    sizeof(g_overviewRows[0]);

                return g_overviewRows;

            case DisplayPages::Page::Solar:
                count =
                    sizeof(g_solarRows) /
                    sizeof(g_solarRows[0]);

                return g_solarRows;

            case DisplayPages::Page::Battery:
            {
                if (subPage == 1U)
                {
                    count =
                        sizeof(g_batteryDetailRows) /
                        sizeof(g_batteryDetailRows[0]);

                    return g_batteryDetailRows;
                }

                count =
                    sizeof(g_batteryRows) /
                    sizeof(g_batteryRows[0]);

                return g_batteryRows;
            }

            case DisplayPages::Page::Load:
                count =
                    sizeof(g_loadRows) /
                    sizeof(g_loadRows[0]);

                return g_loadRows;

            case DisplayPages::Page::Temperature:
                count =
                    sizeof(g_temperatureRows) /
                    sizeof(g_temperatureRows[0]);

                return g_temperatureRows;

            case DisplayPages::Page::System:
            {
                if (subPage == 1U)
                {
                    count =
                        sizeof(g_systemDetailRows) /
                        sizeof(g_systemDetailRows[0]);

                    return g_systemDetailRows;
                }

                count =
                    sizeof(g_systemRows) /
                    sizeof(g_systemRows[0]);

                return g_systemRows;
            }

            default:
                count = 0;
                return nullptr;
        }
    }