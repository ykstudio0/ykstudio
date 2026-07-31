//-------------------------------------------------------------
// File : DeviceBase.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 5.0.0
// Description : Device Base
//-------------------------------------------------------------

#pragma once

namespace SVEMS::Device
{
    enum class DeviceState
    {
        Offline,
        Initializing,
        Online,
        Error
    };

    class DeviceBase
    {
    public:
        virtual ~DeviceBase() = default;

        virtual bool Begin() = 0;
        virtual void Update() = 0;
        virtual const char* GetName() const = 0;

        DeviceState GetState() const{
            return m_state;
        }

        bool IsOnline() const
        {
            return m_state == DeviceState::Online;
        }

    protected:
        void SetState(DeviceState state)
        {
            m_state = state;
        }

    private:
        DeviceState m_state = DeviceState::Offline;
    };
}