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
    EpeverMap::Load load{};

    if (!ReadRegisters(
            EpeverRegister::EPEVER_LOAD_VOLTAGE,
            sizeof(load) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&load)))
    {
        DataManager::Load.status.updated = false;
        return false;
    }

    ApplyLoadData(load);

    return true;
}

void Epever::ApplyLoadData(
    const EpeverMap::Load& load)
{
    // Load Voltage
    DataManager::Load.voltage =
        ToVoltage(load.voltage);

    // Load Current
    DataManager::Load.current =
        ToCurrent(load.current);

    // Load Power
    const uint32_t rawPower =
        (static_cast<uint32_t>(load.powerHigh) << 16) |
        load.powerLow;

    DataManager::Load.power =
        ToPower(rawPower);

    // Communication Status
    DataManager::Load.status.updated = true;
    DataManager::Load.status.online = true;
    DataManager::Load.status.lastUpdate = millis();
}

bool Epever::DecodeLoadResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Load& load)
{
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

    auto readRegister =
        [response](size_t offset) -> uint16_t
    {
        return
            (static_cast<uint16_t>(response[offset]) << 8) |
            static_cast<uint16_t>(response[offset + 1U]);
    };

    load.voltage   = readRegister(3U);
    load.current   = readRegister(5U);
    load.powerLow  = readRegister(7U);
    load.powerHigh = readRegister(9U);

    return true;
}

bool Epever::StartAsyncLoad()
{
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::EPEVER_LOAD_VOLTAGE,
        4U
    );
}

bool Epever::ProcessAsyncLoadResponse()
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

    EpeverMap::Load load{};

    if (!DecodeLoadResponse(
            response,
            responseLength,
            load))
    {
        return false;
    }

    ApplyLoadData(load);

    Logger::Info(
        "LOAD",
        "Async Read OK"
    );

    return true;
}

void Epever::HandleAsyncLoadFailure()
{
    // 이번 통신에서 데이터 갱신 실패
    DataManager::Load.status.updated = false;

    // Modbus 실패 원인 확인
    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "Load Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
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

    ApplyTemperatureData(temp);

    return true;
}

void Epever::ApplyTemperatureData(
    const EpeverMap::Temperature& temp)
{
    //---------------------------------------------------------
    // Convert temperature values
    //---------------------------------------------------------
    DataManager::Temperature.controller =
        ToTemperature(
            static_cast<int16_t>(temp.battery));

    DataManager::Temperature.controllerBoard =
        ToTemperature(
            static_cast<int16_t>(temp.device));

    //---------------------------------------------------------
    // Update communication status
    //---------------------------------------------------------
    DataManager::Temperature
        .controllerStatus.updated = true;

    DataManager::Temperature
        .controllerStatus.online = true;

    DataManager::Temperature
        .controllerStatus.state =
            DataManager::CommunicationState::Online;

    DataManager::Temperature
        .controllerStatus.lastUpdate = millis();
}

bool Epever::DecodeTemperatureResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Temperature& temp)
{
    //---------------------------------------------------------
    // Validate response
    //---------------------------------------------------------
    if (response == nullptr)
        return false;

    // Slave(1) + Function(1) + ByteCount(1)
    // + Data(4) + CRC(2)
    if (length != 9U)
        return false;

    if (response[0] != MODBUS_SLAVE_ID)
        return false;

    if (response[1] != 0x04U)
        return false;

    if (response[2] != 0x04U)
        return false;

    //---------------------------------------------------------
    // Decode Battery Temperature (0x3110)
    //---------------------------------------------------------
    temp.battery =
        (static_cast<uint16_t>(response[3]) << 8) |
         static_cast<uint16_t>(response[4]);

    //---------------------------------------------------------
    // Decode Device Temperature (0x3111)
    //---------------------------------------------------------
    temp.device =
        (static_cast<uint16_t>(response[5]) << 8) |
         static_cast<uint16_t>(response[6]);

    return true;
}

bool Epever::StartAsyncTemperature()
{
    //---------------------------------------------------------
    // Start asynchronous Temperature read
    //
    // Register : 0x3110
    // Count    : 2 registers
    //
    // 0x3110 : Battery Temperature
    // 0x3111 : Device Temperature
    //---------------------------------------------------------
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::EPEVER_BATTERY_TEMPERATURE,
        2U
    );
}

bool Epever::ProcessAsyncTemperatureResponse()
{
    //---------------------------------------------------------
    // Copy asynchronous Modbus response
    //---------------------------------------------------------
    uint8_t response[64];
    size_t length = 0;

    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            length))
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode Temperature response
    //---------------------------------------------------------
    EpeverMap::Temperature temp{};

    if (!DecodeTemperatureResponse(
            response,
            length,
            temp))
    {
        return false;
    }

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    ApplyTemperatureData(temp);

    Logger::Info(
        "TEMPERATURE",
        "Async Read OK"
    );

    return true;
}

