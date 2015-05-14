//
// Xone_Driver_Pref_Pane.m
// Xone Driver Pref Pane
//
// Created by Drew Mills on 10/01/14.
// Copyright (c) 2014 Drew Mills
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

@import GameController;
#import <PreferencePanes/PreferencePanes.h>
#import <QuartzCore/QuartzCore.h>
#import <DDHidLib/DDHidLib.h>

#import "Xone_Driver_Pref_Pane.h"

//
// NOTE: I didn't see where this is used anywhere
//
enum class {
	xo_l_inv_x,
	xo_l_inv_y,
	xo_r_inv_x,
	xo_r_inv_y,
	xo_l_stick_d,
	xo_r_stick_d,
	xo_t_d,
};

//
// Controller type (Xbox 360 or Xbox One)
//
typedef NS_ENUM (NSInteger, ControllerType) {
	X360,
	XONE,
};

//
// class catagory (to declare local (non-exported) methods)
//
@interface Xone_Driver_Pref_Pane ()

- (void)	mainViewDidLoad;
- (IBAction)reset_clicked:(id)sender;
- (IBAction)checkbox_clicked:(id)sender;

@end

//
// class implementation
//
@implementation Xone_Driver_Pref_Pane
{
	IBOutlet NSTabViewItem *general_tab;
	IBOutlet NSTextField *left_stick_deadzone;
	IBOutlet NSTextField *right_stick_deadzone;
	IBOutlet NSTextField *trigger_deadzone;
	IBOutlet NSButton *left_invert_x;
	IBOutlet NSButton *left_invert_y;
	IBOutlet NSButton *right_invert_x;
	IBOutlet NSButton *right_invert_y;
    IBOutlet NSButton *southpaw;

	IBOutlet NSImageView *a_img;
	IBOutlet NSImageView *b_img;
	IBOutlet NSImageView *x_img;
	IBOutlet NSImageView *y_img;
	IBOutlet NSImageView *menu_img;
	IBOutlet NSImageView *view_img;
	IBOutlet NSImageView *guide_img;
	IBOutlet NSImageView *ls_img;   // Left Stick
	IBOutlet NSImageView *rs_img;   // Right Stick
	IBOutlet NSImageView *lb_img;   // Left Bumper
	IBOutlet NSImageView *rb_img;   // Right Bumper
	IBOutlet NSImageView *up_img;
	IBOutlet NSImageView *down_img;
	IBOutlet NSImageView *left_img;
	IBOutlet NSImageView *right_img;
	IBOutlet NSImageView *lsc_img;
	IBOutlet NSImageView *lt_img;
	IBOutlet NSImageView *rsc_img;
	IBOutlet NSImageView *rt_img;
	// NSImageView* sync_img; // Unused
	IBOutlet NSComboBox *controller_combo;

	ControllerType controllerType;

	NSRect img_pos;

	NSNumber *max_stick;
	NSNumber *min_stick;
	NSNumber *max_trigger;
	NSNumber *min_trigger;

	CFStringRef appID;

	NSMutableArray *joysticks;

	int selected_controller;
}

static const int left_stick_deadzone_default = 7849;
static const int right_stick_deadzone_default = 8689;
static const int trigger_deadzone_default = 120;
static NSString *const invert_x_str = @"Invert X-Axis";
static NSString *const invert_y_str = @"Invert Y-Axis";
static NSString *const southpaw_str = @"Switch Sticks";

#pragma mark - Initialization

- (instancetype)initWithBundle:(NSBundle *)bundle {
	if (self = [super initWithBundle:bundle]) {
		appID = CFSTR("com.vestigl.driver.Xone-Driver");

		max_stick = @32767;     // INT16_MAX
		min_stick = @-32768;    // INT16_MIN
		max_trigger = @1023;
		min_trigger = @0;

// joysticks = [NSMutableArray arrayWithArray:[DDHidJoystick allJoysticks]];
// for (long i = joysticks.count - 1; i >= 0; i--)
// {
// if ([joysticks[i] productId] != 721)
// [joysticks removeObjectAtIndex:i];
// else if ([joysticks[i] vendorId] != 1118)
// [joysticks removeObjectAtIndex:i];
// }
//
// [joysticks makeObjectsPerformSelector:@selector(setDelegate:) withObject:self];
// [joysticks makeObjectsPerformSelector:@selector(startListening) withObject:nil];

		img_pos = NSMakeRect(-139, -122, 901, 771);
	}

	return (self);
} /* initWithBundle */

