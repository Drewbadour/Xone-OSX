Xone-OSX
========

### Current Release: Beta 0.3.1 (Dec. 23, 2014)

### This is a beta release. Use at your own discretion.

**NOTE: If you want to use the driver without access to the source code, download the latest version [here](https://mega.co.nz/#!CspWVSIC!5lngix_yrFEcnKWnLk_Kl0ctm72O6-CvmoRdrEVay8I).**

## Installation

#### Install on OS X 10.9+
 - Run the installer located above.
 - Reboot
 - Plug the controller in, without powering it on, and it should light up.
 - Enjoy!

I don't recommend leaving batteries in the controller while using it wired for extended periods of time. This could potentially cause wear on the batteries that is best avoided.

This is a kext and preference pane that allows users to use the Xbox One controller with their OS X computer. Because the Xbox One controller does not identify as a Human Interface Device, and requires custom startup code, a custom driver had to be made.

## Features
 - Performs custom startup and handles all buttons on the controller
 - Customizable deadzones for both sticks and triggers
 - Invert X and Y for each stick individually
 - Preference pane allows you to see your controller in action
 - New awesome graphics, thanks to k1down
 - Signed

## Support List:
 - DDHidLib (Ex. OpenEmu)
 - Steamworks Controller API (Steam Big Picture, and other Steam games that implement the API)

## Future Features
 - Force Feedback (Rumble)
 - Recognize controller as GCController
 - Change controller input to keyboard input (setting) so as to support unsupported games
 - Compatibility with games not using Steamworks API
 - Wireless implementation is probably unlikely, since Microsoft uses a proprietary wireless implementation for the controller

## Developer Notes

#### Yosemite Kext Signatures
OS X 10.10 Yosemite introduced a new security feature requiring _all_ kernel extensions to be signed with a _special_ certificate specifically for signing kernel extensions. For those hoping to build and run the kext from source, have a couple of options.

1. A kext developer mode can be enabled to allow invalid signatures. _This is a global setting and will disable kext signature checking for the entire system!_ To enable the mode, run the following and reboot:
```sh
$ sudo nvram boot-args=kext-dev-mode=1
```
To revert to the system default setting of disallowing invalid kext signatures, run the following and reboot:
```sh
$ sudo nvram -d boot-args
```

2. Those enrolled in the [Mac Developer Program](https://developer.apple.com/programs/mac/) can request the certificate on the Apple Developer page [Requesting a Developer ID Certificate for Signing Kexts](https://developer.apple.com/contact/kext/) - though it is worth noting that the certificate is intended for those needing to sign a kext that is commercially shipped or broadly distributed through a large organization.

#### Load State
Kernel extensions are loaded on demand and if the controller does not light up when attached one could check if to ensure it is loaded. The kext's status should be displayed if running via:
```sh
$ kextstat | grep Xone
```

Though generally unnecessary, the kext can be manually loaded via:
```sh
$ sudo kextutil -t /Library/Extensions/Xone\ Driver.kext
```

## Discussion
 - I've done my best to match the controls of the Xbox One controller to the Xbox 360 controller.
 - There doesn't appear to be any documentation as to how to implement a KEXT so that the controller is recognized as a GCController, so currently it is not.
 - I couldn't quite grasp how to convert the D-pad Data (4 consecutive bits with 0 for unpressed and 1 for currently pressed) into a proper HID implementation. So the D-Pad is currently bound to buttons 13 - 16.
 - I needed a game that supported rumble to implement rumble, and Bastion appears to not like my implementation of the controller. (Ex. The left trigger appears to take the place of every face button, and also appears to act as the left stick in game)
 - The guide button is handled in a separate packet, but I managed to have it recognized as button 11.
 - The sync button is also visible to the user and is encoded as button 12.

## Button Layout
| HID Button Number | Controller Button Name |
|:-----------------:|:----------------------:|
| 1                 | A                      |
| 2                 | B                      |
| 3                 | X                      |
| 4                 | Y                      |
| 5                 | Left Bumper            |
| 6                 | Right Bumper           |
| 7                 | View (Back)            |
| 8                 | Menu (Start)           |
| 9                 | Left Stick Click       |
| 10                | Right Stick Click      |
| 11                | Guide (Xbox)           |
| 12                | Sync                   |
| 13                | D-pad Up               |
| 14                | D-pad Down             |
| 15                | D-pad Left             |
| 16                | D-pad Right            |

| Axis ID           | Axis on Controller     |
|:-----------------:|:----------------------:|
| X                 | Left Stick X           |
| Y                 | Left Stick Y           |
| Z                 | Right Stick X          |
| Rx                | Right Stick Y          |
| Ry                | Left Trigger           |
| Rz                | Right Trigger          |
