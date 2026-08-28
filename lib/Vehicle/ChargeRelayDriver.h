//-------------------------------------------------------------
// File : ChargeRelaydriver.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-28
// Project : SVEMS
// Version : 0.7.0
// Description : Vehicle Battery Charge Control
//-------------------------------------------------------------

#pragma once

namespace SVEMS
{
    namespace Vehicle
    {

        class ChargeRelayDriver
        {
        public:
            static void Begin();
            static void SetEnabled(bool enabled);
            static bool IsEnabled();

        private:
            static bool m_enabled;
        };

    } // namespace Vehicle
} // namespace SVEMS