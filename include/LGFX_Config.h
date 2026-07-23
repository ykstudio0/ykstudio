//-------------------------------------------------------------
// File : LGFX_Config.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-23
// Project : SVEMS
// Version : 0.3.3
// Description : LovyanGFX configuration for ILI9341V
//-------------------------------------------------------------

#ifndef LGFX_CONFIG_H
#define LGFX_CONFIG_H

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

#include "Pins.h"

class LGFX_SVEMS : public lgfx::LGFX_Device
{
private:
    lgfx::Panel_ILI9341 m_panel;
    lgfx::Bus_SPI       m_bus;
    lgfx::Light_PWM     m_light;

public:
    LGFX_SVEMS()
    {
        // SPI bus configuration
        {
            auto config = m_bus.config();

            config.spi_host    = SPI2_HOST;
            config.spi_mode    = 0;
            config.freq_write  = 40000000;
            config.freq_read   = 16000000;
            config.spi_3wire   = false;
            config.use_lock    = true;
            config.dma_channel = SPI_DMA_CH_AUTO;

            config.pin_sclk    = PIN_TFT_SCK;
            config.pin_mosi    = PIN_TFT_MOSI;
            config.pin_miso    = PIN_TFT_MISO;
            config.pin_dc      = PIN_TFT_DC;

            m_bus.config(config);

            m_panel.setBus(&m_bus);
        }

        // ILI9341 panel configuration
        {
            auto config = m_panel.config();

            config.pin_cs          = PIN_TFT_CS;
            config.pin_rst         = PIN_TFT_RST;
            config.pin_busy        = -1;

            config.memory_width    = 240;
            config.memory_height   = 320;
            config.panel_width     = 240;
            config.panel_height    = 320;

            config.offset_x        = 0;
            config.offset_y        = 0;
            config.offset_rotation = 0;
            
            config.readable        = true;
            config.invert          = true;
            config.rgb_order       = false;
            config.dlen_16bit      = false;
            
            config.bus_shared      = true;

            m_panel.config(config);
        }

        // Backlight PWM configuration
        {
            auto config = m_light.config();

            config.pin_bl      = PIN_TFT_BL;
            config.invert      = false;
            config.freq        = 12000;
            config.pwm_channel = 7;

            m_light.config(config);
            m_panel.setLight(&m_light);
        }

        setPanel(&m_panel);
    }
};

#endif