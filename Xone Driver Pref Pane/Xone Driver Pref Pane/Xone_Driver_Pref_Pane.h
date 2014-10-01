//
//  Xone_Driver_Pref_Pane.h
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



#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>
#import <QuartzCore/QuartzCore.h>
#import <DDHidLib/DDHidLib.h>

enum class
{
    xo_l_inv_x,
    xo_l_inv_y,
    xo_r_inv_x,
    xo_r_inv_y,
    xo_l_stick_d,
    xo_r_stick_d,
    xo_t_d,
};

@interface Xone_Driver_Pref_Pane : NSPreferencePane
{
    IBOutlet NSTabViewItem* general_tab;
    IBOutlet NSTextField* left_stick_deadzone;
    IBOutlet NSTextField* right_stick_deadzone;
    IBOutlet NSTextField* trigger_deadzone;
    IBOutlet NSButton* left_invert_x;
    IBOutlet NSButton* left_invert_y;
    IBOutlet NSButton* right_invert_x;
    IBOutlet NSButton* right_invert_y;
    
    IBOutlet NSImageView* a_img;
    IBOutlet NSImageView* b_img;
    IBOutlet NSImageView* x_img;
    IBOutlet NSImageView* y_img;
    IBOutlet NSImageView* menu_img;
    IBOutlet NSImageView* view_img;
    NSImageView* guide_img; // Unused
    IBOutlet NSImageView* ls_img; // Left Stick
    IBOutlet NSImageView* rs_img; // Right Stick
    IBOutlet NSImageView* lb_img; // Left Bumper
    IBOutlet NSImageView* rb_img; // Right Bumper
    IBOutlet NSImageView* up_img;
    IBOutlet NSImageView* down_img;
    IBOutlet NSImageView* left_img;
    IBOutlet NSImageView* right_img;
    NSImageView* sync_img; // Unused
    NSImageView* lt_img; // Left Trigger Unused
    NSImageView* rt_img; // Right Trigger Unused
    IBOutlet NSComboBox* controller_combo;
}

- (void)mainViewDidLoad;
- (IBAction)reset_clicked:(id)sender;
- (IBAction)checkbox_clicked:(id)sender;

@end
