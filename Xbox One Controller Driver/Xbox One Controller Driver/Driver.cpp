//
//  Driver.cpp
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

#include "Driver.h"
#include "ControlStruct.h"

namespace HID_One {
#if X360MODE
#include "360hid.h"
#else
#include "xonehid.h"
#endif
}

OSDefineMetaClassAndStructors(com_vestigl_driver_Xone_Driver, IOUSBHIDDriver)

// Define the superclass
#define super IOUSBHIDDriver

// This looks a little bit nicer
#define XboxOneControllerDriver com_vestigl_driver_Xone_Driver

// This is where the device data is stored
#define kDriverSettingKey "DeviceData"

#pragma mark - Wakeup Messages
constexpr UInt8 XboxOneControllerWakeupMsg1[] = { 0x02, 0x20, 0x01, 0x1C, 0x7E, 0xED, 0x8B, 0x11, 0x0F, 0xA8, 0x00, 0x00, 0x5E, 0x04, 0xD1, 0x02, 0x01, 0x00, 0x01, 0x00, 0x17, 0x01, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00 };
constexpr UInt8 XboxOneControllerWakeupMsg2[] = { 0x05, 0x20, 0x00, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x53 };
constexpr UInt8 XboxOneControllerWakeupMsg3[] = { 0x05, 0x20, 0x01, 0x01, 0x00 };
constexpr UInt8 XboxOneControllerWakeupMsg4[] = { 0x0A, 0x20, 0x02, 0x03, 0x00, 0x01, 0x14 };


#pragma mark - IOHID Methods
bool XboxOneControllerDriver::init(OSDictionary* dict)
{
    if (!super::init(dict))
    {
        IOLog("Xbox One Controller - Init failed.\n");
        return false;
    }
    
    _interruptPipe = nullptr;
    
    // Set some default values just in case no preferences are set
    xoneInvertLeftX = false;
    xoneInvertLeftY = false;
    xoneInvertRightX = false;
    xoneInvertRightY = false;
    xoneLeftDead = 0;
    xoneRightDead = 0;
    xoneTriggerDead = 0;
    
    return true;
}

bool XboxOneControllerDriver::didTerminate(IOService *provider, IOOptionBits options, bool *defer)
{
    // Clean up your mess
    if (_interruptPipe != nullptr)
    {
        _interruptPipe->release();
        _interruptPipe = nullptr;
    }
    
    return super::didTerminate(provider, options, defer);
}

IOReturn XboxOneControllerDriver::handleReport(IOMemoryDescriptor* descriptor, IOHIDReportType type, IOOptionBits options)
{
    UInt8 reportid;
    IOByteCount read = descriptor->readBytes(0, &reportid, sizeof(reportid));
    if (read != 1)
    {
        IOLog("Xbox One Controller - handleReport read from report descriptor failed\n");
        return kIOReturnNoMemory;
    }
    
    if (reportid == 0x20)
    {
        XONE_IN_REPORT report;
        descriptor->readBytes(0, &report, sizeof(report));
        applyUserSettings(&report);
#if X360MODE
        changeTo360Packet(&report);
#endif
        descriptor->writeBytes(0, &report, sizeof(report));
    }
    else if (reportid == 0x07)
    {
        
    }
    else
    {
        return kIOReturnSuccess;
    }
    
    return super::handleReport(descriptor, type, options);
}

bool XboxOneControllerDriver::handleStart(IOService *provider)
{
    IOLog("Xbox One Controller - handleStart\n");
    IOUSBInterface* interface = nullptr;
    IOUSBPipe* interruptPipe = nullptr;
    IOReturn ret = kIOReturnSuccess;
    
    IOUSBFindEndpointRequest pipeRequest = {
        .type = kUSBInterrupt,
        .direction = kUSBOut,
    };
    
    if (!super::handleStart(provider))
    {
        IOLog("Xbox One Controller - Super handleStart\n");
        goto cleanup;
    }
    
    interface = OSDynamicCast(IOUSBInterface, provider);
    if (interface == nullptr)
    {
        IOLog("Xbox One Controller - handleStart interface incorrect type\n");
        goto cleanup;
    }
    
    interruptPipe = interface->FindNextPipe(nullptr, &pipeRequest);
    if (interruptPipe == nullptr)
    {
        IOLog("Xbox One Controller - handleStart no interrupt pipe\n");
        goto cleanup;
    }
    
    _interruptPipe = interruptPipe;
    ret = wakeupController();
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - handleStart couldn't wakeup ERROR: %08x\n", ret);
        _interruptPipe = nullptr;
        goto cleanup;
    }
    
    _interruptPipe->retain();
    
