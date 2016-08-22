// MidiClient.cpp 

#include "stdafx.h"
#include "WinRTMidi.h"
#include <iostream>
#include <mutex>
#include <string>

using namespace std;
using namespace WinRT;

std::mutex g_mutex;  

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

void printPortNames(const WinRTMidiPortWatcherPtr watcher)
{
    if (watcher == nullptr)
    {
        return;
    }

    WinRTMidiPortType type = gWatcherPortTypeFunc(watcher);
    if (type == WinRTMidiPortType::In)
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
    lock_guard<mutex> lock(g_mutex);
    string portName = gWatcherPortTypeFunc(portWatcher) == WinRTMidiPortType::In ? "In" : "Out";

    switch (update)
    {
    case WinRTMidiPortUpdateType::PortAdded:
        cout << "***MIDI " << portName << " port added***" << endl;
        break;

    case WinRTMidiPortUpdateType::PortRemoved:
        cout << "***MIDI " << portName << " port removed***" << endl;
        break;

    case WinRTMidiPortUpdateType::EnumerationComplete:
        cout << "***MIDI " << portName << " port enumeration complete***" << endl;
        break;
    }

    printPortNames(portWatcher);
}

void midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes)
{
    if (gMidiOutPort != nullptr)
    {
        gMidiOutPortSendFunc(gMidiOutPort, message, nBytes);
    }

    lock_guard<mutex> lock(g_mutex);
    for (unsigned int i = 0; i < nBytes; i++)
    {
        cout << "Byte " << i << " = " << (int)message[i] << ", ";
    }

    if (nBytes > 0)
    {
        cout << "timestamp = " << timeStamp << endl;
    }
}

int main()
{
    HINSTANCE dllHandle = NULL;
    WinRTMidiPtr midiPtr = nullptr;
    WinRTMidiInPortPtr midiInPort = nullptr;

    //Load the WinRTMidi dll
    dllHandle = LoadLibrary(L"WinRTMidi.dll");
    if (NULL == dllHandle)
    {
        cout << "Unable to load WinRTMidi.dll" << endl;
        return -1;
    }
  
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
    midiPtr = gMidiInitFunc(midiPortChangedCallback);

    // open Midi In port 0
    midiInPort = gMidiInPortOpenFunc(midiPtr, 0, midiInCallback);

    // open Midi out port 1
    gMidiOutPort = gMidiOutPortOpenFunc(midiPtr, 1);

    const WinRTMidiPortWatcherPtr watcher = gMidiGetPortWatcher(midiPtr, WinRTMidiPortType::In);
    unsigned int numPorts = gWatcherPortCountFunc(watcher);
    const char* name = gWatcherPortNameFunc(watcher, 0);

    // process midi until user presses key on keyboard
    char c = getchar();

    // clean up midi objects
    gMidiOutPortFreeFunc(gMidiOutPort);
    gMidiInPortFreeFunc(midiInPort);
    gMidiFreeFunc(midiPtr);

    //Free the library:
    FreeLibrary(dllHandle);

    return 0;
}