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

#include "MidiClient.h"
#include "WindowsVersionHelper.h"
#include <iostream>
#include <string>
#include <conio.h>
#include <assert.h>
#include <functional>

#define USING_APP_MANIFEST
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;
using namespace WinRT;

MidiClient::MidiClient()
{
    mInitialized = false;
    mDllHandle = NULL;
    mMidiPtr = nullptr;
    mMidiInPort = nullptr;
    mMidiOutPort = nullptr;
    mWatcherPortCountFunc = nullptr;
    mWatcherPortNameFunc = nullptr;
    mWatcherPortTypeFunc = nullptr;
    mMidiInitFunc = nullptr;
    mMidiFreeFunc = nullptr;
    mMidiGetPortWatcherFunc = nullptr;
    mMidiInPortOpenFunc = nullptr;
    mMidiInPortFreeFunc = nullptr;
    mMidiOutPortOpenFunc = nullptr;
    mMidiOutPortFreeFunc = nullptr;
    mMidiOutPortSendFunc = nullptr;

    InitializeCriticalSection(&mCriticalSection);
}

MidiClient::~MidiClient()
{
    // clean up midi objects
    if (mMidiOutPortFreeFunc && mMidiOutPort)
    {
        mMidiOutPortFreeFunc(mMidiOutPort);
    }

    if (mMidiInPortFreeFunc && mMidiInPort)
    {
        mMidiInPortFreeFunc(mMidiInPort);
    }

    if (mMidiFreeFunc && mMidiPtr)
    {
        mMidiFreeFunc(mMidiPtr);
    }

    //Free the library:
    if (mDllHandle)
    {
        FreeLibrary(mDllHandle);
    }

    DeleteCriticalSection(&mCriticalSection);

}

void MidiClient::printPortNames(const WinRTMidiPortWatcherPtr watcher)
{
    if(watcher == nullptr)
    {
        return;
    }

    WinRTMidiPortType type = mWatcherPortTypeFunc(watcher);
    if(type == In)
    {
        cout << "MIDI In Ports" << endl;
    }
    else
    {
        cout << "MIDI Out Ports" << endl;
    }

    auto cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    int nPorts = mWatcherPortCountFunc(watcher);
    for (int i = 0; i < nPorts; i++)
    {
        const char* name = mWatcherPortNameFunc(watcher, i);
        cout << i << ": " << mWatcherPortNameFunc(watcher, i) << endl;
    }

    SetConsoleOutputCP(cp);

    cout << endl;
}

void MidiClient::midiPortChangedCallback(const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update)
{
    EnterCriticalSection(&mCriticalSection);
    string portName = mWatcherPortTypeFunc(portWatcher) == In ? "In" : "Out";

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
    LeaveCriticalSection(&mCriticalSection);
}

void MidiClient::midiPortChangedCallback(const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update, void* context)
{
    if (context != nullptr)
    {
        MidiClient* client = static_cast<MidiClient *>(context);
        client->midiPortChangedCallback(portWatcher, update);
    }
}

void MidiClient::midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes, void* context)
{
    if (context != nullptr)
    {
        MidiClient* client = static_cast<MidiClient *>(context);
        client->midiInCallback(port, timeStamp, message, nBytes);
    }
}

void MidiClient::midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes)
{
    assert(port == mMidiInPort);

    if(mMidiOutPort != nullptr)
    {
        mMidiOutPortSendFunc(mMidiOutPort, message, nBytes);
    }

    EnterCriticalSection(&mCriticalSection);
    for (unsigned int i = 0; i < nBytes; i++)
    {
        cout << "Byte " << i << " = " << (int)message[i] << ", ";
    }

    if(nBytes > 0)
    {
        cout << "timestamp = " << timeStamp << endl;
    }
    LeaveCriticalSection(&mCriticalSection);
}

