Xone-OSX
========

### Current Release: 1.0.0 (Feb. 28, 2015)

**NOTE: If you want to use the driver without access to the source code, [check the releases page](https://github.com/FranticRain/Xone-OSX/releases/latest).**

## Installation

#### Install on OS X 10.9+
 - Run the installer located on the releases page.
 - Reboot
 - Plug the controller in, without powering it on, and it should light up.
 - Enjoy!

I don't recommend leaving batteries in the controller while using it wired for extended periods of time. This could potentially cause wear on the batteries that is best avoided.

This is a kext and preference pane that allows users to use the Xbox One controller with their OS X computer. Because the Xbox One controller does not identify as a Human Interface Device, and requires custom startup code, a custom driver had to be made.

## Uninstall

There is an uninstaller included in the .dmg. Simply run it, and it will take care of the rest!

## Features
 - Performs custom startup and handles all buttons on the controller
 - Customizable deadzones for both sticks and triggers
 - Invert X and Y for each stick individually
 - Preference pane allows you to see your controller in action
 - New awesome graphics, thanks to k1down
 - Signed

## Support List:
 - Most games that support a 360 controller. Create an issue if a game works with the 360, but not the Xbox One controller.
 - DDHidLib (Ex. OpenEmu)
 - Steamworks Controller API (Steam Big Picture, and other Steam games that implement the API)

## Future Features
 - Force Feedback (Rumble)
 - Recognize controller as GCController
 - Change controller input to keyboard input (setting) so as to support unsupported games
 - Compatibility with games not using Steamworks API
 - Wireless will work with the future release of the controller dongle.

## Developer Notes

#### Yosemite Kext Signatures
OS X Yosemite introduced a new security feature requiring _all_ kernel extensions to be signed by a certificate with a _new_ custom extension (OID `1.2.840.113635.100.6.1.18`) that designates it as a kext-signing certificate. On reboot, unsigned kexts or those with invalid signatures will be left unloaded and the system will alert the user via a warning.
<img src="http://i.imgur.com/huqvz6Y.png" width="532" height="287" alt="Yosemite kext signature alert">

Those hoping to build and run the kext from source, have a couple of options.

##### Disable Signature Checking
A kext developer mode can be enabled to allow invalid signatures. _This is a global setting and will disable kext signature checking for the entire system!_ To enable the mode, run the following and reboot:
```sh
$ sudo nvram boot-args=kext-dev-mode=1
```
To revert to the system default setting of disallowing invalid kext signatures, run the following and reboot:
```sh
$ sudo nvram -d boot-args
```

##### Request A Kext-Signing Cert
If enrolled in the [Mac Developer Program](https://developer.apple.com/programs/mac/), one can request the special certificate on the Apple Developer page [Requesting a Developer ID Certificate for Signing Kexts](https://developer.apple.com/contact/kext/) - it is worth noting that the certificate is intended for those needing to sign a kext that is commercially shipped or broadly distributed through a large organization.

#### Kext Loading
Kernel extensions are loaded on demand and if the controller does not light up when attached, one could check if the kext is loaded - the kext's status should be displayed if loaded via:
```sh
$ kextstat | grep Xone
```

Though generally unnecessary, the kext can be manually loaded as well:
```sh
$ sudo kextutil -t /Library/Extensions/Xone\ Driver.kext
```

## Discussion
 - There doesn't appear to be any documentation as to how to implement a KEXT so that the controller is recognized as a GCController, so currently it is not.
 - I couldn't quite grasp how to convert the D-pad Data (4 consecutive bits with 0 for unpressed and 1 for currently pressed) into a proper HID implementation.
 - I'm looking into updates to the TattieBogle driver that make rumble implementation clearer.
 - The xbox button is handled in a separate packet, which means that it has to be different from the Xbox 360 implementation.

## Button Layout
| HID Button Number | Controller Button Name |
|:-----------------:|:----------------------:|
| 1                 | A                      |
| 2                 | B                      |
| 3                 | X                      |
| 4                 | Y                      |
| 5                 | Left Bumper            |
| 6                 | Right Bumper           |
| 7                 | Left Stick Click       |
| 8                 | Right Stick Click      |
| 9                 | Menu (Start)           |
| 10                | View (Back)            |
| 11                | Guide (Xbox) on 360    |
| 12                | D-pad Up               |
| 13                | D-pad Down             |
| 14                | D-pad Left             |
| 15                | D-pad Right            |
| 16                | Xbox (Guide) on Xone   |

| Axis ID           | Axis on Controller     |
|:-----------------:|:----------------------:|
| X                 | Left Stick X           |
| Y                 | Left Stick Y           |
| Z                 | Left Trigger           |
| Rx                | Right Stick X          |
| Ry                | Right Stick Y          |
| Rz                | Right Trigger          |

## Button Layout to use with Unity


When using the Xone-Driver-X.X.X.dmg in your Unity Engine project, the Layout is different.
However, when using Xone-Driver-360-X.X.X.dmg, layout is identical to a 360 controller.

| HID Button Number | Controller Button Name |
|:-----------------:|:----------------------:|
| 1                 | Menu (Start)           |
| 2                 | View (Back)            |
| 3                 | A                      |
| 4                 | B                      |
| 5                 | X                      |
| 6                 | Y                      |
| 7                 | D-Pad Up               |
| 8                 | D-Pad Down             |
| 9                 | D-Pad Left             |
| 10                | D-Pad Right            |
| 11                | Left Bumper            |
| 12                | Right Bumper           |
| 13                | Left Stick             |
| 14                | Right Stick            |

| Axis ID           | Axis on Controller     |
|:-----------------:|:----------------------:|
| X Axis            | Left Trigger           |
| Y Axis            | Right Trigger          |
| 3rd Axis          | Left Stick X           |
| 4th Axis          | Left Stick Y           |
| 5th Axis          | Right Stick X          |
| 6th Axis          | Right Stick Y          |
