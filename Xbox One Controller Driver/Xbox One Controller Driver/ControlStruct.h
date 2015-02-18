//
//  ControlStruct.h
//  Xone Driver
//
//  Created by Drew Mills on 10/01/14.
//  Copyright (c) 2014 Drew Mills
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __CONTROLSTRUCT_H__
#define __CONTROLSTRUCT_H__

typedef struct {
    UInt8 messageType;
    UInt8 dummy;
    UInt8 counter;
    UInt8 packetSize;
} XONE_HEADER;

typedef struct {
    XONE_HEADER header;
    UInt16 buttons;
    UInt16 triggerLeft;
    UInt16 triggerRight;
    SInt16 leftStickX;
    SInt16 leftStickY;
    SInt16 rightStickX;
    SInt16 rightStickY;
} XONE_IN_REPORT;

typedef struct {
    XONE_HEADER header;
    UInt8 state;
    UInt8 dummy;
} XONE_IN_GUIDE_REPORT;

typedef struct {
    UInt8 command; // 0x09
    UInt8 reserved1; // So far 0x08
    UInt8 reserved2; // So far always 0x00
    UInt8 substructure; // 0x08 - Continuous, 0x09 - Single
    UInt8 mode; // So far always 0x00
    UInt8 rumbleMask; // So far always 0x0F
    UInt8 trigL, trigR;
    UInt8 little, big;
    UInt8 length; // Length of time to rumble
    UInt8 period; // Period of time between pulses. DO NOT INCLUDE WHEN SUBSTRUCTURE IS 0x09
} XONE_OUT_RUMBLE;

// Imitates the X-Input implementation of button mapping. Should help devs set up their games with bit twiddling.
typedef enum {
    X360_DPAD_UP        = 0x0001, // Bit 00
    X360_DPAD_DOWN      = 0x0002, // Bit 01
    X360_DPAD_LEFT      = 0x0004, // Bit 02
    X360_DPAD_RIGHT     = 0x0008, // Bit 03
    X360_START          = 0x0010, // Bit 04
    X360_BACK           = 0x0020, // Bit 05
    X360_LEFT_THUMB     = 0x0040, // Bit 06
    X360_RIGHT_THUMB    = 0x0080, // Bit 07
    X360_LEFT_SHOULDER  = 0x0100, // Bit 08
    X360_RIGHT_SHOULDER = 0x0200, // Bit 09
    X360_A              = 0x1000, // Bit 12
    X360_B              = 0x2000, // Bit 13
    X360_X              = 0x4000, // Bit 14
    X360_Y              = 0x8000, // Bit 15
} GAMEPAD_X360;

typedef enum {
    XONE_SYNC           = 0x0001, // Bit 00
    XONE_MENU           = 0x0004, // Bit 02
    XONE_VIEW           = 0x0008, // Bit 03
    XONE_A              = 0x0010, // Bit 04
    XONE_B              = 0x0020, // Bit 05
    XONE_X              = 0x0040, // Bit 06
    XONE_Y              = 0x0080, // Bit 07
    XONE_DPAD_UP        = 0x0100, // Bit 08
    XONE_DPAD_DOWN      = 0x0200, // Bit 09
    XONE_DPAD_LEFT      = 0x0400, // Bit 10
    XONE_DPAD_RIGHT     = 0x0800, // Bit 11
    XONE_LEFT_SHOULDER  = 0x1000, // Bit 12
    XONE_RIGHT_SHOULDER = 0x2000, // Bit 13
    XONE_LEFT_THUMB     = 0x4000, // Bit 14
    XONE_RIGHT_THUMB    = 0x8000, // Bit 15
} GAMEPAD_XONE;

typedef enum {
    BIT_X360_DPAD_UP        = 0,
    BIT_X360_DPAD_DOWN      = 1,
    BIT_X360_DPAD_LEFT      = 2,
    BIT_X360_DPAD_RIGHT     = 3,
    BIT_X360_START          = 4,
    BIT_X360_BACK           = 5,
    BIT_X360_LEFT_THUMB     = 6,
    BIT_X360_RIGHT_THUMB    = 7,
    BIT_X360_LEFT_SHOULDER  = 8,
    BIT_X360_RIGHT_SHOULDER = 9,
    BIT_X360_A              = 12,
    BIT_X360_B              = 13,
    BIT_X360_X              = 14,
    BIT_X360_Y              = 15,
} BITINDEX_360;

#endif /* __CONTROLSTRUCT_H__ */