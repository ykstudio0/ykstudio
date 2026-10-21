//-------------------------------------------------------------
// File : Epever.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Description : Epever 장치
//-------------------------------------------------------------

#include "Epever.h"
#include "Logger.h"
#include "EpeverRegisters.h"
#include "Modbus.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "Units.h"
#include "EpeverMaps.h"
#include "DataManager.h"
#include "EpeverStatusParser.h"

EpeverData Epever::Data;

static void CountModbusError()
{
    switch (ModbusRTU::GetLastErrorReason())
    {
        case ModbusRTU::ErrorReason::NoResponse:
            ++DataManager::CommStats.modbusNoResponseCount;
            break;

        case ModbusRTU::ErrorReason::FrameTooShort:
            ++DataManager::CommStats.modbusFrameTooShortCount;
            break;

        case ModbusRTU::ErrorReason::InvalidSlave:
            ++DataManager::CommStats.modbusInvalidSlaveCount;
            break;

        case ModbusRTU::ErrorReason::InvalidFunction:
            ++DataManager::CommStats.modbusInvalidFunctionCount;
            break;

        case ModbusRTU::ErrorReason::InvalidLength:
            ++DataManager::CommStats.modbusInvalidLengthCount;
            break;

        case ModbusRTU::ErrorReason::CrcError:
            ++DataManager::CommStats.modbusCrcErrorCount;
            break;

        case ModbusRTU::ErrorReason::Exception:
            ++DataManager::CommStats.modbusExceptionCount;
            break;

        case ModbusRTU::ErrorReason::InvalidByteCount:
            ++DataManager::CommStats.modbusInvalidByteCount;
            break;

        case ModbusRTU::ErrorReason::None:
        default:
            break;
    }
}

bool Epever::Begin()
{
    return true;
}

bool Epever::Update()
{
    bool ok = true;

    return ok;
}

//-------------------------------------------------------------
// Epever - Read Solar
//-------------------------------------------------------------
bool Epever::ReadSolar()
{
    EpeverMap::Solar solar;

    if (!ReadRegisters(
        EpeverRegister::PV_ARRAY_VOLTAGE,
        sizeof(solar) / sizeof(uint16_t),
        reinterpret_cast<uint16_t*>(&solar)))
    {
        ++DataManager::CommStats.solarTimeoutCount;
        CountModbusError();

        DataManager::Solar.status.updated = false;
        return false;
    }

    ApplySolarData(solar);

    return true;
}

//-------------------------------------------------------------
// Epever - Decode Solar Response
//-------------------------------------------------------------
bool Epever::DecodeSolarResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Solar& solar)
{
    constexpr size_t REGISTER_COUNT =
        sizeof(EpeverMap::Solar) / sizeof(uint16_t);

    constexpr size_t EXPECTED_LENGTH =
        5U + REGISTER_COUNT * 2U;

    //---------------------------------------------------------
    // Validate response
    //---------------------------------------------------------
    if (response == nullptr ||
        length != EXPECTED_LENGTH)
    {
        return false;
    }

    if (response[0] != MODBUS_SLAVE_ID ||
        response[1] != MODBUS_READ_INPUT_REGISTERS ||
        response[2] != REGISTER_COUNT * 2U)
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode Modbus registers (Big Endian)
    //---------------------------------------------------------
    solar.voltage =
        (static_cast<uint16_t>(response[3]) << 8) |
        response[4];

    solar.current =
        (static_cast<uint16_t>(response[5]) << 8) |
        response[6];

    solar.powerLow =
        (static_cast<uint16_t>(response[7]) << 8) |
        response[8];

    solar.powerHigh =
        (static_cast<uint16_t>(response[9]) << 8) |
        response[10];

    return true;
}

//-------------------------------------------------------------
// Epever - Process Asynchronous Solar Response
//-------------------------------------------------------------
bool Epever::ProcessAsyncSolarResponse()
{
    uint8_t response[64] = {};
    size_t responseLength = 0U;

    //---------------------------------------------------------
    // Copy validated Modbus response
    //---------------------------------------------------------
    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            responseLength))
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode Solar registers
    //---------------------------------------------------------
    EpeverMap::Solar solar = {};

    if (!DecodeSolarResponse(
            response,
            responseLength,
            solar))
    {
        return false;
    }

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    ApplySolarData(solar);

    return true;
}

//-------------------------------------------------------------
// Epever - Apply Solar Data
// Description:
//   Convert decoded Solar registers and update DataManager.
//-------------------------------------------------------------
void Epever::ApplySolarData(
    const EpeverMap::Solar& solar)
{
    // PV Voltage
    DataManager::Solar.voltage =
        ToVoltage(solar.voltage);

    // PV Current
    DataManager::Solar.current =
        ToCurrent(solar.current);

    // PV Power (32bit)
    uint32_t rawPower =
        (static_cast<uint32_t>(solar.powerHigh) << 16) |
        solar.powerLow;

    DataManager::Solar.power = ToPower(rawPower);

    // Solar Status
    DataManager::Solar.status.updated = true;
    DataManager::Solar.status.online = true;
    DataManager::Solar.status.state =
        DataManager::CommunicationState::Online;
    DataManager::Solar.status.lastUpdate = millis();

    Logger::Info("SOLAR", "Read OK");
}

