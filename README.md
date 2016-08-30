# WinRTMidi DLL #

This GitHub project wraps the Windows Runtime [Windows::Devices::Midi](https://msdn.microsoft.com/library/windows/apps/windows.devices.midi.aspx) API 
in a Win32 DLL that can be dynamically loaded by a standard Win32 application. If the application is running on a device with Windows 10, the DLL will 
load and you will be able to use the Windows::Devices::Midi API via a C interface exported by the DLL. The Windows::Devices::Midi API is ony available on devices running Windows 10.
If you attempt to load the WinRTMidi DLL when your app is running on Windows 7 or 8/8.1, the DLL will fail to load and you will know that the Windows::Devices::Midi API is not available to 
your application.

The WinRTMid DLL enables the following MIDI functionality from the Windows::Devices::Midi API:

* Enumerate MIDI ports.
* Notification when MIDI ports are added or removed.
* Create a MIDI in or out port.
* Send MIDI messages on a MIDI out port.
* Receive MIDI messages from a MIDI in port.
* Destroy a MIDI port.


---
# Requirements to build the WinRTMidi DLL #

Visual Studio 2015 (Update 3 recommended) with **Universal Windows App Development Tools and Windows 10 Tools and SDKs** [installed](https://msdn.microsoft.com/en-us/library/e2h7fzkw.aspx)

# Adding the WinRTMidi DLL to your Win32 Project #



### Updating your Win32 Project Properties ###







	

