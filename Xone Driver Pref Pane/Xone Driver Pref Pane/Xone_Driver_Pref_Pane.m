//
//  Xone_Driver_Pref_Pane.m
//  Xone Driver Pref Pane
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

#import "Xone_Driver_Pref_Pane.h"
@import GameController;

@implementation Xone_Driver_Pref_Pane
{
    NSRect img_pos;
    
    NSNumber* max_stick;
    NSNumber* min_stick;
    NSNumber* max_trigger;
    NSNumber* min_trigger;
    
    CFStringRef appID;
    
    NSMutableArray* joysticks;
    
    int selected_controller;
}

const int left_stick_deadzone_default = 7849;
const int right_stick_deadzone_default = 8689;
const int trigger_deadzone_default = 120;
NSString* const invert_x_str = @"Invert X-Axis";
NSString* const invert_y_str = @"Invert Y-Axis";

#pragma mark - Initialization

- (id)initWithBundle:(NSBundle *)bundle
{
    if (self = [super initWithBundle:bundle])
    {
        appID = CFSTR("com.vestigl.driver.Xone-Driver");
        
        max_stick = [NSNumber numberWithInt:32767]; //INT16_MAX
        min_stick = [NSNumber numberWithInt:-32768]; // INT16_MIN
        max_trigger = [NSNumber numberWithInt:1023];
        min_trigger = [NSNumber numberWithInt:0];
        
//        joysticks = [NSMutableArray arrayWithArray:[DDHidJoystick allJoysticks]];
//        for (long i = [joysticks count] - 1; i >= 0; i--)
//        {
//            if ([joysticks[i] productId] != 721)
//                [joysticks removeObjectAtIndex:i];
//            else if ([joysticks[i] vendorId] != 1118)
//                [joysticks removeObjectAtIndex:i];
//        }
//        
//        [joysticks makeObjectsPerformSelector:@selector(setDelegate:) withObject:self];
//        [joysticks makeObjectsPerformSelector:@selector(startListening) withObject:nil];
        
        img_pos = NSMakeRect(-139, -122, 901, 771);
    }
    
    return self;
}

- (void)mainViewDidLoad
{
    [self loadBooleanValue:CFSTR("left_invert_x") forButton:left_invert_x];
    [self loadBooleanValue:CFSTR("left_invert_y") forButton:left_invert_y];
    [self loadBooleanValue:CFSTR("right_invert_x") forButton:right_invert_x];
    [self loadBooleanValue:CFSTR("right_invert_y") forButton:right_invert_y];
    
    [self loadStringValue:CFSTR("left_deadzone") forTextField:left_stick_deadzone withDefault:[NSString stringWithFormat:@"%i", left_stick_deadzone_default]];
    [self loadStringValue:CFSTR("right_deadzone") forTextField:right_stick_deadzone withDefault:[NSString stringWithFormat:@"%i", right_stick_deadzone_default]];
    [self loadStringValue:CFSTR("trigger_deadzone") forTextField:trigger_deadzone withDefault:[NSString stringWithFormat:@"%i", trigger_deadzone_default]];
    
    [self refresh:0];
    for (int i = 0; i < [joysticks count]; i++)
        [controller_combo addItemWithObjectValue:[NSString stringWithFormat:@"Controller %d", i + 1]];
    [controller_combo setNumberOfVisibleItems:[joysticks count]];
    if ([joysticks count] > 0)
        [controller_combo selectItemAtIndex:0];
}

- (IBAction)refresh:(id)sender
{
    joysticks = [NSMutableArray arrayWithArray:[DDHidJoystick allJoysticks]];
    NSLog(@"JOYSTICK COUNT - %lu\n", (unsigned long)[joysticks count]);
    for (long i = [joysticks count] - 1; i >= 0; i--)
    {
        if ([joysticks[i] vendorId] == 1118)
        {
            if ([joysticks[i] productId] == 721) {
                controllerType = XONE;
                NSLog(@"Xone Controller\n");
            }
            else if ([joysticks[i] productId] == 654) {
                controllerType = X360;
                NSLog(@"X360 Controller\n");
            }
            else {
                [joysticks removeObjectAtIndex:i];
                NSLog(@"Remove product\n");
            }
        }
        else {
            [joysticks removeObjectAtIndex:i];
            NSLog(@"Remove vendor\n");
        }
    }
    
    [joysticks makeObjectsPerformSelector:@selector(setDelegate:) withObject:self];
    [joysticks makeObjectsPerformSelector:@selector(startListening) withObject:nil];
    
    for (int i = 0; i < [joysticks count]; i++)
        [controller_combo addItemWithObjectValue:[NSString stringWithFormat:@"Controller %d", i + 1]];
    [controller_combo setNumberOfVisibleItems:[joysticks count]];
    if ([joysticks count] > 0)
        [controller_combo selectItemAtIndex:0];
}

