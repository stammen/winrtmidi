// MidiClient.cpp 

#include "stdafx.h"
#include "WinRTMidi.h"
#include <iostream>
#include <mutex>
#include <string>

using namespace std;
using namespace WinRT;

std::mutex g_mutex;  

WinRTWatcherPortCountFunc gWatcherPortCountFunc = nullptr;
WinRTWatcherPortNameFunc gWatcherPortNameFunc = nullptr;
WinRTWatcherPortTypeFunc gWatcherPortTypeFunc = nullptr;



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
    //string portName = portWatcher->GetPortType() == WinRTMidiPortType::In ? "In" : "Out";
    std::string portName = "In";

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

    //Load the dll and keep the handle to it
    dllHandle = LoadLibrary(L"WinRTMidi.dll");

    // If the handle is valid, try to get the function addresses. 
    if (NULL != dllHandle)
    {
        //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
        gWatcherPortCountFunc = reinterpret_cast<WinRTWatcherPortCountFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_count"));

        //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
        gWatcherPortNameFunc = reinterpret_cast<WinRTWatcherPortNameFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_name"));

        //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
        gWatcherPortTypeFunc = reinterpret_cast<WinRTWatcherPortTypeFunc>(::GetProcAddress(dllHandle, "winrt_watcher_get_port_type"));


        //Get pointer to the WinRTMidiInitializeFunc function using GetProcAddress:  
        WinRTMidiInitializeFunc MidiInitFunc = reinterpret_cast<WinRTMidiInitializeFunc>(::GetProcAddress(dllHandle, "winrt_initialize_midi"));
        if (NULL != MidiInitFunc)
        {
            midiPtr = MidiInitFunc(midiPortChangedCallback);
        }

        //Get pointer to the WinRTMidiInitializeFunc function using GetProcAddress:  
        WinRTMidiFreeFunc MidiFreeFunc = reinterpret_cast<WinRTMidiFreeFunc>(::GetProcAddress(dllHandle, "winrt_free_midi"));

#if 0
        //Get pointer to the setMidiPortChangedCallback function using GetProcAddress:  
        SetMidiPortChangedCallbackFunc setCallbackFunc = reinterpret_cast<SetMidiPortChangedCallbackFunc>(::GetProcAddress(dllHandle, "SetMidiPortChangedCallback"));
        if (NULL != setCallbackFunc)
        {
            setCallbackFunc(&midiPortChangedCallback);
        }

        //Get pointer to the setMidiPortChangedCallback function using GetProcAddress:
        GetMidiPortWatcherFunc getMidiPortWatcherFunc = reinterpret_cast<GetMidiPortWatcherFunc>(::GetProcAddress(dllHandle, "GetMidiPortWatcher"));
        if (NULL != getMidiPortWatcherFunc)
        {
            IWinRTMidiPortWatcher* watcher = getMidiPortWatcherFunc(WinRTMidiPortType::In);
            int n = watcher->GetPortCount();
            std::string name = watcher->GetPortName(0);
        }
#endif

        //Get pointer to the MidiInPortFreeFunc function using GetProcAddress:
        WinRTMidiInPortFreeFunc MidiInPortFreeFunc = reinterpret_cast<WinRTMidiInPortFreeFunc>(::GetProcAddress(dllHandle, "winrt_free_midi_in_port"));


        //Get pointer to the MidiInPortOpenFunc function using GetProcAddress:
        WinRTMidiInPortOpenFunc MidiInPortOpenFunc = reinterpret_cast<WinRTMidiInPortOpenFunc>(::GetProcAddress(dllHandle, "winrt_open_midi_in_port"));
        if (NULL != MidiInPortOpenFunc)
        {
            midiInPort = MidiInPortOpenFunc(midiPtr, 0, midiInCallback);
        }

        char c = getchar();

        // clean up Midi objects
        MidiInPortFreeFunc(midiInPort);
        MidiFreeFunc(midiPtr);

        //Free the library:
        FreeLibrary(dllHandle);
    }

    return 0;
}