cleanup:
    return _interruptPipe != nullptr;
}

IOReturn XboxOneControllerDriver::newReportDescriptor(IOMemoryDescriptor **descriptor) const
{
    if (descriptor == nullptr)
    {
        return kIOReturnBadArgument;
    }
    
    constexpr size_t descriptorSize = sizeof(HID_One::ReportDescriptor);
    IOMemoryDescriptor* buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, descriptorSize);
    if (buffer == nullptr)
    {
        return kIOReturnNoMemory;
    }
    
    IOByteCount written = buffer->writeBytes(0, HID_One::ReportDescriptor, descriptorSize);
    if (written != sizeof(HID_One::ReportDescriptor))
    {
        buffer->release();
        return kIOReturnNoSpace;
    }
    
    *descriptor = buffer;
    return kIOReturnSuccess;
}

IOReturn XboxOneControllerDriver::setPowerState(unsigned long powerStateOrdinal, IOService* device)
{
    // Apple returns kIOPM constants as IOReturn codes. Don't check them with SUCCEEDED or FAILED.
    {
        IOReturn ret = super::setPowerState(powerStateOrdinal, device);
        if (ret != kIOPMAckImplied)
        {
            IOLog("Xbox One Controller - setPowerState failed\n");
            return ret;
        }
    }
    
    if (powerStateOrdinal == kUSBHIDPowerStateOn)
    {
        IOReturn ret = wakeupController();
        if (ret != kIOReturnSuccess)
        {
            IOLog("Xbox One Controller - setPowerState wakeup failed ERROR: %08x\n", ret);
        }
    }
    
    return kIOPMAckImplied;
}


#pragma mark - Force Feedback
IOReturn XboxOneControllerDriver::setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options)
{
    unsigned char data[4];
    report->readBytes(0, data, 4);
    switch (data[0]) {
        case 0x00:  // Set force feedback
            XONE_OUT_RUMBLE rumble;
            rumble.command = 0x09;
            rumble.reserved1 = 0x08;
            rumble.reserved2 = 0x00;
            rumble.substructure = 0x09;
            rumble.mode = 0x00;
            rumble.rumbleMask = 0x0F;
            rumble.length = 0x80;
            
            rumble.trigL = 0x00;
            rumble.trigR = 0x00;
            rumble.little = data[2];
            rumble.big = data[3];
            
            queueWrite(&rumble, 11);
            return kIOReturnSuccess;
        case 0x01: // Unsupported LED
            return kIOReturnSuccess;
        default:
            IOLog("Xbox One Controller - setReport unknown escape ERROR:%d\n", data[0]);
            return kIOReturnUnsupported;
    }
    return kIOReturnUnsupported;
}


#pragma mark - Wakeup Controller
IOReturn XboxOneControllerDriver::wakeupController()
{
    IOReturn ret = kIOReturnSuccess;
    
    if (_interruptPipe == nullptr)
    {
        IOLog("Xbox One Controller - wakeupController _interruptPipe is null\n");
        ret = kIOReturnInternalError;
        goto exit;
    }
    
    ret = wakeupControllerHelper(XboxOneControllerWakeupMsg1);
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - wakeupController Wakeup 1 failed\n");
        goto exit;
    }
    
    ret = wakeupControllerHelper(XboxOneControllerWakeupMsg2);
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - wakeupController Wakeup 2 failed\n");
        goto exit;
    }
    
    ret = wakeupControllerHelper(XboxOneControllerWakeupMsg3);
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - wakeupController Wakeup 3 failed\n");
        goto exit;
    }
    
    ret = wakeupControllerHelper(XboxOneControllerWakeupMsg4);
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - wakeupController Wakeup 4 failed\n");
        goto exit;
    }
    
exit:
    return ret;
}

IOReturn XboxOneControllerDriver::wakeupControllerHelper(const UInt8 *buffer)
{
    IOReturn ret = kIOReturnSuccess;
    IOByteCount bytesWritten = 0;
    
    size_t wakeupSize = sizeof(buffer);
    IOMemoryDescriptor* wakeup = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, wakeupSize);
    if (wakeup == nullptr)
    {
        IOLog("Xbox One Controller - wakeupControllerHelper could not allocate wakeup\n");
        ret = kIOReturnNoMemory;
        goto cleanup;
    }
    
    bytesWritten = wakeup->writeBytes(0, buffer, wakeupSize);
    if (bytesWritten != wakeupSize)
    {
        IOLog("Xbox One Controller - wakeupControllerHelper bytesWritten != wakeupSize\n");
        ret = kIOReturnOverrun;
        goto cleanup;
    }
    
    ret = _interruptPipe->Write(wakeup, 0, 0, wakeup->getLength());
    if (ret != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - wakeupControllerHelper could not send wakeup\n");
        goto cleanup;
    }
    