//-------------------------------------------------------------
// Epever - Start Asynchronous Solar Polling
//-------------------------------------------------------------
bool Epever::StartAsyncSolar()
{
    constexpr uint16_t REGISTER_COUNT =
        sizeof(EpeverMap::Solar) / sizeof(uint16_t);

    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::PV_ARRAY_VOLTAGE,
        REGISTER_COUNT
    );
}

//-------------------------------------------------------------
// Epever - Handle Asynchronous Solar Failure
//-------------------------------------------------------------
void Epever::HandleAsyncSolarFailure()
{
    ++DataManager::CommStats.solarTimeoutCount;

    CountModbusError();

    DataManager::Solar.status.updated = false;

    Logger::Warning(
        "EPEVER ASYNC",
        "Solar communication failed"
    );
}

bool Epever::DecodeBatteryResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Battery& battery)
{
    //-----------------------------------------------------
    // Battery: 4 registers
    // Modbus RTU: Slave + Function + ByteCount
    //             + Data(8) + CRC(2) = 13 bytes
    //-----------------------------------------------------
    constexpr size_t REGISTER_COUNT = 4U;
    constexpr size_t EXPECTED_LENGTH =
        5U + REGISTER_COUNT * 2U;

    if (response == nullptr ||
        length != EXPECTED_LENGTH)
    {
        return false;
    }

    if (response[0] != MODBUS_SLAVE_ID ||
        response[1] != 0x04U ||
        response[2] != REGISTER_COUNT * 2U)
    {
        return false;
    }

    //-----------------------------------------------------
    // Modbus register data: High byte first
    //-----------------------------------------------------
    const auto readRegister =
        [response](size_t offset) -> uint16_t
    {
        return
            (static_cast<uint16_t>(
                response[offset]) << 8) |
            static_cast<uint16_t>(
                response[offset + 1U]);
    };

    battery.voltage   = readRegister(3U);
    battery.current   = readRegister(5U);
    battery.powerLow  = readRegister(7U);
    battery.powerHigh = readRegister(9U);

    return true;
}

//-------------------------------------------------------------
// Start asynchronous Battery register request
//-------------------------------------------------------------
bool Epever::StartAsyncBattery()
{
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::EPEVER_BATTERY_VOLTAGE,
        4U
    );
}

//-------------------------------------------------------------
// Process asynchronous Battery response
//-------------------------------------------------------------
bool Epever::ProcessAsyncBatteryResponse()
{
    uint8_t response[64] = {};
    size_t responseLength = 0U;

    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            responseLength))
    {
        return false;
    }

    EpeverMap::Battery battery{};

    if (!DecodeBatteryResponse(
            response,
            responseLength,
            battery))
    {
        return false;
    }

    ApplyBatteryData(battery);

    Logger::Info(
        "BATTERY",
        "Async Read OK"
    );

    return true;
}

//-------------------------------------------------------------
// Handle asynchronous Battery communication failure
//-------------------------------------------------------------
void Epever::HandleAsyncBatteryFailure()
{
    DataManager::ControllerBattery
        .status.updated = false;

    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "Battery Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
}

// Read Battery Information
bool Epever::ReadBattery()
{
    EpeverMap::Battery battery{};

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_VOLTAGE,
            sizeof(battery) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&battery)))
    {
        DataManager::ControllerBattery
            .status.updated = false;

        return false;
    }

    ApplyBatteryData(battery);

    return true;
}

void Epever::ApplyBatteryData(
    const EpeverMap::Battery& battery)
{
    DataManager::ControllerBattery.voltage =
        ToVoltage(battery.voltage);

    DataManager::ControllerBattery.current =
        ToCurrent(battery.current);

    const uint32_t rawPower =
        (static_cast<uint32_t>(
            battery.powerHigh) << 16) |
        battery.powerLow;

    DataManager::ControllerBattery.power =
        ToPower(rawPower);

    DataManager::ControllerBattery
        .status.updated = true;

    DataManager::ControllerBattery
        .status.online = true;

    DataManager::ControllerBattery
        .status.lastUpdate = millis();
}

bool Epever::ReadLoad()
{
    EpeverMap::Load load;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_LOAD_VOLTAGE,
            sizeof(load) / sizeof(uint16_t),
            (uint16_t*)&load))
    {
        DataManager::Load.status.updated = false;
        return false;
    }

    // Load Voltage
    DataManager::Load.voltage = ToVoltage(load.voltage);

    // Load Current
    DataManager::Load.current = ToCurrent(load.current);

    // Load Power
    uint32_t rawPower =
        ((uint32_t)load.powerHigh << 16) |
        load.powerLow;
    
    DataManager::Load.power = 
        ToPower(rawPower);

    DataManager::Load.status.updated = true;
    DataManager::Load.status.online = true;
    DataManager::Load.status.lastUpdate = millis();

    return true;
}

