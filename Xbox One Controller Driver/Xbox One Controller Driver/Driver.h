//
//  Driver.h
//  Xone Driver
//
//  Created by Drew Mills on 2/10/15.
//  Copyright (c) 2015 Vestigl. All rights reserved.
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
//  Big thanks to @zneak on Github for clueing me in to IOUSBHIDDriver
//  and being the original author of a lot of this new code.
//

#ifndef XONE_DRIVER_H
#define XONE_DRIVER_H

// 0 = Xbox One Mode; 1 = Xbox 360 Mode (Default)
#define X360MODE 1

#include <IOKit/usb/IOUSBHIDDriver.h>

class com_vestigl_driver_Xone_Driver : public IOUSBHIDDriver
{
    OSDeclareDefaultStructors(com_vestigl_driver_Xone_Driver)
    
private:
    bool xoneInvertLeftX, xoneInvertLeftY;
    bool xoneInvertRightX, xoneInvertRightY;
    bool southpaw;
    SInt16 xoneLeftDead, xoneRightDead;
    UInt16 xoneTriggerDead;
    
    IOUSBPipe* _interruptPipe;
    
    
private:
    IOReturn wakeupController();
    IOReturn wakeupControllerHelper(const UInt8 *buffer);
    void readSettings();
    
    
protected:
    virtual bool handleStart(IOService* provider) override;
    virtual IOReturn handleReport(IOMemoryDescriptor* descriptor, IOHIDReportType type, IOOptionBits options) override;
    
    void applyUserSettings(void* report);
#if X360MODE
    void changeTo360Packet(void* reportVoid);
#endif
    
    bool queueWrite(const void* bytes, UInt32 length);
    void writeComplete(void* parameter, IOReturn status, UInt32 bufferSizeRemaining);
    
    
public:
    virtual bool init(OSDictionary* dictionary = nullptr) override;
    virtual bool didTerminate(IOService* provider, IOOptionBits options, bool* defer) override;
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService* device) override;
    virtual IOReturn setProperties(OSObject* properties);
    virtual IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    
    static void writeCompleteInternal(void* target, void* parameter, IOReturn status, UInt32 bufferSizeRemaining);

virtual OSString* newProductString() const override;
#if X360MODE
    OSNumber* newProductIDNumber() const override;
#endif
};

#endif
