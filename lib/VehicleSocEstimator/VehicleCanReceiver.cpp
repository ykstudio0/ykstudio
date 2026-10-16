//-------------------------------------------------------------
// File : VehicleCanReceiver.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Read-only vehicle CAN receiver
//-------------------------------------------------------------

#include "VehicleCanReceiver.h"

//-------------------------------------------------------------
// CAN configuration
//-------------------------------------------------------------
static constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_41;
static constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_39;

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
bool VehicleCanReceiver::Begin()
{
    if (m_ready)
    {
        return true;
    }

    twai_general_config_t generalConfig =
        TWAI_GENERAL_CONFIG_DEFAULT(
            CAN_TX_PIN,
            CAN_RX_PIN,
            TWAI_MODE_LISTEN_ONLY);

    generalConfig.rx_queue_len = 64;

    twai_timing_config_t timingConfig =
        TWAI_TIMING_CONFIG_500KBITS();

    // //---------------------------------------------------------
    // // Hardware acceptance filter
    // // Accept standard CAN IDs 0x7E0 - 0x7EF.
    // //---------------------------------------------------------
    // twai_filter_config_t filterConfig = {};

    // filterConfig.acceptance_code =
    //     (0x7E0UL << 21);

    // filterConfig.acceptance_mask =
    //     ~((0x7F0UL) << 21);

    // filterConfig.single_filter = true;

    //---------------------------------------------------------
    // Hardware acceptance filter
    // Diagnostic Mode: Accept all CAN IDs.
    //---------------------------------------------------------
    twai_filter_config_t filterConfig =
        TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t result = twai_driver_install(
        &generalConfig,
        &timingConfig,
        &filterConfig);

    if (result != ESP_OK)
    {
        return false;
    }

    result = twai_start();

    if (result != ESP_OK)
    {
        twai_driver_uninstall();
        return false;
    }

    m_ready = true;

    return true;
}

//-------------------------------------------------------------
// Receive
// Description:
//   Drain unrelated CAN frames and return only
//   diagnostic request/response frames.
//-------------------------------------------------------------
bool VehicleCanReceiver::Receive(
    twai_message_t& message)
{
    if (!m_ready)
    {
        return false;
    }

    //---------------------------------------------------------
    // Process queued frames until a diagnostic frame
    // is found or the receive queue becomes empty.
    //---------------------------------------------------------
    while (twai_receive(&message, 0) == ESP_OK)
    {
        // Ignore extended and remote frames.
        if (message.extd || message.rtr)
        {
            continue;
        }

        // Ignore invalid data lengths.
        if (message.data_length_code > 8)
        {
            continue;
        }

        // Accept diagnostic frames only.
        if (message.identifier == 0x7E0 ||
            message.identifier == 0x7E8)
        {
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------
// IsReady
//-------------------------------------------------------------
bool VehicleCanReceiver::IsReady() const
{
    return m_ready;
}

//-------------------------------------------------------------
// ReceiveRaw
// Description:
//   Receive any standard CAN data frame.
//   Diagnostic use only.
//-------------------------------------------------------------
bool VehicleCanReceiver::ReceiveRaw(
    twai_message_t& message)
{
    if (!m_ready)
    {
        return false;
    }

    while (twai_receive(&message, 0) == ESP_OK)
    {
        if (message.extd || message.rtr)
        {
            continue;
        }

        if (message.data_length_code > 8)
        {
            continue;
        }

        return true;
    }

    return false;
}