void Epever::HandleAsyncTemperatureFailure()
{
    //---------------------------------------------------------
    // Mark Temperature update failure
    //---------------------------------------------------------
    DataManager::Temperature
        .controllerStatus.updated = false;

    //---------------------------------------------------------
    // Diagnostic log
    //---------------------------------------------------------
    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "Temperature Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
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

    ApplySocData(soc);

    return true;
}

void Epever::ApplySocData(const EpeverMap::Soc& soc)
{
    DataManager::ControllerBattery.soc =
        static_cast<uint8_t>(soc.value);
}

bool Epever::DecodeSocResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Soc& soc)
{
    //---------------------------------------------------------
    // Validate response
    //---------------------------------------------------------
    if (response == nullptr)
        return false;

    // Slave(1) + Function(1) + ByteCount(1)
    // + Data(2) + CRC(2)
    if (length != 7U)
        return false;

    if (response[0] != MODBUS_SLAVE_ID)
        return false;

    if (response[1] != 0x04U)
        return false;

    if (response[2] != 0x02U)
        return false;

    //---------------------------------------------------------
    // Decode SOC (0x311A)
    //---------------------------------------------------------
    soc.value =
        (static_cast<uint16_t>(response[3]) << 8) |
         static_cast<uint16_t>(response[4]);

    return true;
}

bool Epever::StartAsyncSoc()
{
    //---------------------------------------------------------
    // Start asynchronous Battery SOC read
    //
    // Register : 0x311A
    // Count    : 1 register
    //---------------------------------------------------------
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::EPEVER_BATTERY_SOC,
        1U
    );
}

bool Epever::ProcessAsyncSocResponse()
{
    //---------------------------------------------------------
    // Copy asynchronous Modbus response
    //---------------------------------------------------------
    uint8_t response[64];
    size_t length = 0;

    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            length))
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode SOC response
    //---------------------------------------------------------
    EpeverMap::Soc soc{};

    if (!DecodeSocResponse(
            response,
            length,
            soc))
    {
        return false;
    }

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    ApplySocData(soc);

    Logger::Info(
        "SOC",
        "Async Read OK"
    );

    return true;
}

void Epever::HandleAsyncSocFailure()
{
    //---------------------------------------------------------
    // Diagnostic log
    //---------------------------------------------------------
    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "SOC Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
}

bool Epever::ReadEnergy()
{
    EpeverMap::Energy energy;

    if (!ReadRegisters(
            EpeverRegister::GENERATED_ENERGY_TODAY,
            sizeof(energy) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&energy)))
    {
        return false;
    }

    ApplyEnergyData(energy);

    return true;
}

void Epever::ApplyEnergyData(
    const EpeverMap::Energy& energy)
{
    //---------------------------------------------------------
    // Combine 16-bit registers into 32-bit values
    //---------------------------------------------------------
    uint32_t rawDaily =
        (static_cast<uint32_t>(energy.dailyHigh) << 16) |
        energy.dailyLow;

    uint32_t rawTotal =
        (static_cast<uint32_t>(energy.totalHigh) << 16) |
        energy.totalLow;

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    DataManager::Solar.dailyEnergy =
        ToEnergy(rawDaily);

    DataManager::Solar.totalEnergy =
        ToEnergy(rawTotal);

    //---------------------------------------------------------
    // Diagnostic logs
    //---------------------------------------------------------
    Logger::Info(
        "ENERGY",
        String("Daily = ") +
        String(DataManager::Solar.dailyEnergy, 1) +
        "Wh"
    );

    Logger::Info(
        "ENERGY",
        String("Total = ") +
        String(DataManager::Solar.totalEnergy, 1) +
        "Wh"
    );
}

bool Epever::DecodeEnergyResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::Energy& energy)
{
    //---------------------------------------------------------
    // Validate response
    //---------------------------------------------------------
    if (response == nullptr)
        return false;

    // Slave(1) + Function(1) + ByteCount(1)
    // + Data(16) + CRC(2)
    if (length != 21U)
        return false;

    if (response[0] != MODBUS_SLAVE_ID)
        return false;

    if (response[1] != 0x04U)
        return false;

    if (response[2] != 0x10U)
        return false;

    //---------------------------------------------------------
    // Decode 8 consecutive registers
    //---------------------------------------------------------
    uint16_t registers[8];

    for (uint8_t i = 0; i < 8; ++i)
    {
        const size_t offset = 3U + (i * 2U);

        registers[i] =
            (static_cast<uint16_t>(response[offset]) << 8) |
             static_cast<uint16_t>(response[offset + 1U]);
    }

    //---------------------------------------------------------
    // Map registers to Energy structure
    //---------------------------------------------------------
    energy.dailyLow  = registers[0];
    energy.dailyHigh = registers[1];

    energy.monthLow  = registers[2];
    energy.monthHigh = registers[3];

    energy.yearLow   = registers[4];
    energy.yearHigh  = registers[5];

    energy.totalLow  = registers[6];
    energy.totalHigh = registers[7];

    return true;
}

