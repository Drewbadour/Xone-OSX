//
//  Xone_Controller.cpp
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

//#include <IOKit/usb/IOUSBDevice.h>
//#include "Controller.h"
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBInterface.h>
#include "Xone_Controller.h"
#include "ControlStruct.h"
namespace HID_One {
#include "xonehid.h"
}
#include "Xone_Driver.h"

OSDefineMetaClassAndStructors(Xone_Controller, IOHIDDevice)

static com_vestigl_driver_Xone_Driver* GetOwner(IOService *us)
{
    IOService *prov;
    
    prov = us->getProvider();
    if (prov == NULL)
        return NULL;
    return OSDynamicCast(com_vestigl_driver_Xone_Driver, prov);
}

static IOUSBDevice* GetOwnerProvider(const IOService *us)
{
    IOService *prov, *provprov;
    
    prov = us->getProvider();
    if (prov == NULL)
        return NULL;
    provprov = prov->getProvider();
    if (provprov == NULL)
        return NULL;
    return OSDynamicCast(IOUSBDevice, provprov);
}

bool Xone_Controller::start(IOService *provider)
{
    if (OSDynamicCast(com_vestigl_driver_Xone_Driver, provider) == NULL)
        return false;
    return IOHIDDevice::start(provider);
}

IOReturn Xone_Controller::setProperties(OSObject* properties)
{
    com_vestigl_driver_Xone_Driver* owner = GetOwner(this);
    if (owner == NULL)
        return kIOReturnUnsupported;
    return owner->setProperties(properties);
}

// Returns the HID descriptor for this device
IOReturn Xone_Controller::newReportDescriptor(IOMemoryDescriptor** descriptor) const
{
    IOBufferMemoryDescriptor* buffer;
    
    buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(HID_One::ReportDescriptor));
    if (buffer == NULL)
        return kIOReturnNoResources;
    buffer->writeBytes(0, HID_One::ReportDescriptor, sizeof(HID_One::ReportDescriptor));
    *descriptor = buffer;
    return kIOReturnSuccess;
}

// Handles a message from the userspace IOHIDDeviceInterface122::setReport function
IOReturn Xone_Controller::setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options)
{
    XONE_HEADER* header;
    report->readBytes(0, header, 4);
    switch(header->message_type)
    {
        case force_feedback:  // Set force feedback
            if((header->packet_size != report->getLength()) || (header->packet_size != 0x04))
                return kIOReturnUnsupported;
            else
            {
                // Handle force feedback here
            }
            return kIOReturnSuccess;
        default:
//            IOLog("Unknown escape %d\n", data[0]);
            return kIOReturnUnsupported;
    }
}

// Get report
IOReturn Xone_Controller::getReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options)
{
    // Doesn't do anything yet
    return kIOReturnUnsupported;
}

// Returns the string for the specified index from the USB device's string list, with an optional default
OSString* Xone_Controller::getDeviceString(UInt8 index, const char* def) const
{
    IOReturn err;
    char buf[1024];
    const char* string;
    
    err = GetOwnerProvider(this)->GetStringDescriptor(index, buf, sizeof(buf));
    if (err == kIOReturnSuccess) string = buf;
    else {
        if (def == NULL) string="Unknown";
        else string = def;
    }
    return OSString::withCString(string);
}

OSString* Xone_Controller::newManufacturerString() const
{
    return getDeviceString(GetOwnerProvider(this)->GetManufacturerStringIndex());
}

OSNumber* Xone_Controller::newPrimaryUsageNumber() const
{
    return OSNumber::withNumber(HID_One::ReportDescriptor[3], 8);
}

OSNumber* Xone_Controller::newPrimaryUsagePageNumber() const
{
    return OSNumber::withNumber(HID_One::ReportDescriptor[1], 8);
}

OSNumber* Xone_Controller::newProductIDNumber() const
{
    return OSNumber::withNumber(GetOwnerProvider(this)->GetProductID(),16);
}

OSString* Xone_Controller::newProductString() const
{
    return getDeviceString(GetOwnerProvider(this)->GetProductStringIndex());
}

OSString* Xone_Controller::newSerialNumberString() const
{
    return getDeviceString(GetOwnerProvider(this)->GetSerialNumberStringIndex());
}

OSString* Xone_Controller::newTransportString() const
{
    return OSString::withCString("USB");
}

OSNumber* Xone_Controller::newVendorIDNumber() const
{
    return OSNumber::withNumber(GetOwnerProvider(this)->GetVendorID(),16);
}

OSNumber* Xone_Controller::newLocationIDNumber() const
{
    IOUSBDevice *device;
    OSNumber *number;
    UInt32 location;
    
    device = GetOwnerProvider(this);
    if (device)
    {
        if ((number = OSDynamicCast(OSNumber, device->getProperty("locationID"))))
        {
            location = number->unsigned32BitValue();
        }
        else
        {
            // Make up an address
            if ((number = OSDynamicCast(OSNumber, device->getProperty("USB Address"))))
                location |= number->unsigned8BitValue() << 24;
            
            if ((number = OSDynamicCast(OSNumber, device->getProperty("idProduct"))))
                location |= number->unsigned8BitValue() << 16;
        }
    }
    
    return (location != 0) ? OSNumber::withNumber(location, 32) : 0;
}

