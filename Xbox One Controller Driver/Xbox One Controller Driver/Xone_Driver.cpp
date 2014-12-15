//
//  Xone_Driver.cpp
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

#include <IOKit/IOService.h>
#include "Xone_Driver.h"
#include "Xone_Controller.h"

#define kDriverSettingKey       "DeviceData"
#define kIOSerialDeviceType   "SerialXoneDevice"

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_vestigl_driver_Xone_Driver, IOService)

// Define the driver's superclass.
#define super IOService

#pragma mark - LockRequired

class LockRequired
{
private:
    IOLock *_lock;
public:
    LockRequired(IOLock *lock)
    {
        _lock = lock;
        IOLockLock(_lock);
    }
    ~LockRequired()
    {
        IOLockUnlock(_lock);
    }
};



// Constructor equivalent
#pragma mark - Init

bool com_vestigl_driver_Xone_Driver::init(OSDictionary* dict)
{
    bool result = super::init(dict);
    
    xone_main_lock = IOLockAlloc();
    
    xone_device = nullptr;
    xone_interface = nullptr;
    xone_in_pipe = nullptr;
    xone_out_pipe = nullptr;
    xone_in_buffer = nullptr;
    xone_controller = nullptr;
    
    xone_invert_left_x = false;
    xone_invert_left_y = false;
    xone_invert_right_x = false;
    xone_invert_right_y = false;
    xone_left_thumb_deadzone = 7849;
    xone_right_thumb_deadzone = 8689;
    xone_trigger_deadzone = 120;
    
    return result;
}



// Check to see that driver can handle device
#pragma mark - Probe
IOService* com_vestigl_driver_Xone_Driver::probe(IOService* provider, SInt32 *score)
{
    return super::probe(provider, score);
}



// Called after probe has been satisfied, and got the highest score
#pragma mark - Start Methods

UInt32 com_vestigl_driver_Xone_Driver::get_max_packet_size(IOUSBPipe *pipe)
{
    const IOUSBEndpointDescriptor* ed;
    
    ed = pipe->GetEndpointDescriptor();
    if (ed == nullptr)
        return 0;
    else
        return ed->wMaxPacketSize;
}


bool com_vestigl_driver_Xone_Driver::start(IOService* provider)
{
    const IOUSBConfigurationDescriptor* cd;
    IOUSBFindInterfaceRequest req;
    IOUSBFindEndpointRequest pipe;
    
    if (!super::start(provider))
    {
        return false;
    }
    
    xone_device = OSDynamicCast(IOUSBDevice, provider);
    if (xone_device == nullptr)
    {
        return false;
    }
    
    if (xone_device->GetNumConfigurations() < 1)
    {
        xone_device = nullptr;
        return false;
    }
    
    cd = xone_device->GetFullConfigurationDescriptor(0);
    if (cd == nullptr)
    {
        xone_device = nullptr;
        return false;
    }
    
    if (!xone_device->isOpen())
    {
        if (!xone_device->open(this))
        {
            xone_device = nullptr;
            release_all();
            return false;
        }
    }
    
    if (xone_device->SetConfiguration(this, cd->bConfigurationValue) != kIOReturnSuccess)
    {
        release_all();
        return false;
    }
    
    req.bInterfaceClass = 255;
    req.bInterfaceSubClass = 71;
    req.bInterfaceProtocol = 208;
    req.bAlternateSetting = kIOUSBFindInterfaceDontCare;
    xone_interface = xone_device->FindNextInterface(nullptr, &req);
    if (xone_interface == nullptr)
    {
        release_all();
        return false;
    }
    xone_interface->open(this);
    
    pipe.direction = kUSBIn;
    pipe.interval = 0;
    pipe.type = kUSBInterrupt;
    pipe.maxPacketSize = 0;
    xone_in_pipe = xone_interface->FindNextPipe(nullptr, &pipe);
    if (xone_in_pipe == nullptr)
    {
        release_all();
        return false;
    }
    xone_in_pipe->retain();
    
    pipe.direction = kUSBOut;
    xone_out_pipe = xone_interface->FindNextPipe(nullptr, &pipe);
    if (xone_out_pipe == nullptr)
    {
        release_all();
        return false;
    }
    xone_out_pipe->retain();
    
    xone_in_buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, get_max_packet_size(xone_in_pipe));
    if (xone_in_buffer == nullptr)
    {
        release_all();
        return false;
    }
    
    xone_complete.target = this;
    xone_complete.action = got_data_static;
    xone_complete.parameter = xone_in_buffer;
    
    if (!queue_read())
    {
        release_all();
        return false;
    }
    
    // Minimum byte data needed to light up the Xbox button and allow the controller to take input
    UInt8 write_buffer[2] { 0x05, 0x20 };
    if (!queue_write(write_buffer, 2))
    {
        release_all();
        return false;
    }
    
    xone_connect();
    registerService();
    
    retain();
    
    return true;
}

