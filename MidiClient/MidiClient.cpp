// ******************************************************************
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THE CODE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
// THE CODE OR THE USE OR OTHER DEALINGS IN THE CODE.
// ******************************************************************

#include "stdafx.h"
#include "WinRTMidi.h"
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>

// needed to use GetFileVersionInfo functions 
#pragma comment(lib, "version.lib")

using namespace std;
using namespace WinRT;

CRITICAL_SECTION gCriticalSection;

// WinRTMidi DLL function pointers
WinRTWatcherPortCountFunc   gWatcherPortCountFunc = nullptr;
WinRTWatcherPortNameFunc    gWatcherPortNameFunc = nullptr;
WinRTWatcherPortTypeFunc    gWatcherPortTypeFunc = nullptr;
WinRTMidiInitializeFunc     gMidiInitFunc = nullptr;
WinRTMidiFreeFunc           gMidiFreeFunc = nullptr;
WinRTMidiGetPortWatcherFunc gMidiGetPortWatcher = nullptr;
WinRTMidiInPortOpenFunc     gMidiInPortOpenFunc = nullptr;
WinRTMidiInPortFreeFunc     gMidiInPortFreeFunc = nullptr;
WinRTMidiOutPortOpenFunc    gMidiOutPortOpenFunc = nullptr;
WinRTMidiOutPortFreeFunc    gMidiOutPortFreeFunc = nullptr;
WinRTMidiOutPortSendFunc    gMidiOutPortSendFunc = nullptr;

// Midi Out port
WinRTMidiOutPortPtr gMidiOutPort = nullptr;

/*
    Windows 8.1 and higher make it difficult to access Windows OS version.
    This function checks OS version by getting version of kernel32.dll.
*/
bool windows10orGreater()
{
    static const wchar_t kernel32[] = L"\\kernel32.dll";
    wchar_t path[MAX_PATH];

    unsigned int n = GetSystemDirectory(path, MAX_PATH);
    memcpy_s(path + n, MAX_PATH, kernel32, sizeof(kernel32));

    unsigned int size = GetFileVersionInfoSize(path, NULL);
    if (size == 0)
    {
        return false;
    }

    std::vector<char> verionInfo;
    verionInfo.resize(size);
    BOOL result = GetFileVersionInfo(path, 0, size, verionInfo.data());
    if (!result || GetLastError() != S_OK)
    {
        return false;
    }

    VS_FIXEDFILEINFO *vinfo;
    result = VerQueryValue(verionInfo.data(), L"\\", (LPVOID *)&vinfo, &size);
    if (!result || size < sizeof(VS_FIXEDFILEINFO))
    {
        return false;
    }

    cout << "Windows version: "
        << (int)HIWORD(vinfo->dwProductVersionMS) << "."
        << (int)LOWORD(vinfo->dwProductVersionMS) << "."
        << (int)HIWORD(vinfo->dwProductVersionLS) << endl << endl;

    return HIWORD(vinfo->dwProductVersionMS) >= 10;
}

void printPortNames(const WinRTMidiPortWatcherPtr watcher)
{
    if (watcher == nullptr)
    {
        return;
    }

    WinRTMidiPortType type = gWatcherPortTypeFunc(watcher);
    if (type == In)
    {
        cout << "MIDI In Ports" << endl;
    }
    else
    {
        cout << "MIDI Out Ports" << endl;
    }

    int nPorts = gWatcherPortCountFunc(watcher);
    for (int i = 0; i < nPorts; i++)
    {
        const char* name = gWatcherPortNameFunc(watcher, i);
        cout << i << ": " << gWatcherPortNameFunc(watcher, i) << endl;
    }

    cout << endl;
}

void midiPortChangedCallback(const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update)
{
    EnterCriticalSection(&gCriticalSection);
    string portName = gWatcherPortTypeFunc(portWatcher) == In ? "In" : "Out";

    switch (update)
    {
    case PortAdded:
        cout << "***MIDI " << portName << " port added***" << endl;
        break;

    case PortRemoved:
        cout << "***MIDI " << portName << " port removed***" << endl;
        break;

    case EnumerationComplete:
        cout << "***MIDI " << portName << " port enumeration complete***" << endl;
        break;
    }

    printPortNames(portWatcher);
    LeaveCriticalSection(&gCriticalSection);
}

void midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes)
{
    if (gMidiOutPort != nullptr)
    {
        gMidiOutPortSendFunc(gMidiOutPort, message, nBytes);
    }

    EnterCriticalSection(&gCriticalSection);
    for (unsigned int i = 0; i < nBytes; i++)
    {
        cout << "Byte " << i << " = " << (int)message[i] << ", ";
    }

    if (nBytes > 0)
    {
        cout << "timestamp = " << timeStamp << endl;
    }
    LeaveCriticalSection(&gCriticalSection);
}

