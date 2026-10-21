//-----------------------------------------------------
// File    : ModbusRTU.cpp
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-06
//-----------------------------------------------------

#include "ModbusRTU.h"
#include "Modbus.h"
#include "Config.h"
#include "CRC16.h"
#include "RS485.h"
#include "Logger.h"

bool ModbusRTU::Ready = false;

uint32_t ModbusRTU::ConsecutiveFrameErrorCount =
    0U;

ModbusRTU::AsyncTransaction
    ModbusRTU::Transaction = {};

void ModbusRTU::ResetCommunicationState()
{
    ConsecutiveFrameErrorCount = 0U;
}

ModbusRTU::ErrorReason
    ModbusRTU::LastErrorReason =
        ModbusRTU::ErrorReason::None;

void ModbusRTU::RecordFrameError()
{
    ++ConsecutiveFrameErrorCount;
}

bool ModbusRTU::Begin()
{   
    Ready = false;

    ConsecutiveFrameErrorCount =
        0U;

    if (!RS485::IsReady())
    {
        Logger::Error(
            "MODBUS",
            "RS485 Not Ready");

        return false;
    }

    Ready = true;

    Logger::Info("MODBUS", "Ready (" + String(MODBUS_BAUDRATE) + "bps)");
    
    return true;
}

//-------------------------------------------------------------
// Asynchronous Modbus - Reset Transaction
//-------------------------------------------------------------
void ModbusRTU::ResetAsync()
{
    Transaction = AsyncTransaction{};
}

//-------------------------------------------------------------
// Asynchronous Modbus - Get Current State
//-------------------------------------------------------------
ModbusRTU::AsyncState ModbusRTU::GetAsyncState()
{
    return Transaction.state;
}

//-------------------------------------------------------------
// Asynchronous Modbus - Copy Response
//-------------------------------------------------------------
bool ModbusRTU::CopyAsyncResponse(
    uint8_t* buffer,
    size_t bufferSize,
    size_t& length)
{
    length = 0U;

    //---------------------------------------------------------
    // Validate transaction state
    //---------------------------------------------------------
    if (Transaction.state != AsyncState::Success)
    {
        return false;
    }

    //---------------------------------------------------------
    // Validate destination buffer
    //---------------------------------------------------------
    if (buffer == nullptr ||
        bufferSize < Transaction.responseLength)
    {
        return false;
    }

    //---------------------------------------------------------
    // Copy validated Modbus response
    //---------------------------------------------------------
    memcpy(
        buffer,
        Transaction.response,
        Transaction.responseLength
    );

    length = Transaction.responseLength;

    return true;
}

