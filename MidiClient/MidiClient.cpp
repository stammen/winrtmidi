// MathClient.cpp : Defines the entry point for the console application.
// Compile by using: cl /EHsc /link MathLibrary.lib MathClient.cpp

#include "stdafx.h"
#include <iostream>
#include <iostream>
#include <mutex>
#include <windows.h>

#include "IWinRTMidiPortWatcher.h"
#include "IWinRTMidiPort.h"

using namespace std;
using namespace WinRT;


std::mutex g_mutex;  

void printPortNames(const IWinRTMidiPortWatcher* watcher)
{
    if (watcher == nullptr)
    {
        return;
    }

    if (watcher->GetPortType() == WinRTMidiPortType::In)
    {
        cout << "MIDI In Ports" << endl;
    }
    else
    {
        cout << "MIDI Out Ports" << endl;
    }

    int nPorts = watcher->GetPortCount();
    for (int i = 0; i < nPorts; i++)
    {
        cout << i << ": " << watcher->GetPortName(i) << endl;
    }

    cout << endl;
}

void midiPortChangedCallback(const IWinRTMidiPortWatcher* watcher, WinRTMidiPortUpdateType update)
{
    lock_guard<mutex> lock(g_mutex);

    string portName = watcher->GetPortType() == WinRTMidiPortType::In ? "In" : "Out";

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

    printPortNames(watcher);
}

void midiInCallback(const IWinRTMidiInPort* midiInPort, double deltatime, vector< unsigned char > *message)
{
    unsigned int nBytes = static_cast<unsigned int>(message->size());
    for (unsigned int i = 0; i < nBytes; i++)
    {
        cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    }

    if (nBytes > 0)
    {
        cout << "timestamp = " << deltatime << endl;
    }
}

int main(Platform::Array<Platform::String^>^ args) 
{
    HINSTANCE dllHandle = NULL;
    IWinRTMidiInPort* midiInPort = nullptr;

    //Load the dll and keep the handle to it
    dllHandle = LoadLibrary(L"WinRTMidi.dll");

    // If the handle is valid, try to get the function addresses. 
    if (NULL != dllHandle)
    {
        //Get pointer to the setIMidiPortChangedCallback function using GetProcAddress:  
        SetIMidiPortChangedCallbackFunc setCallbackFunc = reinterpret_cast<SetIMidiPortChangedCallbackFunc>(::GetProcAddress(dllHandle, "SetMidiPortChangedCallback"));

        if (NULL != setCallbackFunc)
        {
            setCallbackFunc(&midiPortChangedCallback);
        }

        //Get pointer to the setIMidiPortChangedCallback function using GetProcAddress:
        GetIMidiPortWatcherFunc getMidiPortWatcherFunc = reinterpret_cast<GetIMidiPortWatcherFunc>(::GetProcAddress(dllHandle, "GetIMidiPortWatcher"));

        if (NULL != getMidiPortWatcherFunc)
        {
            IWinRTMidiPortWatcher* watcher = getMidiPortWatcherFunc(WinRTMidiPortType::In);
            int n = watcher->GetPortCount();
            std::string name = watcher->GetPortName(0);
        }

        //Get pointer to the IWinRTMidiInPortFactoryFunc function using GetProcAddress:
        IWinRTMidiInPortFactoryFunc IMidiInPortFactoryFunc = reinterpret_cast<IWinRTMidiInPortFactoryFunc>(::GetProcAddress(dllHandle, "IWinRTMidiInPortFactory"));

        if (NULL != IMidiInPortFactoryFunc)
        {
            midiInPort = IMidiInPortFactoryFunc();
            midiInPort->OpenPort(0);
            midiInPort->SetCallback(&midiInCallback);
        }

        char c = getchar();

        if (midiInPort)
        {
            midiInPort->Destroy();
        }

        //Free the library:
        FreeLibrary(dllHandle);
    }

    return 0;
}