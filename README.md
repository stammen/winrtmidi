# WinRTMidi DLL #

This GitHub WinRTMidi project wraps the Windows Runtime [Windows::Devices::Midi](https://msdn.microsoft.com/library/windows/apps/windows.devices.midi.aspx) API 
in a Win32 DLL that can be dynamically loaded by a standard Win32 application. If the application is running on a device with Windows 10, the DLL will 
load and you will be able to use the Windows::Devices::Midi API via a C interface exported by the DLL.
If your application is running on devices with Windows 7 or Windows 8/8.1, the WinRTMidi DLL will not load. Your application will need to test if it is running on Windows 10 before
attempting to load the DLL (See [Testing for Windows 10](#testing-for-windows-10) below).  The Windows::Devices::Midi API is ony available on devices running Windows 10.

This DLL is useful for the scenario where you have an existing Win32 MIDI application and want to use the new Windows::Devices::Midi API. If you are not able to update your application to a 
Windows 10 UWP app, you can use the WinRTMidi DLL to access the Windows::Devices::Midi API when your application  is running on a device with Windows 10. 

The recommended steps to use this DLL are as follows:

1. Add the WinRTMidi DLL to your Win32 application but do not link to the DLL.
1. When your Win32 application runs, check if your application is running on Windows 10 (See Testing for Windows 10 below).
1. If your application is running on Windows 10, dynamically load the WinRTMidi DLL using **LoadLibrary()**.
	* If your application is not running on Windows 10, continue to use the WinMM MIDI API.
1. Get pointers to the various WinRTMIDI functions using **GetProcAddress()**.
1. Initialize the WinRTMIDI API using the **winrt_initialize_midi()** function.
1. Obtain a MIDI port using **winrt_open_midi_in_port()** or **winrt_open_midi_out_port()** functions.
1. For more information see example code below.


The WinRTMid DLL enables the following MIDI functionality from the Windows::Devices::Midi API:

* Enumerate MIDI ports.
* Notification when MIDI ports are added or removed.
* Create a MIDI in or out port.
* Send MIDI messages on a MIDI out port.
* Receive MIDI messages from a MIDI in port.
* Destroy a MIDI port.
* Access Bluetooth MIDI ports
* Multi-client MIDI port support

---
# Requirements to build the WinRTMidi DLL #

Visual Studio 2015 (Update 3 recommended) with **Universal Windows App Development Tools and Windows 10 Tools and SDKs** [installed](https://msdn.microsoft.com/en-us/library/e2h7fzkw.aspx)

### Adding the WinRTMidi DLL to your Win32 Project ###

### Testing for Windows 10 <a id="testing-for-windows-10"/>###

Starting with Windows 8.1, the following Win32 version checking functions will return return version 6.2.0.0 for Windows 8.1 and above:
 
* [GetVersion()](https://msdn.microsoft.com/en-us/library/windows/desktop/ms724439(v=vs.85).aspx)
* [GetVersionEx()](https://msdn.microsoft.com/en-us/library/windows/desktop/ms724451(v=vs.85).aspx)
* [VerifyVersionInfo()](https://msdn.microsoft.com/en-us/library/windows/desktop/ms725492(v=vs.85).aspx)

GetVersion() and GetVersionEx() have also been deprecated in Windows 10. In order to correctly obtain the Windows OS version your application is running on, you can do one of the following strategies:

1. Add a manifest to your application and use VerifyVersionInfo() to test for Windows 10.
1. Get the version of kernel32.dll. This method does not require adding an app manifest to ensure correct Windows 8.1/10 version numbers.

If you do not check for Windows 10 and attempt to load the WinRT DLL, your application will quit with the following error:

![WinRT DLL Load Error](Images/dllloaderror.png "WinRT DLL Load Error")









	

