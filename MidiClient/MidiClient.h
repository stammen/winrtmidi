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

#pragma once

#include "WinRTMidi.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace WinRT
{
    class MidiClient 
    {
    public:
        MidiClient();
        ~MidiClient();
        WinRTMidiErrorType initialize();


    private:
        static void midiPortChangedCallback(const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update, void* contect);
        void midiPortChangedCallback(const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update);
        static void midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes, void* contect);
        void midiInCallback(const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes);
        void printPortNames(const WinRTMidiPortWatcherPtr watcher);

        // WinRTMidi DLL function pointers
        WinRTWatcherPortCountFunc   mWatcherPortCountFunc;
        WinRTWatcherPortNameFunc    mWatcherPortNameFunc;
        WinRTWatcherPortTypeFunc    mWatcherPortTypeFunc;
        WinRTMidiInitializeFunc     mMidiInitFunc;
        WinRTMidiFreeFunc           mMidiFreeFunc;
        WinRTMidiGetPortWatcherFunc mMidiGetPortWatcherFunc;
        WinRTMidiInPortOpenFunc     mMidiInPortOpenFunc;
        WinRTMidiInPortFreeFunc     mMidiInPortFreeFunc;
        WinRTMidiOutPortOpenFunc    mMidiOutPortOpenFunc;
        WinRTMidiOutPortFreeFunc    mMidiOutPortFreeFunc;
        WinRTMidiOutPortSendFunc    mMidiOutPortSendFunc;

        // WinRTPtr
        WinRTMidiPtr mMidiPtr;

        // Midi In port
        WinRTMidiInPortPtr mMidiInPort;

        // Midi Out port
        WinRTMidiOutPortPtr mMidiOutPort;

        // WinRTMidi DLL Handle
        HINSTANCE mDllHandle;

        bool mInitialized;

        CRITICAL_SECTION mCriticalSection;
    };
};