void com_vestigl_driver_Xone_Driver::xone_connect()
{
    xone_disconnect();
    
    xone_controller = OSTypeAlloc(Xone_Controller);
    if (xone_controller != nullptr)
    {
        const OSString* keys[] = {
            OSString::withCString("SerialOneDevice"),
            OSString::withCString("IOCFPlugInTypes"),
            OSString::withCString("IOKitDebug"),
        };
        const OSObject* objects[] = {
            OSNumber::withNumber((unsigned long long)1, 32),
            getProperty("IOCFPlugInTypes"),
            OSNumber::withNumber((unsigned long long)65535, 32),
        };
        OSDictionary* dictionary = OSDictionary::withObjects(objects, keys, sizeof(keys) / sizeof(keys[0]), 0);
        if (xone_controller->init(dictionary))
        {
            xone_controller->attach(this);
            xone_controller->start(this);
        }
        else
        {
            xone_controller->release();
            xone_controller = nullptr;
        }
    }
}

void com_vestigl_driver_Xone_Driver::xone_disconnect()
{
    if (xone_controller)
    {
        xone_controller->terminate();
        xone_controller->release();
        xone_controller = nullptr;
    }
}



// Static Queue Read / Write methods
#pragma mark - Queue Read / Write

bool com_vestigl_driver_Xone_Driver::queue_read()
{
    IOReturn err;
    bool ret = true;
    
    if ((xone_in_pipe == nullptr) || (xone_in_buffer == nullptr))
    {
        ret = false;
    }
    
    if (ret)
    {
        err = xone_in_pipe->Read(xone_in_buffer, 0, 0, xone_in_buffer->getLength(), &xone_complete);
        if (err == kIOReturnSuccess)
            ret = true;
        else
            ret = false;
    }
    return ret;
}


bool com_vestigl_driver_Xone_Driver::queue_write(const void *bytes, UInt32 length)
{
    IOBufferMemoryDescriptor* out_buffer;
    IOUSBCompletion complete;
    IOReturn err;
    
    out_buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, length);
    if (out_buffer == nullptr)
    {
        IOLog("out_buffer == nullptr");
        return false;
    }
    out_buffer->writeBytes(0, bytes, length);
    complete.target = this;
    complete.action = sent_data_static;
    complete.parameter = out_buffer;

    err = xone_out_pipe->Write(out_buffer, 0, 0, length, &complete);
    if (err == kIOReturnSuccess)
    {
        return true;
    }
    else
    {
        IOLog("err != kIOReturnSuccess");
        return false;
    }
}



// Handles completion of data sends
#pragma mark - Sent Data

void com_vestigl_driver_Xone_Driver::sent_data_static(void* target, void* parameter, IOReturn status, UInt32 bufferSizeRemaining)
{
    if (target != nullptr)
    {
        ((com_vestigl_driver_Xone_Driver*)target)->sent_data(target, parameter, status, bufferSizeRemaining);
    }
}


