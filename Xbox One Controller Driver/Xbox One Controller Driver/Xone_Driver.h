//
//  Xone_Driver.h
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

#ifndef __XONEDRIVER_H__
#define __XONEDRIVER_H__

#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/IOLib.h>
#include "ControlStruct.h"

class Xone_Controller;

class com_vestigl_driver_Xone_Driver : public IOService
{
OSDeclareDefaultStructors(com_vestigl_driver_Xone_Driver)
private:
    IOUSBDevice* xone_device;
    IOUSBInterface* xone_interface;
    IOUSBPipe* xone_in_pipe;
    IOUSBPipe* xone_out_pipe;
    IOBufferMemoryDescriptor* xone_in_buffer;
    Xone_Controller* xone_controller;
    IOLock* xone_main_lock;
    IOUSBCompletion xone_complete;
    
    bool xone_invert_left_x;
    bool xone_invert_left_y;
    bool xone_invert_right_x;
    bool xone_invert_right_y;
    SInt16 xone_left_thumb_deadzone;
    SInt16 xone_right_thumb_deadzone;
    UInt16 xone_trigger_deadzone;
    
    
protected:
    /*! @function get_max_packet_size
     @abstract Returns max packet size.
     @discussion Should return 64.
     @result Max packet size. */
    static UInt32 get_max_packet_size(IOUSBPipe* pipe);
    
    /*! @function sent_data_static
     @abstract Callback after data sent.
     @discussion Calls sent_data. */
    static void sent_data_static(void* target, void *parameter, IOReturn status, UInt32 buffer_size_remaining);
    
    /*! @function got_data_static
     @abstract Callback when data recieved.
     @discussion Calls got_data. */
    static void got_data_static(void* target, void *parameter, IOReturn status, UInt32 buffer_size_remaining);
    
    /*! @function queue_read
     @abstract Queues a read of data sent from the controller.
     @discussion Different packets send from the controller are documented in "Data Notes.rtf"
     @result <code>true</code> if read succeeded. <code>false</code> if the read failed.*/
    bool queue_read();
    
    /*! @function release_all
     @abstract Releases all newed memory.
     @discussion Checks values and then subsequently releases them. */
    void release_all();
    
    /*! @function xone_connect
     @abstract News the Xbox One controller device.
     @discussion Inits dictionaries and releases the controller if it is instantiated improperly. */
    void xone_connect();
    
    /*! @function xone_disconnect
     @abstract Custom deconstruction of Xbox One device.
     @discussion Terminates and releases the controller. */
    void xone_disconnect();
    
    /*! @function change_to_x360_packet
     @abstract Reorder the bytes of the packet to be identical to that of the Xbox 360 packet
     @discussion Not sure if this is the most efficient way to do this.
     @param buffer The memory buffer from the recieved data callback. */
    void change_to_x360_packet(IOBufferMemoryDescriptor *buffer);
    
    /*! @function apply_user_settings
     @abstract Applies settings to controller data.
     @discussion Inverts X and Y axes based on settings. Applies deadzones based on user settings.
     @param buffer The memory buffer from the recieved data callback. */
    void apply_user_settings(IOBufferMemoryDescriptor *buffer);
    
    /*! @function read_settings
     @abstract Parses the settings data.
     @discussion Data is stored in a dictionary, and each value is accessed by a key. */
    void read_settings();
    
    
public:
    virtual void free(void);
    virtual bool init(OSDictionary *dictionary = 0);
    virtual IOService* probe(IOService *provider, SInt32 *score);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    
    // IOService methods
    virtual IOReturn setProperties(OSObject *properties);
    virtual IOReturn message(UInt32 type, IOService *provider, void *argument);
    
    /*! @function queue_write
     @abstract Queues a write to the Xbox One controller.
     @discussion Only used for writing the init bytes to the controller.
     @param bytes The data to be sent.
     @param length The count of bytes to send.
     @result <code>true</code> if write succeeded, <code>false</code> if the write failed. */
    bool queue_write(const void* bytes, UInt32 length);
    
    /*! @function sent_data
     @abstract Callback after data sent.
     @discussion Releases memory. Writes and error to the log if the write failed. */
    virtual void sent_data(void* target, void* parameter, IOReturn status, UInt32 buffer_size_remaining);
    /*! @function got_data
     @abstract Callback when data recieved.
     @discussion Clears stalls. Checks report type and checks that the length is accurate before
        sending value to the report descriptor Queues up a subsequent read. */
    virtual void got_data(void* target, void* parameter, IOReturn status, UInt32 buffer_size_remaining);
};

#endif /* __XONEDRIVER_H__ */