- (void)mainViewDidLoad {
    [self loadBooleanValue:CFSTR("l_inv_x") forButton:left_invert_x];
    [self loadBooleanValue:CFSTR("l_inv_y") forButton:left_invert_y];
    [self loadBooleanValue:CFSTR("r_inv_x") forButton:right_invert_x];
    [self loadBooleanValue:CFSTR("r_inv_y") forButton:right_invert_y];

    [self loadStringValue:CFSTR("l_stick_d")
             forTextField:left_stick_deadzone
              withDefault:[NSString stringWithFormat:@"%i", left_stick_deadzone_default]];
    [self loadStringValue:CFSTR("r_stick_d")
             forTextField:right_stick_deadzone
              withDefault:[NSString stringWithFormat:@"%i", right_stick_deadzone_default]];
    [self loadStringValue:CFSTR("t_d")
             forTextField:trigger_deadzone
              withDefault:[NSString stringWithFormat:@"%i", trigger_deadzone_default]];

    [self refresh:0];
    for (int i = 0; i < joysticks.count; i++) {
        [controller_combo addItemWithObjectValue:[NSString stringWithFormat:@"Controller %d", i + 1]];
    }

    [controller_combo setNumberOfVisibleItems:joysticks.count];
    if (joysticks.count > 0) {
        [controller_combo selectItemAtIndex:0];
    }
} /* mainViewDidLoad */

- (IBAction)refresh:(id)sender {
	joysticks = [NSMutableArray arrayWithArray:[DDHidJoystick allJoysticks]];
	NSLog(@"JOYSTICK COUNT - %lu", (unsigned long)joysticks.count);
	for (long i = joysticks.count - 1; i >= 0; i--) {
		if ([joysticks[i] vendorId] == 1118) {
			if ([joysticks[i] productId] == 721) {
				controllerType = XONE;
			} else if ([joysticks[i] productId] == 654) {
				controllerType = X360;
			} else {
				[joysticks removeObjectAtIndex:i];
			}
		} else {
			[joysticks removeObjectAtIndex:i];
		}
	}

	[joysticks makeObjectsPerformSelector:@selector(setDelegate:) withObject:self];
	[joysticks makeObjectsPerformSelector:@selector(startListening) withObject:nil];
	for (int i = 0; i < joysticks.count; i++) {
		[controller_combo addItemWithObjectValue:[NSString stringWithFormat:@"Controller %d", i + 1]];
	}

	[controller_combo setNumberOfVisibleItems:joysticks.count];
	if (joysticks.count > 0) {
		[controller_combo selectItemAtIndex:0];
	}
} /* refresh */

#pragma mark - Loading