int main()
{
    HINSTANCE dllHandle = NULL;
    WinRTMidiPtr midiPtr = nullptr;
    WinRTMidiInPortPtr midiInPort = nullptr;

    //Load the WinRTMidi dll

    //Load the WinRTMidi dll
    if (windows10orGreater())
    {
        dllHandle = LoadLibrary(L"WinRTMidi.dll");
    }

    if (NULL == dllHandle)
    {
        cout << "Unable to load WinRTMidi.dll" << endl;
        goto cleanup;
    }
  
    InitializeCriticalSection(&gCriticalSection);

    // GetWinRTMidi DLL function pointers. Error checking needs to be added!
    //Get pointer to the WinRTMidiInitializeFunc function using GetProcAddress:  
    gMidiInitFunc = reinterpret_cast<WinRTMidiInitializeFunc>(::GetProcAddress(dllHandle, "winrt_initialize_midi"));

    //Get pointer to the WinRTMidiFreeFunc function using GetProcAddress:  
    gMidiFreeFunc = reinterpret_cast<WinRTMidiFreeFunc>(::GetProcAddress(dllHandle, "winrt_free_midi"));

    //Get pointer to the WinRTMidiGetPortWatcherFunc function using GetProcAddress:  
    gMidiGetPortWatcher = reinterpret_cast<WinRTMidiGetPortWatcherFunc>(::GetProcAddress(dllHandle, "winrt_get_portwatcher"));

    //Get pointer to the WinRTMidiInPortOpenFunc function using GetProcAddress:
    gMidiInPortOpenFunc = reinterpret_cast<WinRTMidiInPortOpenFunc>(::GetProcAddress(dllHandle, "winrt_open_midi_in_port"));

    //Get pointer to the WinRTMidiInPortFreeFunc function using GetProcAddress:
    gMidiInPortFreeFunc = reinterpret_cast<WinRTMidiInPortFreeFunc>(::GetProcAddress(dllHandle, "winrt_free_midi_in_port"));

    //Get pointer to the WinRTMidiOutPortOpenFunc function using GetProcAddress:
    gMidiOutPortOpenFunc = reinterpret_cast<WinRTMidiOutPortOpenFunc>(::GetProcAddress(dllHandle, "winrt_open_midi_out_port"));

    //Get pointer to the WinRTMidiOutPortFreeFunc function using GetProcAddress:
    gMidiOutPortFreeFunc = reinterpret_cast<WinRTMidiOutPortFreeFunc>(::GetProcAddress(dllHandle, "winrt_free_midi_out_port"));

    //Get pointer to the WinRTMidiOutPortSendFunc function using GetProcAddress:
    gMidiOutPortSendFunc = reinterpret_cast<WinRTMidiOutPortSendFunc>(::GetProcAddress(dllHandle, "winrt_midi_out_port_send"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    gWatcherPortCountFunc = reinterpret_cast<WinRTWatcherPortCountFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_count"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    gWatcherPortNameFunc = reinterpret_cast<WinRTWatcherPortNameFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_name"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    gWatcherPortTypeFunc = reinterpret_cast<WinRTWatcherPortTypeFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_type"));

    // initialize Midi interface
    WinRTMidiErrorType result = gMidiInitFunc(midiPortChangedCallback, &midiPtr);
    if(result != WINRT_NO_ERROR)
    {
        cout << "Unable to initialize WinRTMidi" << endl;
        goto cleanup;
    }

    // open midi in port 0
    result = gMidiInPortOpenFunc(midiPtr, 0, midiInCallback, &midiInPort);
    if (result != WINRT_NO_ERROR)
    {
        cout << "Unable to create Midi In port" << endl;
        goto cleanup;
    }

    // open midi out port 0
    result = gMidiOutPortOpenFunc(midiPtr, 0, &gMidiOutPort);
    if (result != WINRT_NO_ERROR)
    {
        cout << "Unable to create Midi Out port" << endl;
        goto cleanup;
    }

    // example on how get midi port info
    const WinRTMidiPortWatcherPtr watcher = gMidiGetPortWatcher(midiPtr, In);
    if (watcher != nullptr)
    {
        unsigned int numPorts = gWatcherPortCountFunc(watcher);
        if (numPorts > 0)
        {
            const char* name = gWatcherPortNameFunc(watcher, 0);
        }
    }

cleanup:

    // process midi until user presses key on keyboard
    cout << "Press any key to exit..." << endl;
    char c = _getch();


    // clean up midi objects
    // okay to call with nullptrs
    if(gMidiOutPortFreeFunc)
    {
        gMidiOutPortFreeFunc(gMidiOutPort);
    }

    if(gMidiInPortFreeFunc)
    {
        gMidiInPortFreeFunc(midiInPort);
    }

    if(gMidiFreeFunc)
    {
        gMidiFreeFunc(midiPtr);
    }

    //Free the library:
    if (dllHandle)
    {
        FreeLibrary(dllHandle);
    }

    DeleteCriticalSection(&gCriticalSection);

    return 0;
}