bool ModbusRTU::ReadInputRegisters(
    uint8_t slave,
    uint16_t address,
    uint16_t count,
    uint8_t* response,
    size_t responseSize)
{
    static uint32_t seq = 0;

    LastErrorReason =
        ErrorReason::None;

    seq++;
    Logger::Info("MODBUS", "Read #" + String(seq));

    uint8_t frame[8];

    frame[0] = slave;
    frame[1] = MODBUS_READ_INPUT_REGISTERS;
    
    frame[2] = address >> 8;
    frame[3] = address & 0xFF;

    frame[4] = count >> 8;
    frame[5] = count & 0xFF;

    uint16_t crc = CRC16::Calculate(frame, 6);

    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;

    size_t len = 0;

    for (uint8_t retry = 0;
        retry <= MODBUS_RETRY;
        retry++)
    {
        len = 0;
        memset(response, 0, responseSize);

        Logger::Hex("TX", frame, sizeof(frame));

        RS485::Send(frame, sizeof(frame));

        bool received =
            RS485::ReceiveFrame(
                response,
                len,
                responseSize,
                MODBUS_TIMEOUT);

        //-------------------------------------------------
        // Partial Frame Recovery
        //
        // 일부 byte가 수신되었지만 완전한 Frame을
        // 받지 못한 경우 동일 요청을 1회 재전송한다.
        //
        // 구형 EPEVER Tracer-BP 통신 대응
        //-------------------------------------------------

        if (!received && len > 0)
        {
            Logger::Warning(
                "MODBUS",
                "Partial Frame - Recovery Retry");

            len = 0;

            memset(
                response,
                0,
                responseSize);

            Logger::Hex(
                "TX RECOVERY",
                frame,
                sizeof(frame));

            RS485::Send(
                frame,
                sizeof(frame));

            received =
                RS485::ReceiveFrame(
                    response,
                    len,
                    responseSize,
                    MODBUS_TIMEOUT);

            if (received)
            {
                Logger::Info(
                    "MODBUS",
                    "Partial Recovery OK");
            }
            else
            {
                Logger::Warning(
                    "MODBUS",
                    "Partial Recovery Failed");
            }
        }

        //-------------------------------------------------
        // 수신 실패
        //-------------------------------------------------

        if (!received)
        {
            if (retry < MODBUS_RETRY)
            {
                Logger::Info(
                    "MODBUS",
                    "Retry #" +
                    String(retry + 1));
            }

            continue;
        }

        break;
    }

    if (len == 0)
    {
        LastErrorReason =
            ErrorReason::NoResponse;

        ConsecutiveFrameErrorCount =
            0U;

        Logger::Warning(
            "MODBUS",
            "NO Response");

        return false;
    }

    Logger::Hex("RX", response, len);

    // 최소 프레임 확인
    if (len < 5)
    {
        LastErrorReason =
            ErrorReason::FrameTooShort;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Frame Too Short");

        return false;
    }

    // Slave 확인
    if (response[0] != slave)
    {
        LastErrorReason =
            ErrorReason::InvalidSlave;

        RecordFrameError();

        Logger::Error("MODBUS", "Invalid Slave");
        return false;
    }

    // Exception인지 먼저 구분
    const bool isException = 
        (response[1] & 0x80) != 0;

    size_t expectedLength = 0;

    if (isException)
    {
        expectedLength = 5;
    }
    else
    {
        if (response[1] != MODBUS_READ_INPUT_REGISTERS)
        {
            LastErrorReason =
                ErrorReason::InvalidFunction;

            RecordFrameError();

            Logger::Error("MODBUS", "Invalid Function");
            return false;
        }

        expectedLength =
            5U + static_cast<size_t>(response[2]);
    }

    if (len != expectedLength)
    {
        LastErrorReason =
            ErrorReason::InvalidLength;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Invalid Length (" +
            String(len) + "/" +
            String(expectedLength) + ")");
        
        return false;
    }

    // CRC 검사
    uint16_t crcCalc = 
        CRC16::Calculate(response, len - 2);

    uint16_t crcRecv =
        (uint16_t)response[len - 2] |
        ((uint16_t)response[len - 1] << 8);

    if (crcCalc != crcRecv)
    {
        LastErrorReason =
            ErrorReason::CrcError;

        RecordFrameError();

        char msg[48];

        sprintf(
            msg,
            "CRC Error (Recv=%04X Calc=%04X)",
            crcRecv,
            crcCalc);

        Logger::Error("MODBUS", msg);

        return false;
    }

    if (isException)
    {
        LastErrorReason =
            ErrorReason::Exception;

        RecordFrameError();

        char msg[32];

        sprintf(
            msg,
            "Exception %02X",
            response[2]);

        Logger::Error("MODBUS", msg);
        return false;
    }

    const uint8_t requestedBytes =
        static_cast<uint8_t>(count * 2U);
    
    if (response[2] != requestedBytes)
    {
        LastErrorReason =
            ErrorReason::InvalidByteCount;

        RecordFrameError();

        Logger::Error(
            "MODBUS",
            "Invalid Byte Count (" +
            String(response[2]) + "/" +
            String(requestedBytes) + ")");

        return false;
    }
    
    ConsecutiveFrameErrorCount =
        0U;
    
    LastErrorReason =
        ErrorReason::None;

    return true;
}

ModbusRTU::ErrorReason
    ModbusRTU::GetLastErrorReason()
{
    return LastErrorReason;
}

bool ModbusRTU::IsReady()
{
    return Ready;
}

bool ModbusRTU::IsCommunicationError()
{
    if (!Ready)
    {
        return false;
    }

    return
        ConsecutiveFrameErrorCount >=
        FRAME_ERROR_THRESHOLD;
}

uint32_t ModbusRTU::GetConsecutiveFrameErrorCount()
{
    return ConsecutiveFrameErrorCount;
}