- (void)loadBooleanValue:(CFStringRef)str forButton:(NSButton *)object {
	CFPropertyListRef value = CFPreferencesCopyValue(str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    
	if (value) {
		if ((CFGetTypeID(value) == CFBooleanGetTypeID())) {
			object.state = CFBooleanGetValue(value);
		}
		CFRelease(value);
	} else {
		object.state = NO;
	}
} /* loadBooleanValue */

- (void)loadStringValue:(CFStringRef)str forTextField:(NSTextField *)object withDefault:(NSString *)def {
	CFPropertyListRef value = CFPreferencesCopyValue(str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

	if (value) {
		if (CFGetTypeID(value) == CFStringGetTypeID()) {
			object.stringValue = [NSString stringWithFormat:@"%@", value];
		}
		CFRelease(value);
	} else {
		object.stringValue = def;
	}
} /* loadStringValue */

#pragma mark - Saving

- (IBAction)checkbox_clicked:(id)sender {
	CFStringRef str;
	NSButton *from = (NSButton *)sender;

	if (from.imagePosition == NSImageLeft) { // Left Stick
		if ([from.title isEqualToString:invert_x_str]) {
			str = CFSTR("l_inv_x");
		} else if ([from.title isEqualToString:invert_y_str]) {
			str = CFSTR("l_inv_y");
        } else if ([from.title isEqualToString:southpaw_str]) {
            str = CFSTR("southpaw");
		} else {
			str = CFSTR("error");
		}
	} else if (from.imagePosition == NSImageRight) {
		if ([from.title isEqualToString:invert_x_str]) {
			str = CFSTR("r_inv_x");
		} else if ([from.title isEqualToString:invert_y_str]) {
			str = CFSTR("r_inv_y");
		} else {
			str = CFSTR("error");
		}
	} else {
		str = CFSTR("error");
	}
	if ([sender state] == YES) {
		CFPreferencesSetValue(str, kCFBooleanTrue, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
	} else {
		CFPreferencesSetValue(str, kCFBooleanFalse, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
	}
    CFPreferencesSynchronize(appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    [self pushSettings];
} /* checkbox_clicked */

- (void)didUnselect {
	// Save for Preference Pane
	CFNotificationCenterRef center;
	NSString *str = left_stick_deadzone.stringValue;
	if ([str isEqual:@""]) {
		str = [NSString stringWithFormat:@"%i", 7849];
	}

	CFPreferencesSetValue(CFSTR("l_stick_d"), (CFStringRef) str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

	str = right_stick_deadzone.stringValue;
	if ([str isEqual:@""]) {
		str = [NSString stringWithFormat:@"%i", 8689];
	}

	CFPreferencesSetValue(CFSTR("r_stick_d"), (CFStringRef) str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

	str = trigger_deadzone.stringValue;
	if ([str isEqual:@""]) {
		str = [NSString stringWithFormat:@"%i", 120];
	}

	CFPreferencesSetValue(CFSTR("t_d"), (CFStringRef) str, appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

	CFPreferencesSynchronize(appID, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

	center = CFNotificationCenterGetDistributedCenter();
	CFNotificationCenterPostNotification(center, CFSTR("Preferences Changed"), appID, NULL, TRUE);

	// Save for driver
    [self pushSettings];
} /* didUnselect */

- (void)pushSettings {
    int lstick = [left_stick_deadzone intValue];
    int rstick = [right_stick_deadzone intValue];
    int trigger = [trigger_deadzone intValue];
    CFMutableDictionaryRef dictRef = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFBooleanRef linvx = left_invert_x.state ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(dictRef, CFSTR("l_inv_x"), linvx);
    CFRelease(linvx);
    
    CFBooleanRef linvy = left_invert_y.state ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(dictRef, CFSTR("l_inv_y"), linvy);
    CFRelease(linvy);
    
    CFBooleanRef rinvx = right_invert_x.state ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(dictRef, CFSTR("r_inv_x"), rinvx);
    CFRelease(rinvx);
    
    CFBooleanRef rinvy = right_invert_y.state ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(dictRef, CFSTR("r_inv_y"), rinvy);
    CFRelease(rinvy);
    
    CFBooleanRef south = southpaw.state ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(dictRef, CFSTR("southpaw"), south);
    CFRelease(south);
    
    CFNumberRef numberRefL = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &lstick);
    CFDictionarySetValue(dictRef, CFSTR("l_stick_d"), numberRefL);
    CFRelease(numberRefL);
    
    CFNumberRef numberRefR = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &rstick);
    CFDictionarySetValue(dictRef, CFSTR("r_stick_d"), numberRefR);
    CFRelease(numberRefR);
    
    CFNumberRef numberRefT = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &trigger);
    CFDictionarySetValue(dictRef, CFSTR("t_d"), numberRefT);
    CFRelease(numberRefT);
    
    if (controller_combo.indexOfSelectedItem > -1) {
        IORegistryEntrySetCFProperties([joysticks[controller_combo.indexOfSelectedItem] ioDevice], dictRef);
    }
    
    // TODO(Drew): See if previous code can be simplified to commented code.
    //    NSDictionary * dict = @{
    //                            @"l_inv_x": @(left_invert_x.state == NSOnState),
    //                            @"l_inv_y": @(left_invert_y.state == NSOnState),
    //                            @"r_inv_x": @(right_invert_x.state == NSOnState),
    //                            @"r_inv_y": @(right_invert_y.state == NSOnState),
    //                            @"l_stick_d": @([left_stick_deadzone intValue]),
    //                            @"r_stick_d": @([right_stick_deadzone intValue]),
    //                            @"t_d": @([trigger_deadzone intValue]),
    //                            };
    //	if (controller_combo.indexOfSelectedItem > -1) {
    //		IORegistryEntrySetCFProperties([joysticks[controller_combo.indexOfSelectedItem] ioDevice], (__bridge CFDictionaryRef) dict);
    //	}
} /* pushSettings */

#pragma mark - Reset

- (IBAction)reset_clicked:(id)sender;
{
    left_stick_deadzone.intValue = left_stick_deadzone_default;
    right_stick_deadzone.intValue = right_stick_deadzone_default;
    trigger_deadzone.intValue = trigger_deadzone_default;

    left_invert_x.state = 0;
	left_invert_y.state = 0;
	right_invert_x.state = 0;
	right_invert_y.state = 0;
}

#pragma mark - Joystick Delegate

- (void)ddhidJoystick:(DDHidJoystick *)joystick buttonDown:(unsigned)buttonNumber {
	if ([joystick locationId] == [joysticks[controller_combo.indexOfSelectedItem] locationId]) {
        // NSLog(@"Button Number - %d", buttonNumber);
		switch (buttonNumber) {
			case 0: {
				a_img.hidden = NO;
				break;
			}

			case 1: {
				b_img.hidden = NO;
				break;
			}

			case 2: {
				x_img.hidden = NO;
				break;
			}

			case 3: {
				y_img.hidden = NO;
				break;
			}

			case 4: {
				lb_img.hidden = NO;
				break;
			}

			case 5: {
				rb_img.hidden = NO;
				break;
			}

			case 6: {
				if (controllerType == X360) {
					lsc_img.hidden = NO;
				} else if (controllerType == XONE) {
					view_img.hidden = NO;
				}
				break;
			}

			case 7: {
				if (controllerType == X360) {
					rsc_img.hidden = NO;
				} else if (controllerType == XONE) {
					menu_img.hidden = NO;
                }
                break;
			}

			case 8: {
				if (controllerType == X360) {
					menu_img.hidden = NO;
				} else if (controllerType == XONE) {
					lsc_img.hidden = NO;
                }
                break;
			}

			case 9: {
				if (controllerType == X360) {
					view_img.hidden = NO;
				} else if (controllerType == XONE) {
					rsc_img.hidden = NO;
                }
                break;  // should this fall thru to buttonNumber 10 also?
			}

			case 10: {
				// 360 Location of Guide Button
				guide_img.hidden = NO;
				break;
			}

			case 11: {
				if (controllerType == X360) {
					up_img.hidden = NO;
				}

				// Sync on XONE. No image.
				break;
			}

			case 12: {
				if (controllerType == X360) {
					down_img.hidden = NO;
				} else if (controllerType == XONE) {
					up_img.hidden = NO;
                }
                break;
			}

			case 13: {
				if (controllerType == X360) {
					left_img.hidden = NO;
				} else if (controllerType == XONE) {
					down_img.hidden = NO;
                }
                break;
			}

			case 14: {
				if (controllerType == X360) {
					right_img.hidden = NO;
				} else if (controllerType == XONE) {
					left_img.hidden = NO;
                }
                break;
			}

			case 15: {
				if (controllerType == X360) {
					guide_img.hidden = NO;
				} else if (controllerType == XONE) {
					right_img.hidden = NO;
                }
                break;
			}

			default: {
				break;
			}
		} /* switch */
	}
} /* ddhidJoystick */

- (void)ddhidJoystick:(DDHidJoystick *)joystick buttonUp:(unsigned)buttonNumber {
	if ([joystick locationId] == [joysticks[controller_combo.indexOfSelectedItem] locationId]) {
		switch (buttonNumber) {
			case 0: {
				a_img.hidden = YES;
				break;
			}

			case 1: {
				b_img.hidden = YES;
				break;
			}

			case 2: {
				x_img.hidden = YES;
				break;
			}

			case 3: {
				y_img.hidden = YES;
				break;
			}

			case 4: {
				lb_img.hidden = YES;
				break;
			}

			case 5: {
				rb_img.hidden = YES;
				break;
			}

			case 6: {
				if (controllerType == X360) {
					lsc_img.hidden = YES;
				} else if (controllerType == XONE) {
					view_img.hidden = YES;
                }
                break;
			}

			case 7: {
				if (controllerType == X360) {
					rsc_img.hidden = YES;
				} else if (controllerType == XONE) {
					menu_img.hidden = YES;
                }
                break;
			}

			case 8: {
				if (controllerType == X360) {
					menu_img.hidden = YES;
				} else if (controllerType == XONE) {
					lsc_img.hidden = YES;
                }
                break;
			}

			case 9: {
				if (controllerType == X360) {
					view_img.hidden = YES;
				} else if (controllerType == XONE) {
					rsc_img.hidden = YES;
				}
                // break; // fall thru to buttonNumber 10
			}

			case 10: {
				// 360 Location of Guide Button
				guide_img.hidden = YES;
				break;
			}

			case 11: {
				if (controllerType == X360) {
					up_img.hidden = YES;
				}

				// Sync on XONE. No image.
				break;
			}

			case 12: {
				if (controllerType == X360) {
					down_img.hidden = YES;
				} else if (controllerType == XONE) {
					up_img.hidden = YES;
                }
                break;
			}

			case 13: {
				if (controllerType == X360) {
					left_img.hidden = YES;
				} else if (controllerType == XONE) {
					down_img.hidden = YES;
                }
                break;
			}

			case 14: {
				if (controllerType == X360) {
					right_img.hidden = YES;
				} else if (controllerType == XONE) {
					left_img.hidden = YES;
                }
                break;
			}

			case 15: {
				if (controllerType == X360) {
					guide_img.hidden = YES;
				} else if (controllerType == XONE) {
					right_img.hidden = YES;
                }
                break;
			}

			default: {
				break;
			}
		} /* switch */
	}
} /* ddhidJoystick */

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick xChanged:(int)value {
	if ([joystick locationId] == [joysticks[controller_combo.indexOfSelectedItem] locationId]) {
        NSRect frame = NSMakeRect(img_pos.origin.x, ls_img.frame.origin.y, img_pos.size.width, img_pos.size.height);
		if (abs(value) > left_stick_deadzone.intValue) {
			// Left Stick Light
			double offset = 11 * value / max_stick.intValue;
            frame.origin.x += offset;//left_invert_x.state ? -offset : offset;
		}
        ls_img.frame = frame;
	}
} /* ddhidJoystick */

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick yChanged:(int)value {
	if ([joystick locationId] == [joysticks[controller_combo.indexOfSelectedItem] locationId]) {
        NSRect frame = NSMakeRect(ls_img.frame.origin.x, img_pos.origin.y, img_pos.size.width, img_pos.size.height);
		if (abs(value) > left_stick_deadzone.intValue) {
			// Left Stick Light
			double offset = 11 * value / max_stick.intValue;
            frame.origin.y += -offset;//left_invert_y ? offset : -offset;
		}
        ls_img.frame = frame;
	}
} /* ddhidJoystick */

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick otherAxis:(unsigned)otherAxis valueChanged:(int)value {
	if ([joystick locationId] == [joysticks[controller_combo.indexOfSelectedItem] locationId]) {
		// Xbox One
		if (stick == 0) {
			// Left Trigger
			if (otherAxis == 0) {
				[self leftTrigger:value];
			}
			if (otherAxis == 1) {
				[self rightTrigger:value];
			}
			if (otherAxis == 2) {
				[self rightStickX:value];
			}
			if (otherAxis == 3) {
				[self rightStickY:value];
			}
		}
		// 360 - Right Stick
		if (stick == 1) {
			if (otherAxis == 0) {
				[self rightStickX:value];
			} else if (otherAxis == 1) {
				[self rightStickY:value];
			}
		}
		// 360 - Triggers
		if (stick == 2) {
			if (otherAxis == 0) {
				[self leftTrigger:value];
			}
			if (otherAxis == 1) {
				[self rightTrigger:value];
			}
		}
	}
} /* ddhidJoystick */

- (void)rightStickX:(int)value {
    NSRect frame = NSMakeRect(img_pos.origin.x, rs_img.frame.origin.y, img_pos.size.width, img_pos.size.height);
	if (abs(value) > right_stick_deadzone.intValue) {
		double offset = 11 * value / max_stick.intValue;
        frame.origin.x += offset;//right_invert_x.state ? -offset : offset;
	}
    rs_img.frame = frame;
} /* rightStickX */

- (void)rightStickY:(int)value {
    NSRect frame = NSMakeRect(rs_img.frame.origin.x, img_pos.origin.y, img_pos.size.width, img_pos.size.height);
	if (abs(value) > right_stick_deadzone.intValue) {
		double offset = 11 * value / max_stick.intValue;
        frame.origin.y += -offset;//right_invert_y.state ? offset : -offset;
	}
    rs_img.frame = frame;
} /* rightStickY */

- (void)leftTrigger:(int)value {
    lt_img.hidden = (value <= trigger_deadzone.intValue);
} /* leftTrigger */

- (void)rightTrigger:(int)value {
    rt_img.hidden = (value <= trigger_deadzone.intValue);
} /* rightTrigger */

- (void)ddhidJoystick:(DDHidJoystick *)joystick stick:(unsigned)stick povNumber:(unsigned)povNumber valueChanged:(int)value {
	// Hat reported as buttons
}

@end

