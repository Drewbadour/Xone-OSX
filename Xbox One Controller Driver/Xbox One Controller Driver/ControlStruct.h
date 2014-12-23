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
    UInt8 message_type;
    UInt8 dummy;
    UInt8 counter;
    UInt8 packet_size;
} XONE_HEADER;

typedef struct {
    XONE_HEADER header;
    UInt8 button_one;
    UInt8 button_two;
    UInt16 trigger_left;
    UInt16 trigger_right;
    SInt16 left_stick_x;
    SInt16 left_stick_y;
    SInt16 right_stick_x;
    SInt16 right_stick_y;
} XONE_IN_REPORT;

typedef struct {
    XONE_HEADER header;
    UInt8 state;
    UInt8 dummy;
} XONE_IN_GUIDE_REPORT;

typedef struct {
    XONE_HEADER header;
    UInt8 dummy;
    UInt8 rumble_style;
    UInt8 left_trigger;
    UInt8 right_trigger;
    UInt8 left_hand;
    UInt8 right_hand;
    UInt8 length; // Length of time to rumble
    UInt16 dummy_pack;
} XONE_OUT_RUMBLE;

enum {
    heart_beat              = 0x03,
    guide_report            = 0x07,
    force_feedback          = 0x09,
    in_report               = 0x20,
};

enum {
    heart_beat_length       = 0x04,
    guide_report_legnth     = 0x02,
    force_feedback_length   = 0x09,
    in_report_length        = 0x0E,
};

#endif /* __CONTROLSTRUCT_H__ */