void com_vestigl_driver_Xone_Driver::sent_data(void* target, void* parameter, IOReturn status, UInt32 bufferSizeRemaining)
{
    IOMemoryDescriptor* memory = (IOMemoryDescriptor*)parameter;
    if (status != kIOReturnSuccess)
    {
        IOLog("write - Error writing: 0x%.8x\n",status);
    }
    memory->release();
}



// Handles recipt and parsing of data. Also applies user settings to data.
#pragma mark - Got Data

void com_vestigl_driver_Xone_Driver::got_data_static(void* target, void* parameter, IOReturn status, UInt32 buffer_size_remaining)
{
    if (target != nullptr)
    {
        ((com_vestigl_driver_Xone_Driver*)target)->got_data(target, parameter, status, buffer_size_remaining);
    }
}


void com_vestigl_driver_Xone_Driver::got_data(void* target, void* parameter, IOReturn status, UInt32 buffer_size_remaining)
{
    if (xone_controller != nullptr)
    {
        LockRequired locker(xone_main_lock);
        IOReturn err;
        bool reread = !isInactive();
        
        switch (status)
        {
            case kIOReturnOverrun:
                IOLog("kIOReturnOverrun, clearing stall.\n");
                if (xone_in_pipe != nullptr)
                    xone_in_pipe->ClearStall();
                // Fall through
            case kIOReturnSuccess:
                if (xone_in_buffer != nullptr)
                {
                    const XONE_IN_REPORT* report = (const XONE_IN_REPORT*)xone_in_buffer->getBytesNoCopy();
                    switch (report->header.message_type)
                    {
                        case in_report:
                            if (report->header.packet_size == in_report_length)
                            {
                                apply_user_settings(xone_in_buffer);
                                err = xone_controller->handleReport(xone_in_buffer, kIOHIDReportTypeInput);
                                if (err != kIOReturnSuccess)
                                {
                                    IOLog("Failed to handle report.\n");
                                }
                            }
                            break;
                        case guide_report:
                            if (report->header.packet_size == guide_report_legnth)
                            {
                                err = xone_controller->handleReport(xone_in_buffer, kIOHIDReportTypeInput);
                                if (err != kIOReturnSuccess)
                                {
                                    IOLog("Failed to handle report.\n");
                                }
                            }
                            break;
                    }
                }
                break;
            case kIOReturnNotResponding:
                reread = false;
                break;
            default:
                reread = false;
                break;
        }
        if (reread)
        {
            queue_read();
        }
    }
}


static inline SInt16 get_absolute(SInt16 value)
{
    SInt16 reverse;
    
#ifdef __LITTLE_ENDIAN__
    reverse=value;
#elif __BIG_ENDIAN__
    reverse=((value&0xFF00)>>8)|((value&0x00FF)<<8);
#else
#error Unknown CPU byte order
#endif
    return (reverse<0)?~reverse:reverse;
}


void com_vestigl_driver_Xone_Driver::apply_user_settings(IOBufferMemoryDescriptor *buffer)
{
    XONE_IN_REPORT* report = (XONE_IN_REPORT*)buffer->getBytesNoCopy();
    if (xone_invert_left_x)
        report->left_stick_x = ~report->left_stick_x;
    if (xone_invert_right_x)
        report->right_stick_x = ~report->right_stick_x;
    if (!xone_invert_left_y)
        report->left_stick_y = ~report->left_stick_y;
    if (!xone_invert_right_y)
        report->right_stick_y = ~report->right_stick_y;
    
    if (xone_left_thumb_deadzone != 0)
    {
        if (get_absolute(report->left_stick_x) < xone_left_thumb_deadzone)
            report->left_stick_x = 0;
        if (get_absolute(report->left_stick_y) < xone_left_thumb_deadzone)
            report->left_stick_y = 0;
    }
    if (xone_right_thumb_deadzone != 0)
    {
        if (get_absolute(report->right_stick_x) < xone_right_thumb_deadzone)
            report->right_stick_x = 0;
        if (get_absolute(report->right_stick_y) < xone_right_thumb_deadzone)
            report->right_stick_y = 0;
    }
    if (xone_trigger_deadzone != 0)
    {
        if (report->trigger_left != 0 && report->trigger_right != 0)
        {
            if (report->trigger_left < xone_trigger_deadzone)
            {
                report->trigger_left = 0;
            }
            if (report->trigger_right < xone_trigger_deadzone)
            {
                report->trigger_right = 0;
            }
        }
    }
}