#pragma mark - Loading

- (void)loadBooleanValue:(CFStringRef)str forButton:(NSButton*)object
{
    CFPropertyListRef value = CFPreferencesCopyValue(str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    if (value)
    {
        if ((CFGetTypeID(value) == CFBooleanGetTypeID()))
            [object setState:CFBooleanGetValue(value)];
        CFRelease(value);
    }
    else
        [object setState:NO];
}

- (void)loadStringValue:(CFStringRef)str forTextField:(NSTextField*)object withDefault:(NSString*)def
{
    CFPropertyListRef value = CFPreferencesCopyValue(str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    if (value)
    {
        if (CFGetTypeID(value) == CFStringGetTypeID())
            [object setStringValue:[NSString stringWithFormat:@"%@", value]];
        CFRelease(value);
    }
    else
        [object setStringValue:def];
}

#pragma mark - Saving

- (IBAction)checkbox_clicked:(id)sender
{
    CFStringRef str;
    NSButton* from = (NSButton*)sender;
    
    if ([from imagePosition] == NSImageLeft) // Left Stick
    {
        if ([[from title] isEqualToString:invert_x_str])
            str = CFSTR("left_invert_x");
        else if ([[from title] isEqualToString:invert_y_str])
            str = CFSTR("left_invert_y");
        else
            str = CFSTR("error");
    }
    else if ([from imagePosition] == NSImageRight)
    {
        if ([[from title] isEqualToString:invert_x_str])
            str = CFSTR("right_invert_x");
        else if ([[from title] isEqualToString:invert_y_str])
            str = CFSTR("right_invert_y");
        else
            str = CFSTR("error");
    }
    else
        str = CFSTR("error");
        
    if ([sender state] == YES)
        CFPreferencesSetValue(str, kCFBooleanTrue, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    else
        CFPreferencesSetValue(str, kCFBooleanFalse, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
}

- (void)didUnselect
{
    // Save for Preference Pane
    CFNotificationCenterRef center;
    NSString* str;
    
    str = [left_stick_deadzone stringValue];
    if ([str isEqual:@""])
        str = [NSString stringWithFormat:@"%i", 7849];
    CFPreferencesSetValue(CFSTR("left_deadzone"), CFStringCreateWithCString(kCFAllocatorDefault, [str UTF8String], kCFStringEncodingUTF8), appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    
    str = [right_stick_deadzone stringValue];
    if ([str isEqual:@""])
        str = [NSString stringWithFormat:@"%i", 8689];
    CFPreferencesSetValue(CFSTR("right_deadzone"), CFStringCreateWithCString(kCFAllocatorDefault, [str UTF8String], kCFStringEncodingUTF8), appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    
    str = [trigger_deadzone stringValue];
    if ([str isEqual:@""])
        str = [NSString stringWithFormat:@"%i", 120];
    CFPreferencesSetValue(CFSTR("trigger_deadzone"), CFStringCreateWithCString(kCFAllocatorDefault, [str UTF8String], kCFStringEncodingUTF8), appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

    CFPreferencesSynchronize(appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

    center = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(center, CFSTR("Preferences Changed"), appID, NULL, TRUE);
    
    // Save for driver
    CFDictionaryRef dict;
    CFStringRef keys[7];
    CFTypeRef values[7];
    UInt16 i;
    
    keys[0] = CFSTR("l_inv_x");
    values[0] = ([left_invert_x state] == NSOnState) ? kCFBooleanTrue : kCFBooleanFalse;
    
    keys[1] = CFSTR("l_inv_y");
    values[1] = ([left_invert_y state] == NSOnState) ? kCFBooleanTrue : kCFBooleanFalse;
    
    keys[2] = CFSTR("r_inv_x");
    values[2] = ([right_invert_x state] == NSOnState) ? kCFBooleanTrue : kCFBooleanFalse;
    
    keys[3] = CFSTR("r_inv_y");
    values[3] = ([right_invert_y state] == NSOnState) ? kCFBooleanTrue : kCFBooleanFalse;
    
    keys[4] = CFSTR("l_stick_d");
    i = [left_stick_deadzone intValue];
    values[4] = CFNumberCreate(NULL, kCFNumberShortType, &i);
    
    keys[5] = CFSTR("r_stick_d");
    i = [right_stick_deadzone intValue];
    values[5] = CFNumberCreate(NULL, kCFNumberShortType, &i);
    
    keys[6] = CFSTR("t_d");
    i = [trigger_deadzone intValue];
    values[6] = CFNumberCreate(NULL, kCFNumberShortType, &i);
    
    dict = CFDictionaryCreate(NULL, (const void**)keys, (const void**)values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if ([controller_combo indexOfSelectedItem] > 0)
        IORegistryEntrySetCFProperties([joysticks[[controller_combo indexOfSelectedItem]] ioDevice], dict);
}

#pragma mark - Reset

- (IBAction)reset_clicked:(id)sender;
{
    [left_stick_deadzone setStringValue:[NSString stringWithFormat:@"%i", left_stick_deadzone_default]];
    [right_stick_deadzone setStringValue:[NSString stringWithFormat:@"%i", right_stick_deadzone_default]];
    [trigger_deadzone setStringValue:[NSString stringWithFormat:@"%i", trigger_deadzone_default]];
    
    [left_invert_x setState:0];
    [left_invert_y setState:0];
    [right_invert_x setState:0];
    [right_invert_y setState:0];
}



#pragma mark - Joystick Delegate

- (void)ddhidJoystick:(DDHidJoystick *)joystick buttonDown:(unsigned)buttonNumber
{
    if ([joystick locationId] == [joysticks[[controller_combo indexOfSelectedItem]] locationId])
    {
//        NSLog(@"Button Number - %d\n", buttonNumber);
        switch (buttonNumber)
        {
            case 0:
                [a_img setHidden:NO];
                break;
            case 1:
                [b_img setHidden:NO];
                break;
            case 2:
                [x_img setHidden:NO];
                break;
            case 3:
                [y_img setHidden:NO];
                break;
            case 4:
                [lb_img setHidden:NO];
                break;
            case 5:
                [rb_img setHidden:NO];
                break;
            case 6:
                if (controllerType == X360)
                    [lsc_img setHidden:NO];
                else if (controllerType == XONE)
                    [view_img setHidden:NO];
                break;
            case 7:
                if (controllerType == X360)
                    [rsc_img setHidden:NO];
                else if (controllerType == XONE)
                    [menu_img setHidden:NO];
                break;
            case 8:
                if (controllerType == X360)
                    [menu_img setHidden:NO];
                else if (controllerType == XONE)
                    [lsc_img setHidden:NO];
                break;
            case 9:
                if (controllerType == X360)
                    [view_img setHidden:NO];
                else if (controllerType == XONE)
                    [rsc_img setHidden:NO];
                break;
            case 10:
                // 360 Location of Guide Button
                [guide_img setHidden:NO];
                break;
            case 11:
                if (controllerType == X360)
                    [up_img setHidden:NO];
                // Sync on XONE. No image.
                break;
            case 12:
                if (controllerType == X360)
                    [down_img setHidden:NO];
                else if (controllerType == XONE)
                    [up_img setHidden:NO];
                break;
            case 13:
                if (controllerType == X360)
                    [left_img setHidden:NO];
                else if (controllerType == XONE)
                    [down_img setHidden:NO];
                break;
            case 14:
                if (controllerType == X360)
                    [right_img setHidden:NO];
                else if (controllerType == XONE)
                    [left_img setHidden:NO];
                break;
            case 15:
                if (controllerType == X360)
                    [guide_img setHidden:NO];
                else if (controllerType == XONE)
                    [right_img setHidden:NO];
                break;
            default:
                break;
        }
    }
}

- (void)ddhidJoystick:(DDHidJoystick *)joystick buttonUp:(unsigned)buttonNumber
{
    if ([joystick locationId] == [joysticks[[controller_combo indexOfSelectedItem]] locationId])
    {
        switch (buttonNumber)
        {
            case 0:
                [a_img setHidden:YES];
                break;
            case 1:
                [b_img setHidden:YES];
                break;
            case 2:
                [x_img setHidden:YES];
                break;
            case 3:
                [y_img setHidden:YES];
                break;
            case 4:
                [lb_img setHidden:YES];
                break;
            case 5:
                [rb_img setHidden:YES];
                break;
            case 6:
                if (controllerType == X360)
                    [lsc_img setHidden:YES];
                else if (controllerType == XONE)
                    [view_img setHidden:YES];
                break;
            case 7:
                if (controllerType == X360)
                    [rsc_img setHidden:YES];
                else if (controllerType == XONE)
                    [menu_img setHidden:YES];
                break;
            case 8:
                if (controllerType == X360)
                    [menu_img setHidden:YES];
                else if (controllerType == XONE)
                    [lsc_img setHidden:YES];
                break;
            case 9:
                if (controllerType == X360)
                    [view_img setHidden:YES];
                else if (controllerType == XONE)
                    [rsc_img setHidden:YES];
//                break;
            case 10:
                // 360 Location of Guide Button
                [guide_img setHidden:YES];
                break;
            case 11:
                if (controllerType == X360)
                    [up_img setHidden:YES];
                // Sync on XONE. No image.
                break;
            case 12:
                if (controllerType == X360)
                    [down_img setHidden:YES];
                else if (controllerType == XONE)
                    [up_img setHidden:YES];
                break;
            case 13:
                if (controllerType == X360)
                    [left_img setHidden:YES];
                else if (controllerType == XONE)
                    [down_img setHidden:YES];
                break;
            case 14:
                if (controllerType == X360)
                    [right_img setHidden:YES];
                else if (controllerType == XONE)
                    [left_img setHidden:YES];
                break;
            case 15:
                if (controllerType == X360)
                    [guide_img setHidden:YES];
                else if (controllerType == XONE)
                    [right_img setHidden:YES];
                break;
            default:
                break;
        }
    }
}

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick xChanged:(int)value
{
    if ([joystick locationId] == [joysticks[[controller_combo indexOfSelectedItem]] locationId])
    {
        if(abs(value) > [left_stick_deadzone intValue])
        {
            // Left Stick Light
            double offset = 11 * value / [max_stick intValue];
            if ([left_invert_x state])
                offset = -offset;
            [ls_img setFrame:NSMakeRect(img_pos.origin.x + offset, [ls_img frame].origin.y, img_pos.size.width, img_pos.size.height)];
        }
        else
        {
            [ls_img setFrame:NSMakeRect(img_pos.origin.x, [ls_img frame].origin.y, img_pos.size.width, img_pos.size.height)];
        }
    }
}

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick yChanged:(int)value
{
    if ([joystick locationId] == [joysticks[[controller_combo indexOfSelectedItem]] locationId])
    {
        if(abs(value) > [left_stick_deadzone intValue])
        {
            // Left Stick Light
            double offset = 11 * value / [max_stick intValue];
            if (![left_invert_y state])
                offset = -offset;
            [ls_img setFrame:NSMakeRect([ls_img frame].origin.x, img_pos.origin.y + offset, img_pos.size.width, img_pos.size.height)];
        }
        else
        {
            [ls_img setFrame:NSMakeRect([ls_img frame].origin.x, img_pos.origin.y, img_pos.size.width, img_pos.size.height)];
        }
    }
}

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick otherAxis:(unsigned)otherAxis valueChanged:(int)value
{
    if ([joystick locationId] == [joysticks[[controller_combo indexOfSelectedItem]] locationId])
    {
        // Xbox One
        if (stick == 0)
        {
            // Left Trigger
            if (otherAxis == 0)
            {
                [self leftTrigger:value];
            }
            if (otherAxis == 1)
            {
                [self rightTrigger:value];
            }
            if (otherAxis == 2)
            {
                [self rightStickX:value];
            }
            if (otherAxis == 3)
            {
                [self rightStickY:value];
            }
        }
        
        // 360 - Right Stick
        if (stick == 1)
        {
            if (otherAxis == 0)
            {
                    [self rightStickX:value];
            }
            else if (otherAxis == 1)
            {
                    [self rightStickY:value];
            }
        }
        // 360 - Triggers
        if (stick == 2)
        {
            if (otherAxis == 0)
            {
                    [self leftTrigger:value];
            }
            if (otherAxis == 1)
            {
                    [self rightTrigger:value];
            }
        }
    }
}

- (void)rightStickX:(int)value
{
    if (abs(value) > [right_stick_deadzone intValue])
    {
        double offset = 11 * value / [max_stick intValue];
        if ([right_invert_x state])
            offset = -offset;
        [rs_img setFrame:NSMakeRect(img_pos.origin.x + offset, [rs_img frame].origin.y, img_pos.size.width, img_pos.size.height)];
    }
    else
    {
        [rs_img setFrame:NSMakeRect(img_pos.origin.x, [rs_img frame].origin.y, img_pos.size.width, img_pos.size.height)];
    }
}

- (void)rightStickY:(int)value
{
    if (abs(value) > [right_stick_deadzone intValue])
    {
        double offset = 11 * value / [max_stick intValue];
        if (![right_invert_y state])
            offset = -offset;
        [rs_img setFrame:NSMakeRect([rs_img frame].origin.x, img_pos.origin.y + offset, img_pos.size.width, img_pos.size.height)];
    }
    else
    {
        [rs_img setFrame:NSMakeRect([rs_img frame].origin.x, img_pos.origin.y, img_pos.size.width, img_pos.size.height)];
    }
}

- (void)leftTrigger:(int)value
{
    if (value > [trigger_deadzone intValue])
    {
        [lt_img setHidden:NO];
    }
    else
    {
        [lt_img setHidden:YES];
    }
}

- (void)rightTrigger:(int)value
{
    if (value > [trigger_deadzone intValue])
    {
        [rt_img setHidden:NO];
    }
    else
    {
        [rt_img setHidden:YES];
    }
}

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick povNumber:(unsigned)povNumber valueChanged:(int)value
{
    // Hat reported as buttons
}


@end