cleanup:
    if (wakeup != nullptr)
    {
        wakeup->release();
    }
    
    return ret;
}


#pragma mark - Packet Modification
void XboxOneControllerDriver::applyUserSettings(void* reportVoid)
{
    XONE_IN_REPORT* report = (XONE_IN_REPORT*)reportVoid;
    // Make sure to invert what needs inverting
    if (xoneInvertLeftX)
        report->leftStickX = ~report->leftStickX;
    if (!xoneInvertLeftY)
        report->leftStickY = ~report->leftStickY;
    if (xoneInvertRightX)
        report->rightStickX = ~report->rightStickX;
    if (!xoneInvertRightY)
        report->rightStickY = ~report->rightStickY;
    
    // Apply the left stick deadzone
    if (xoneLeftDead != 0)
    {
        // Use a nifty bit of bit manipulation to get around the lack of abs
        SInt16 const absMaskX = report->leftStickX >> (sizeof(SInt16) * 8 - 1);
        // Simply, if absolute value < deadzone
        if (((report->leftStickX + absMaskX) ^ absMaskX) < xoneLeftDead)
            report->leftStickX = 0;
        SInt16 const absMaskY = report->leftStickY >> (sizeof(SInt16) * 8 - 1);
        if (((report->leftStickY + absMaskY) ^ absMaskY) < xoneLeftDead)
            report->leftStickY = 0;
    }
    
    // Apply the right stick deadzone
    if (xoneRightDead != 0)
    {
        SInt16 const absMaskX = report->rightStickX >> (sizeof(SInt16) * 8 - 1);
        if (((report->rightStickX + absMaskX) ^ absMaskX) < xoneRightDead)
            report->rightStickX = 0;
        SInt16 const absMaskY = report->rightStickY >> (sizeof(SInt16) * 8 - 1);
        if (((report->rightStickY + absMaskY) ^ absMaskY) < xoneRightDead)
            report->rightStickY = 0;
    }
    
    // Flip the sticks for lefties.
    if (southpaw)
    {
        SInt16 holdLeftX, holdLeftY;
        holdLeftX = report->leftStickX;
        holdLeftY = report->leftStickY;
        report->leftStickX = report->rightStickX;
        report->leftStickY = report->rightStickY;
        report->rightStickX = holdLeftX;
        report->rightStickY = holdLeftY;
    }
    
    // Apply trigger deadzones
    if (xoneTriggerDead != 0)
    {
        if (report->triggerLeft != 0)
        {
            if (report->triggerLeft < xoneTriggerDead)
                report->triggerLeft = 0;
        }
        if (report->triggerRight != 0)
        {
            if (report->triggerRight < xoneTriggerDead)
                report->triggerRight = 0;
        }
    }
}

#if X360MODE
void XboxOneControllerDriver::changeTo360Packet(void* reportVoid)
{
    XONE_IN_REPORT* report = (XONE_IN_REPORT*)reportVoid;
    UInt16 new_buttons = 0;
    new_buttons |= ((report->buttons & XONE_MENU)           == XONE_MENU)           << BIT_X360_START;
    new_buttons |= ((report->buttons & XONE_VIEW)           == XONE_VIEW)           << BIT_X360_BACK;
    new_buttons |= ((report->buttons & XONE_A)              == XONE_A)              << BIT_X360_A;
    new_buttons |= ((report->buttons & XONE_B)              == XONE_B)              << BIT_X360_B;
    new_buttons |= ((report->buttons & XONE_X)              == XONE_X)              << BIT_X360_X;
    new_buttons |= ((report->buttons & XONE_Y)              == XONE_Y)              << BIT_X360_Y;
    new_buttons |= ((report->buttons & XONE_DPAD_UP)        == XONE_DPAD_UP)        << BIT_X360_DPAD_UP;
    new_buttons |= ((report->buttons & XONE_DPAD_DOWN)      == XONE_DPAD_DOWN)      << BIT_X360_DPAD_DOWN;
    new_buttons |= ((report->buttons & XONE_DPAD_LEFT)      == XONE_DPAD_LEFT)      << BIT_X360_DPAD_LEFT;
    new_buttons |= ((report->buttons & XONE_DPAD_RIGHT)     == XONE_DPAD_RIGHT)     << BIT_X360_DPAD_RIGHT;
    new_buttons |= ((report->buttons & XONE_LEFT_SHOULDER)  == XONE_LEFT_SHOULDER)  << BIT_X360_LEFT_SHOULDER;
    new_buttons |= ((report->buttons & XONE_RIGHT_SHOULDER) == XONE_RIGHT_SHOULDER) << BIT_X360_RIGHT_SHOULDER;
    new_buttons |= ((report->buttons & XONE_LEFT_THUMB)     == XONE_LEFT_THUMB)     << BIT_X360_LEFT_THUMB;
    new_buttons |= ((report->buttons & XONE_RIGHT_THUMB)    == XONE_RIGHT_THUMB)    << BIT_X360_RIGHT_THUMB;
    report->buttons = new_buttons;
}
#endif