bool Epever::ReadTemperature()
{
    EpeverMap::Temperature temp;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_TEMPERATURE,
            sizeof(temp) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&temp)))
    {
        DataManager::Temperature
            .controllerStatus.updated = false;

        return false;
    }

    DataManager::Temperature.controller =
        ToTemperature(
            static_cast<int16_t>(
                temp.battery));

    DataManager::Temperature.controllerBoard =
        ToTemperature(
            static_cast<int16_t>(
                temp.device));

    DataManager::Temperature
        .controllerStatus.updated = true;

    DataManager::Temperature
        .controllerStatus.online = true;

    DataManager::Temperature
        .controllerStatus.state =
            DataManager::CommunicationState::Online;

    DataManager::Temperature
        .controllerStatus.lastUpdate = millis();

    return true;
}

bool Epever::ReadSOC()
{
    EpeverMap::Soc soc;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_SOC,
            sizeof(soc) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&soc)))
    {
        return false;
    }

    DataManager::ControllerBattery.soc =
        static_cast<uint8_t>(soc.value);

    return true;
}

bool Epever::ReadEnergy()
{
    EpeverMap::Energy energy;

    if (!ReadRegisters(
        EpeverRegister::GENERATED_ENERGY_TODAY,
        sizeof(energy) / sizeof(uint16_t),
        (uint16_t*)&energy))
    {
        return false;
    }

    uint32_t rawDaily =
        ((uint32_t)energy.dailyHigh << 16) |
        energy.dailyLow;

    uint32_t rawTotal =
        ((uint32_t)energy.totalHigh << 16) |
        energy.totalLow;

    DataManager::Solar.dailyEnergy =
        ToEnergy(
            rawDaily
        );

    DataManager::Solar.totalEnergy =
        ToEnergy(
            rawTotal
        );

    Logger::Info(
        "ENERGY",
        String("Daily = ") +
        String(
            DataManager::Solar.dailyEnergy,
            1
        ) + "Wh"
    );

    Logger::Info(
        "ENERGY",
        String("Total = ") +
        String(
            DataManager::Solar.totalEnergy,
            1
        ) + "Wh"
    );

    return true;
}

bool Epever::ReadChargingStatus()
{
    EpeverMap::ChargingStatus status;

    if (!ReadRegisters(
            EpeverRegister::CHARGING_EQUIPMENT_STATUS,
            sizeof(status) /sizeof(uint16_t),
            (uint16_t*)&status))
    {
        ++DataManager::CommStats.chargeTimeoutCount;
        CountModbusError();

        DataManager::Charge.status.updated = false;

        return false;
    }

    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "Status = 0x%04X",
        status.value);

    Logger::Info("EPEVER", buffer);

    EpeverStatusParser::ChargingStatus parsed =
        EpeverStatusParser::ParseChargingStatus(status.value);

    DataManager::Charge.stage        = parsed.stage;
    DataManager::Charge.inputVoltage = parsed.inputVoltage;
    DataManager::Charge.running      = parsed.running;
    DataManager::Charge.status.updated = true;
    DataManager::Charge.status.online = true;
    DataManager::Charge.status.state =
            DataManager::CommunicationState::Online;
    DataManager::Charge.status.lastUpdate = millis();

    snprintf(
        buffer,
        sizeof(buffer),
        "Stage = %s",
        EpeverStatusParser::ToString(parsed.stage));

    Logger::Info("EPEVER", buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "Running = %s",
        parsed.running ? "YES" : "NO");

    Logger::Info("EPEVER", buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "Input = %s",
        EpeverStatusParser::ToString(parsed.inputVoltage));

    Logger::Info("EPEVER", buffer);
    
    return true;
}

bool Epever::ReadRegisters(
    uint16_t address,
    uint16_t count,
    uint16_t* values)
{
    uint8_t rx[64];

    if (!ModbusRTU::ReadInputRegisters(
        MODBUS_SLAVE_ID,
        address,
        count,
        rx,
        sizeof(rx)))
    {
        return false;
    }


    if (rx[0] != MODBUS_SLAVE_ID)
        return false;
    
    if (rx[1] != MODBUS_READ_INPUT_REGISTERS)
        return false;
    
    if (rx[2] != count * 2)
    {
        Logger::Error(
            "MODBUS",
            "Byte Count Error");
        return false;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        values[i] =
            ((uint16_t)rx[3 + i * 2] << 8) |
            rx[4 + i * 2];
    }
    return true;
}

void Epever::ClearUpdates()
{
    DataManager::Solar.status.updated = false;
    DataManager::Battery.status.updated = false;
    DataManager::Load.status.updated = false;
    DataManager::Temperature.controllerStatus.updated = false;
    DataManager::Soc.status.updated = false;
}