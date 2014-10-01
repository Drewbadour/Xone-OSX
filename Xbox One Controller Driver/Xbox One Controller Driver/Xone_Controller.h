//
//  Xone_Controller.h
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

#include <IOKit/hid/IOHIDDevice.h>

class Xone_Controller : public IOHIDDevice
{
OSDeclareDefaultStructors(Xone_Controller)
private:
    /*! @function getDeviceString
     @abstract Returns the string for the specified index from the USB device's string list, with an optional default
     @result Device string. Unknown if it cannot be found. */
    OSString* getDeviceString(UInt8 index, const char* def = NULL) const;
    
public:
    /*! @function start
     @abstract Calls the com_vestigl_driver_Xone_Driver setProperties
     @param provider Instance of com_vestigl_driver_Xone_Driver. Passed to IOHIDDevice::start.
     @result <code>true</code> if success. <code>false</code> if failure. */
    virtual bool start(IOService* provider);
    
    /*! @function setProperties
     @abstract Calls the com_vestigl_driver_Xone_Driver setProperties
     @param properties Passed to parent setProperties
     @result kIOReturnSuccess on success, or an error return otherwise. */
    virtual IOReturn setProperties(OSObject* properties);
    
    /*! @function newReportDescriptor
     @abstract Create and return a new memory descriptor that describes the report descriptor for the HID device.
     @discussion Returns the report descriptor located in "xonehid.h"
     @param descriptor Pointer to the memory descriptor returned. This memory descriptor will be released by the caller.
     @result kIOReturnSuccess on success, or an error return otherwise. */
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const;
    
    /*! @function setReport
     @abstract Handles a message from the userspace IOHIDDeviceInterface122::setReport function
     @discussion Handles such functions sent to the controller such as force feedback.
     @param reportType The report type.
     @param options The lower 8 bits will represent the Report ID.  The other 24 bits are options to specify the request.
     @result kIOReturnSuccess on success, or an error return otherwise. */
    virtual IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options = 0);
    
    /*! @function getReport
     @abstract Get a report from the HID device.
     @discussion Does nothing.
     @param reportType The report type.
     @param options The lower 8 bits will represent the Report ID.  The other 24 bits are options to specify the request.
     @result kIOReturnSuccess on success, or an error return otherwise. */
    virtual IOReturn getReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    
    /*! @function newManufacturerString
     @abstract Returns a string object that describes the manufacturer of the HID device.
     @result A string object. The caller must decrement the retain count on the object returned. */
    virtual OSString* newManufacturerString() const;
    
    /*! @function newPrimaryUsageNumber
     @abstract Returns a number object that describes the primary usage of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSNumber* newPrimaryUsageNumber() const;
    
    /*! @function newPrimaryUsagePageNumber
     @abstract Returns a number object that describes the primary usage page of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSNumber* newPrimaryUsagePageNumber() const;
    
    /*! @function newProductIDNumber
     @abstract Returns a number object that describes the product ID of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSNumber* newProductIDNumber() const;
    
    /*! @function newProductString
     @abstract Returns a string object that describes the product of the HID device.
     @result A string object. The caller must decrement the retain count on the object returned. */
    virtual OSString* newProductString() const;
    
    /*! @function newSerialNumberString
     @abstract Returns a string object that describes the serial number of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSString* newSerialNumberString() const;
    
    /*! @function newTransportString
     @abstract Returns a string object that describes the transport layer used by the HID device.
     @result A string object. The caller must decrement the retain count on the object returned. */
    virtual OSString* newTransportString() const;
    
    /*! @function newVendorIDNumber
     @abstract Returns a number object that describes the vendor ID of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSNumber* newVendorIDNumber() const;
    
    /*! @function newLocationIDNumber
     @abstract Returns a number object that describes the location ID of the HID device.
     @result A number object. The caller must decrement the retain count on the object returned. */
    virtual OSNumber* newLocationIDNumber() const;
};