//-------------------------------------------------------------
// Asynchronous Modbus - Start Read Input Registers
//-------------------------------------------------------------
bool ModbusRTU::StartReadInputRegisters(
    uint8_t slave,
    uint16_t address,
    uint16_t count)
{
    // 통신 초기화 확인
    if (!Ready)
    {
        return false;
    }

    // 기존 트랜잭션이 종료되지 않았으면 거부
    if (Transaction.state != AsyncState::Idle)
    {
        return false;
    }

    // 입력값 확인
    if (slave == 0U ||
        count == 0U ||
        count > 29U ||
        static_cast<uint32_t>(address) +
            count > 65536U)
    {
        return false;
    }

    // 트랜잭션 초기화
    Transaction = AsyncTransaction{};

    Transaction.slave = slave;
    Transaction.address = address;
    Transaction.count = count;

    //---------------------------------------------------------
    // Start Asynchronous Transaction
    //---------------------------------------------------------
    LastErrorReason = ErrorReason::None;

    SendAsyncRequest();

    Transaction.state = AsyncState::Waiting;

    return true;
}

//-------------------------------------------------------------
// Asynchronous Modbus - Update
// Description:
//   Collect received UART bytes without blocking.
//   Frame validation and timeout handling will follow.
//-------------------------------------------------------------
void ModbusRTU::UpdateAsync()
{
    if (!Ready ||
        Transaction.state != AsyncState::Waiting)
    {
        return;
    }

    // 한 번의 호출에서 최대 64바이트만 처리
    uint8_t processed = 0U;

    while (RS485::Available() > 0 &&
           processed < 64U)
    {
        if (Transaction.responseLength >=
            sizeof(Transaction.response))
        {
            LastErrorReason = ErrorReason::InvalidLength;
            
            RecordFrameError();

            Transaction.state = AsyncState::Failed;
            return;
        }

        const int value = RS485::ReadByte();

        if (value < 0)
        {
            break;
        }

        //---------------------------------------------------------
        // First byte received - Reset NoRx counter
        //---------------------------------------------------------
        if (Transaction.responseLength == 0U)
        {
            RS485::ReportAsyncReceiveResult(true);
        }

        Transaction.response[
            Transaction.responseLength++] =
                static_cast<uint8_t>(value);

        ++processed;

        //---------------------------------------------------------
        // Determine expected Modbus response length
        //---------------------------------------------------------

        if (Transaction.responseLength >= 2U &&
            Transaction.expectedLength == 0U)
        {
            const uint8_t function =
                Transaction.response[1];

            //---------------------------------------------------------
            // Reject unsupported Function Code
            //---------------------------------------------------------
            if (function != MODBUS_READ_INPUT_REGISTERS &&
                function != static_cast<uint8_t>(
                    MODBUS_READ_INPUT_REGISTERS | 0x80U))
            {
                LastErrorReason = ErrorReason::InvalidFunction;

                RecordFrameError();

                Transaction.state = AsyncState::Failed;
                return;
            }

            // Modbus exception response:
            // Slave + Function + ExceptionCode + CRC(2)
            if (function ==
                static_cast<uint8_t>(
                    MODBUS_READ_INPUT_REGISTERS | 0x80U))
            {
                Transaction.expectedLength = 5U;
            }
            // Normal Read Input Registers response:
            // Slave + Function + ByteCount + Data + CRC(2)
            else if (function == MODBUS_READ_INPUT_REGISTERS &&
                    Transaction.responseLength >= 3U)
            {
                Transaction.expectedLength =
                    5U + Transaction.response[2];
            }
        }

        //---------------------------------------------------------
        // Response length protection
        //---------------------------------------------------------

        if (Transaction.expectedLength >
            sizeof(Transaction.response))
        {
            LastErrorReason = ErrorReason::InvalidLength;

            RecordFrameError();

            Transaction.state = AsyncState::Failed;
            return;
        }

        //---------------------------------------------------------
        // Complete frame received - Validate response
        //---------------------------------------------------------
        if (Transaction.expectedLength > 0U &&
            Transaction.responseLength >=
                Transaction.expectedLength)
        {
            const uint8_t* rx = Transaction.response;
            const size_t length = Transaction.responseLength;

            //-----------------------------------------------------
            // Validate CRC16
            //-----------------------------------------------------
            const uint16_t receivedCrc =
                static_cast<uint16_t>(rx[length - 2U]) |
                (static_cast<uint16_t>(rx[length - 1U]) << 8);

            const uint16_t calculatedCrc =
                CRC16::Calculate(rx, length - 2U);

            if (receivedCrc != calculatedCrc)
            {
                LastErrorReason = ErrorReason::CrcError;

                RecordFrameError();

                Transaction.state = AsyncState::Failed;
                return;
            }

            //-----------------------------------------------------
            // Validate Slave Address
            //-----------------------------------------------------
            if (rx[0] != Transaction.slave)
            {
                LastErrorReason = ErrorReason::InvalidSlave;

                RecordFrameError();

                Transaction.state = AsyncState::Failed;
                return;
            }

            //-----------------------------------------------------
            // Check Modbus Exception
            //-----------------------------------------------------
            if (rx[1] ==
                static_cast<uint8_t>(
                    MODBUS_READ_INPUT_REGISTERS | 0x80U))
            {
                LastErrorReason = ErrorReason::Exception;
                Transaction.state = AsyncState::Failed;
                return;
            }

            //-----------------------------------------------------
            // Validate Function Code
            //-----------------------------------------------------
            if (rx[1] != MODBUS_READ_INPUT_REGISTERS)
            {
                LastErrorReason = ErrorReason::InvalidFunction;

                RecordFrameError();

                Transaction.state = AsyncState::Failed;
                return;
            }

            //-----------------------------------------------------
            // Validate Register Byte Count
            //-----------------------------------------------------
            if (rx[2] != Transaction.count * 2U)
            {
                LastErrorReason = ErrorReason::InvalidByteCount;

                RecordFrameError();
                
                Transaction.state = AsyncState::Failed;
                return;
            }

            //-----------------------------------------------------
            // Valid Modbus Response
            //-----------------------------------------------------
            LastErrorReason = ErrorReason::None;

            // 정상 프레임 수신으로 연속 오류 해제
            ResetCommunicationState();

            Transaction.state = AsyncState::Success;
            return;
        }
    }

    //---------------------------------------------------------
    // Asynchronous Response Timeout / Retry
    //---------------------------------------------------------
    const uint32_t now = millis();

    if (static_cast<uint32_t>(
            now - Transaction.startMillis) >= MODBUS_TIMEOUT)
    {
        const bool hasPartialResponse =
            Transaction.responseLength > 0U;

        //-----------------------------------------------------
        // Report no-response timeout
        //-----------------------------------------------------
        if (!hasPartialResponse)
        {
            RS485::ReportAsyncReceiveResult(false);
        }

        //-----------------------------------------------------
        // Partial frame recovery (once per retry cycle)
        //-----------------------------------------------------
        if (hasPartialResponse &&
            !Transaction.recoveryAttempted)
        {
            
            Logger::Info(
                "MODBUS ASYNC",
                "Partial Recovery, Retry=" +
                String(Transaction.retryCount)
            );

            Transaction.recoveryAttempted = true;

            RS485::ClearReceiveBufferNonBlocking();

            SendAsyncRequest();

            return;
        }

        //-----------------------------------------------------
        // Normal Modbus retry
        //-----------------------------------------------------
        if (Transaction.retryCount < MODBUS_RETRY)
        {
            ++Transaction.retryCount;

            Logger::Info(
                "MODBUS ASYNC",
                "Retry=" +
                String(Transaction.retryCount)
            );

            Transaction.recoveryAttempted = false;

            RS485::ClearReceiveBufferNonBlocking();

            SendAsyncRequest();

            return;
        }

        //-----------------------------------------------------
        // Final communication failure
        //-----------------------------------------------------
        LastErrorReason =
            hasPartialResponse
                ? ErrorReason::InvalidLength
                : ErrorReason::NoResponse;

        Transaction.state = AsyncState::Failed;

        return;
    }
}

