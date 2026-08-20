//-------------------------------------------------------------
// File : VehicleInput.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-20
// Project : SVEMS
// Version : 0.6.3
// Description : VehicleInput
//-------------------------------------------------------------

#ifndef VEHICLE_INPUT_H
#define VEHICLE_INPUT_H

class VehicleInput
{
public:
    static bool Begin();
    static void Update();

    static bool IsActive();

private:
    static bool Active;
};

#endif