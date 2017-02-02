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

#if defined(WINRTMIDI_EXPORT)
#define WINRTMIDI_API extern "C" __declspec(dllexport)
#else
#define WINRTMIDI_API extern "C" __declspec(dllimport)
#endif

namespace WinRT
{
    enum WinRTMidiPortType { In, Out };
    enum WinRTMidiPortUpdateType { PortAdded, PortRemoved, EnumerationComplete };

    enum WinRTMidiErrorType {
        WINRT_NO_ERROR = 0,                         //no error
        WINRT_WINDOWS_RUNTIME_ERROR,                // unable to initialize Windows Runtime
        WINRT_WINDOWS_VERSION_ERROR,                // version of Windows does not support Windows::Devices::Midi api
        WINRT_PORTWATCHER_INITIALIZATION_ERROR,     // error initialize midi port watcher
        WINRT_INVALID_PORT_INDEX_ERROR,             // requested port index is out of range
        WINRT_OPEN_PORT_ERROR,                      // open midi port error
        WINRT_INVALID_PARAMETER_ERROR,
        WINRT_MEMORY_ERROR, 
        WINRT_UNSPECIFIED_ERROR
    };

    typedef void* WinRTMidiPtr;
    typedef void* WinRTMidiPortWatcherPtr;
    typedef void* WinRTMidiInPortPtr;
    typedef void* WinRTMidiOutPortPtr;

    // Midi port changed callback
    // 
    typedef void(*MidiPortChangedCallback) (const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update, void* context);

    // Midi In callback
    // Note: timeStamp is now the duration from when the MidiInPort was created to the time the midi message was received. Client will need to
    // calculate the delta times between midi messages. Timestamp value is in milliseconds.
    // To receive delta times (previous behavior), define WINRTMIDI_REPORT_DELTA_TIME and rebuild the winrtmidi dll.
    //#define WINRTMIDI_REPORT_DELTA_TIME
    typedef void(*WinRTMidiInCallback) (const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes, void* context);

    // WinRT Midi Functions
    typedef WinRTMidiErrorType(__cdecl *WinRTMidiInitializeFunc)(MidiPortChangedCallback callback, WinRTMidiPtr* midi, void* context);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_initialize_midi(MidiPortChangedCallback callback, WinRTMidiPtr* winrtMidi, void* context);
 
    typedef void(__cdecl *WinRTMidiFreeFunc)(WinRTMidiPtr midi);
    WINRTMIDI_API void __cdecl winrt_free_midi(WinRTMidiPtr midi);

    typedef const WinRTMidiPortWatcherPtr(__cdecl *WinRTMidiGetPortWatcherFunc)(WinRTMidiPtr midi, WinRTMidiPortType type);
    WINRTMIDI_API const WinRTMidiPortWatcherPtr __cdecl winrt_get_portwatcher(WinRTMidiPtr midi, WinRTMidiPortType type);

    // WinRT Midi In Port Functions
    typedef WinRTMidiErrorType(__cdecl *WinRTMidiInPortOpenFunc)(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback, WinRTMidiInPortPtr* midiPort, void* context);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_open_midi_in_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback, WinRTMidiInPortPtr* midiPort, void* context);

    typedef void(__cdecl *WinRTMidiInPortFreeFunc)(WinRTMidiInPortPtr port);
    WINRTMIDI_API void __cdecl winrt_free_midi_in_port(WinRTMidiInPortPtr port);

    // WinRT Midi Out Port Functions
    typedef WinRTMidiErrorType(__cdecl *WinRTMidiOutPortOpenFunc)(WinRTMidiPtr midi, unsigned int index, WinRTMidiOutPortPtr* midiPort, void* context);
    WINRTMIDI_API WinRTMidiErrorType __cdecl winrt_open_midi_out_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiOutPortPtr* midiPort, void* context);

    typedef void(__cdecl *WinRTMidiOutPortFreeFunc)(WinRTMidiOutPortPtr port);
    WINRTMIDI_API void __cdecl winrt_free_midi_out_port(WinRTMidiOutPortPtr port);

    typedef void(__cdecl *WinRTMidiOutPortSendFunc)(WinRTMidiOutPortPtr port, const unsigned char* message, unsigned int nBytes);
    WINRTMIDI_API void __cdecl winrt_midi_out_port_send(WinRTMidiOutPortPtr port, const unsigned char* message, unsigned int nBytes);

    // WinRT Midi Watcher Functions
    typedef unsigned int(__cdecl *WinRTWatcherPortCountFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API unsigned int __cdecl winrt_watcher_get_port_count(WinRTMidiPortWatcherPtr watcher);

    typedef const char*(__cdecl *WinRTWatcherPortNameFunc)(WinRTMidiPortWatcherPtr watcher, unsigned int index);
    WINRTMIDI_API const char* __cdecl winrt_watcher_get_port_name(WinRTMidiPortWatcherPtr watcher, unsigned int index);

    typedef WinRTMidiPortType(__cdecl *WinRTWatcherPortTypeFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API WinRTMidiPortType __cdecl winrt_watcher_get_port_type(WinRTMidiPortWatcherPtr watcher);
};
 