bool Epever::StartAsyncEnergy()
{
    //---------------------------------------------------------
    // Start asynchronous Energy read
    //
    // Start Register : 0x330C
    // Register Count : 8
    //
    // Daily / Monthly / Yearly / Total Energy
    //---------------------------------------------------------
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::GENERATED_ENERGY_TODAY,
        8U
    );
}

bool Epever::ProcessAsyncEnergyResponse()
{
    //---------------------------------------------------------
    // Copy asynchronous Modbus response
    //---------------------------------------------------------
    uint8_t response[64];
    size_t length = 0;

    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            length))
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode Energy response
    //---------------------------------------------------------
    EpeverMap::Energy energy{};

    if (!DecodeEnergyResponse(
            response,
            length,
            energy))
    {
        return false;
    }

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    ApplyEnergyData(energy);

    Logger::Info(
        "ENERGY",
        "Async Read OK"
    );

    return true;
}

void Epever::HandleAsyncEnergyFailure()
{
    //---------------------------------------------------------
    // Diagnostic log
    //---------------------------------------------------------
    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "Energy Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
}

bool Epever::ReadChargingStatus()
{
    EpeverMap::ChargingStatus status;

    if (!ReadRegisters(
            EpeverRegister::CHARGING_EQUIPMENT_STATUS,
            sizeof(status) / sizeof(uint16_t),
            (uint16_t*)&status))
    {
        ++DataManager::CommStats.chargeTimeoutCount;
        CountModbusError();

        DataManager::Charge.status.updated = false;

        return false;
    }

    ApplyChargingStatus(status);

    return true;
}

void Epever::ApplyChargingStatus(
    const EpeverMap::ChargingStatus& status)
{
    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "Status = 0x%04X",
        status.value);

    Logger::Info("EPEVER", buffer);

    EpeverStatusParser::ChargingStatus parsed =
        EpeverStatusParser::ParseChargingStatus(status.value);

    DataManager::Charge.stage = parsed.stage;
    DataManager::Charge.inputVoltage = parsed.inputVoltage;
    DataManager::Charge.running = parsed.running;

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
}

bool Epever::DecodeChargingStatusResponse(
    const uint8_t* response,
    size_t length,
    EpeverMap::ChargingStatus& status)
{
    //---------------------------------------------------------
    // Validate response
    //---------------------------------------------------------
    if (response == nullptr)
        return false;

    // Slave(1) + Function(1) + ByteCount(1)
    // + Data(2) + CRC(2)
    if (length != 7U)
        return false;

    if (response[0] != MODBUS_SLAVE_ID)
        return false;

    if (response[1] != 0x04U)
        return false;

    if (response[2] != 0x02U)
        return false;

    //---------------------------------------------------------
    // Decode Charging Status (16-bit)
    //---------------------------------------------------------
    status.value =
        (static_cast<uint16_t>(response[3]) << 8) |
         static_cast<uint16_t>(response[4]);

    return true;
}

bool Epever::StartAsyncChargingStatus()
{
    //---------------------------------------------------------
    // Start asynchronous Charging Status read
    //
    // Register : 0x3201
    // Count    : 1 register (16-bit)
    //---------------------------------------------------------
    return ModbusRTU::StartReadInputRegisters(
        MODBUS_SLAVE_ID,
        EpeverRegister::CHARGING_EQUIPMENT_STATUS,
        1U
    );
}

bool Epever::ProcessAsyncChargingStatusResponse()
{
    //---------------------------------------------------------
    // Copy asynchronous Modbus response
    //---------------------------------------------------------
    uint8_t response[64];
    size_t length = 0;

    if (!ModbusRTU::CopyAsyncResponse(
            response,
            sizeof(response),
            length))
    {
        return false;
    }

    //---------------------------------------------------------
    // Decode Charging Status
    //---------------------------------------------------------
    EpeverMap::ChargingStatus status{};

    if (!DecodeChargingStatusResponse(
            response,
            length,
            status))
    {
        return false;
    }

    //---------------------------------------------------------
    // Update DataManager
    //---------------------------------------------------------
    ApplyChargingStatus(status);

    Logger::Info(
        "CHARGING",
        "Async Read OK"
    );

    return true;
}

void Epever::HandleAsyncChargingStatusFailure()
{
    //---------------------------------------------------------
    // Update communication error statistics
    //---------------------------------------------------------
    ++DataManager::CommStats.chargeTimeoutCount;

    CountModbusError();

    //---------------------------------------------------------
    // Mark Charging Status update failure
    //---------------------------------------------------------
    DataManager::Charge.status.updated = false;

    //---------------------------------------------------------
    // Diagnostic log
    //---------------------------------------------------------
    const auto reason =
        ModbusRTU::GetLastErrorReason();

    Logger::Warning(
        "EPEVER ASYNC",
        "Charging Status Failed, Reason=" +
        String(static_cast<uint8_t>(reason))
    );
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