// Used in the stop process, when start returns a fail.
#pragma mark - Stop

void com_vestigl_driver_Xone_Driver::stop(IOService* provider)
{
    IOLog("Stopping\n");
    release_all();
    super::stop(provider);
    release();
}


void com_vestigl_driver_Xone_Driver::release_all()
{
//    LockRequired locker(xone_main_lock);
    
    xone_disconnect();
    
    if (xone_device)
    {
        if (xone_device->isOpen())
        {
            xone_device->close(this);
        }
        xone_device = nullptr;
    }
    
    if (xone_in_pipe)
    {
        xone_in_pipe->Abort();
        xone_in_pipe->release();
        xone_in_pipe = nullptr;
    }
    
    if (xone_out_pipe)
    {
        xone_out_pipe->Abort();
        xone_out_pipe->release();
        xone_out_pipe = nullptr;
    }
    
    if (xone_interface)
    {
        if (xone_interface->isOpen())
        {
            xone_interface->close(this);
        }
        xone_interface = nullptr;
    }
    
    if (xone_in_buffer)
    {
        xone_in_buffer->release();
        xone_in_buffer = nullptr;
    }
}



// Called after failed probe, or after a stop.
#pragma mark - Free

void com_vestigl_driver_Xone_Driver::free()
{
    IOLog("Free!\n");
    IOLockFree(xone_main_lock);
    release_all();
    super::free();
}



// These have to be implemented in order to open using this
#pragma mark - IOService Methods

IOReturn com_vestigl_driver_Xone_Driver::message(UInt32 type, IOService* provider, void* argument)
{
    switch (type)
    {
        case kIOMessageServiceIsTerminated:
            IOLog("kIOMessageServiceIsTerminated\n");
            release_all();
            return kIOReturnSuccess;
        case kIOMessageServiceIsRequestingClose:
            IOLog("kIOMessageServiceIsRequestingClose\n");
            // Ignore this, otherwise the controller will close prematurely.
            return kIOReturnUnsupported;
        default:
            return super::message(type,provider,argument);
    }
}


IOReturn com_vestigl_driver_Xone_Driver::setProperties(OSObject* properties)
{
    OSDictionary* dictionary;
    
    dictionary = OSDynamicCast(OSDictionary, properties);
    if (dictionary != nullptr)
    {
        setProperty(kDriverSettingKey, dictionary);
        read_settings();
        return kIOReturnSuccess;
    }
    else
        return kIOReturnBadArgument;
}




void com_vestigl_driver_Xone_Driver::read_settings()
{
    OSDictionary *data_dictionary = OSDynamicCast(OSDictionary, getProperty(kDriverSettingKey));
    OSBoolean *value;
    OSNumber *number;
    
    if (data_dictionary)
    {
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("l_inv_x"));
        if (value)
            xone_invert_left_x = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("l_inv_y"));
        if (value)
            xone_invert_left_y = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("r_inv_x"));
        if (value)
            xone_invert_right_x = value->getValue();
        
        value = OSDynamicCast(OSBoolean, data_dictionary->getObject("r_inv_y"));
        if (value)
            xone_invert_right_y = value->getValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("l_stick_d"));
        if (number)
            xone_left_thumb_deadzone = number->unsigned32BitValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("r_stick_d"));
        if (number)
            xone_right_thumb_deadzone = number->unsigned32BitValue();
        
        number = OSDynamicCast(OSNumber, data_dictionary->getObject("t_d"));
        if (number)
            xone_trigger_deadzone = number->unsigned32BitValue();
    }
}