#pragma mark - Settings / Preferences
void XboxOneControllerDriver::readSettings()
{
    OSDictionary *data_dictionary = OSDynamicCast(OSDictionary, getProperty(kDriverSettingKey));
    OSBoolean *value;
    OSNumber *number;
    
    if (data_dictionary)
    {
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("l_inv_x"));
        
        if (value)
        {
            xoneInvertLeftX = value->getValue();
        }
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("l_inv_y"));
        if (value)
            xoneInvertLeftY = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("r_inv_x"));
        if (value)
            xoneInvertRightX = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("r_inv_y"));
        if (value)
            xoneInvertRightY = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("southpaw"));
        if (value)
            southpaw = value->getValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("l_stick_d"));
        if (number)
            xoneLeftDead = number->unsigned32BitValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("r_stick_d"));
        if (number)
            xoneRightDead = number->unsigned32BitValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("t_d"));
        if (number)
            xoneTriggerDead = number->unsigned32BitValue();
    }
}

IOReturn XboxOneControllerDriver::setProperties(OSObject* properties)
{
    OSDictionary* dictionary;
    IOReturn ret = kIOReturnSuccess;
    
    dictionary = OSDynamicCast(OSDictionary, properties);
    if (dictionary != nullptr)
    {
        setProperty(kDriverSettingKey, dictionary);
        readSettings();
    }
    else
    {
        ret = kIOReturnBadArgument;
    }
    
    return ret;
}


#pragma mark - Write Data
bool XboxOneControllerDriver::queueWrite(const void* bytes, UInt32 length)
{
    IOBufferMemoryDescriptor* outBuffer;
    IOUSBCompletion complete;
    IOReturn err;
    bool ret = true;

    outBuffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, length);
    if (outBuffer == nullptr)
    {
        IOLog("Xbox One Controller - queueWrite unable to allocate buffer\n");
        ret = false;
    }
    outBuffer->writeBytes(0, bytes, length);
    complete.target = this;
    complete.action = writeCompleteInternal;
    complete.parameter = outBuffer;
    err = _interruptPipe->Write(outBuffer, 0, 0, length, &complete);
    if (err != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - queueWrite failed to start ERROR: %08x\n", err);
        ret = false;
    }
    
    return ret;
}

void XboxOneControllerDriver::writeComplete(void* parameter, IOReturn status, UInt32 bufferSizeRemaining)
{
    IOMemoryDescriptor* memory = (IOMemoryDescriptor*)parameter;
    if (status != kIOReturnSuccess)
    {
        IOLog("Xbox One Controller - writeComplete write failed ERROR: %08x\n", status);
    }
    memory->release();
}

void XboxOneControllerDriver::writeCompleteInternal(void* target, void* parameter, IOReturn status, UInt32 bufferSizeRemaining)
{
    if (target != nullptr)
    {
        ((XboxOneControllerDriver*)target)->writeComplete(parameter, status, bufferSizeRemaining);
    }
}


#pragma mark - 360 Specific Spoofing
#if X360MODE
OSNumber* XboxOneControllerDriver::newProductIDNumber() const
{
    return OSNumber::withNumber(654, 16);
}

OSString* XboxOneControllerDriver::newProductString() const
{
    // Hack to get Shovel Knight to work. Could set alternate name and then submit it to Yacht Club
    return OSString::withCString("X360Controller");
}
#else
OSString* XboxOneControllerDriver::newProductString() const
{
    // Hack to get Shovel Knight to work. Could set alt name and then submit it to Yacht Club
    return OSString::withCString("Xbox One Controller");
}
#endif