//-------------------------------------------------------------
// Asynchronous Modbus - Send Request
//-------------------------------------------------------------
void ModbusRTU::SendAsyncRequest()
{
    //---------------------------------------------------------
    // Reset response state before each transmission
    //---------------------------------------------------------
    memset(
        Transaction.response,
        0,
        sizeof(Transaction.response)
    );

    Transaction.responseLength = 0U;
    Transaction.expectedLength = 0U;

    //---------------------------------------------------------
    // Build Modbus request
    //---------------------------------------------------------
    uint8_t frame[8];

    frame[0] = Transaction.slave;
    frame[1] = MODBUS_READ_INPUT_REGISTERS;

    frame[2] =
        static_cast<uint8_t>(Transaction.address >> 8);

    frame[3] =
        static_cast<uint8_t>(Transaction.address & 0xFF);

    frame[4] =
        static_cast<uint8_t>(Transaction.count >> 8);

    frame[5] =
        static_cast<uint8_t>(Transaction.count & 0xFF);

    const uint16_t crc =
        CRC16::Calculate(frame, 6);

    frame[6] =
        static_cast<uint8_t>(crc & 0xFF);

    frame[7] =
        static_cast<uint8_t>(crc >> 8);

    RS485::Send(frame, sizeof(frame));

    Transaction.startMillis = millis();
}