WinRTMidiErrorType MidiClient::initialize()
{
    if (mInitialized)
    {
        return WINRT_NO_ERROR;
    }

    //Load the WinRTMidi dll
#ifdef USING_APP_MANIFEST
    if(windows10orGreaterWithManifest())
    {
        mDllHandle = LoadLibrary(L"WinRTMidi.dll");
    }
#else
    if (windows10orGreater())
    {
        mDllHandle = LoadLibrary(L"WinRTMidi.dll");
    }
#endif

    if(NULL == mDllHandle)
    {
        cout << "Unable to load WinRTMidi.dll" << endl;
        return WINRT_WINDOWS_VERSION_ERROR;
    }

    // GetWinRTMidi DLL function pointers. Error checking needs to be added!
    //Get pointer to the WinRTMidiInitializeFunc function using GetProcAddress:  
    mMidiInitFunc = reinterpret_cast<WinRTMidiInitializeFunc>(::GetProcAddress(mDllHandle, "winrt_initialize_midi"));

    //Get pointer to the WinRTMidiFreeFunc function using GetProcAddress:  
    mMidiFreeFunc = reinterpret_cast<WinRTMidiFreeFunc>(::GetProcAddress(mDllHandle, "winrt_free_midi"));

    //Get pointer to the WinRTMidiGetPortWatcherFunc function using GetProcAddress:  
    mMidiGetPortWatcherFunc = reinterpret_cast<WinRTMidiGetPortWatcherFunc>(::GetProcAddress(mDllHandle, "winrt_get_portwatcher"));

    //Get pointer to the WinRTMidiInPortOpenFunc function using GetProcAddress:
    mMidiInPortOpenFunc = reinterpret_cast<WinRTMidiInPortOpenFunc>(::GetProcAddress(mDllHandle, "winrt_open_midi_in_port"));

    //Get pointer to the WinRTMidiInPortFreeFunc function using GetProcAddress:
    mMidiInPortFreeFunc = reinterpret_cast<WinRTMidiInPortFreeFunc>(::GetProcAddress(mDllHandle, "winrt_free_midi_in_port"));

    //Get pointer to the WinRTMidiOutPortOpenFunc function using GetProcAddress:
    mMidiOutPortOpenFunc = reinterpret_cast<WinRTMidiOutPortOpenFunc>(::GetProcAddress(mDllHandle, "winrt_open_midi_out_port"));

    //Get pointer to the WinRTMidiOutPortFreeFunc function using GetProcAddress:
    mMidiOutPortFreeFunc = reinterpret_cast<WinRTMidiOutPortFreeFunc>(::GetProcAddress(mDllHandle, "winrt_free_midi_out_port"));

    //Get pointer to the WinRTMidiOutPortSendFunc function using GetProcAddress:
    mMidiOutPortSendFunc = reinterpret_cast<WinRTMidiOutPortSendFunc>(::GetProcAddress(mDllHandle, "winrt_midi_out_port_send"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    mWatcherPortCountFunc = reinterpret_cast<WinRTWatcherPortCountFunc>(::GetProcAddress(mDllHandle, "winrt_watcher_get_port_count"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    mWatcherPortNameFunc = reinterpret_cast<WinRTWatcherPortNameFunc>(::GetProcAddress(mDllHandle, "winrt_watcher_get_port_name"));

    //Get pointer to the WinRTWatcherPortCountFunc function using GetProcAddress:  
    mWatcherPortTypeFunc = reinterpret_cast<WinRTWatcherPortTypeFunc>(::GetProcAddress(mDllHandle, "winrt_watcher_get_port_type"));

    // initialize Midi interface
    WinRTMidiErrorType result = mMidiInitFunc(midiPortChangedCallback, &mMidiPtr, (void*)this);
    if(result != WINRT_NO_ERROR)
    {
        cout << "Unable to initialize WinRTMidi" << endl;
        goto cleanup;
    }

    // open midi in port 0
    result = mMidiInPortOpenFunc(mMidiPtr, 0, midiInCallback, &mMidiInPort, (void*)this);
    if(result != WINRT_NO_ERROR)
    {
        cout << "Unable to create Midi In port" << endl;
        goto cleanup;
    }

    // open midi out port 0
    result = mMidiOutPortOpenFunc(mMidiPtr, 0, &mMidiOutPort, (void*)this);
    if(result != WINRT_NO_ERROR)
    {
        cout << "Unable to create Midi Out port" << endl;
        goto cleanup;
    }

    // send a note on message to the midi out port
    unsigned char buffer[3] = { 144, 60 , 127 };
    cout << "Sending Note On to midi output port 0" << endl;
    mMidiOutPortSendFunc(mMidiOutPort, buffer, 3);

    Sleep(500);

    // send a note off message to the midi out port
    cout << "Sending Note Off to midi output port 0" << endl;
    buffer[0] = 128;
    mMidiOutPortSendFunc(mMidiOutPort, buffer, 3);

    // example on how get midi port info
    const WinRTMidiPortWatcherPtr watcher = mMidiGetPortWatcherFunc(mMidiPtr, In);
    if(watcher != nullptr)
    {
        unsigned int numPorts = mWatcherPortCountFunc(watcher);
        if(numPorts > 0)
        {
            const char* name = mWatcherPortNameFunc(watcher, 0);
        }
    }

    cout << endl << "Sending midi in data to midi out port..." << endl;

cleanup:
    return result;
}