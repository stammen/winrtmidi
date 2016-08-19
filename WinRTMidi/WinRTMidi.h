#pragma once

#include <Windows.h>

#if defined(WINRTMIDI_EXPORT)
#define WINRTMIDI_API extern "C" __declspec(dllexport)
#else
#define WINRTMIDI_API extern "C" __declspec(dllimport)
#endif

namespace WinRT
{
    enum class WinRTMidiPortType : int { In, Out };
    enum class WinRTMidiPortUpdateType : int { PortAdded, PortRemoved, EnumerationComplete };

    typedef void* WinRTMidiPtr;
    typedef void* WinRTMidiPortWatcherPtr;
    typedef void* WinRTMidiInPortPtr;
    typedef void* WinRTMidiOutPortPtr;

    // Midi port changed callback
    typedef void(*MidiPortChangedCallback) (const WinRTMidiPortWatcherPtr portWatcher, WinRTMidiPortUpdateType update);

    // Midi In callback
    typedef void(*WinRTMidiInCallback) (const WinRTMidiInPortPtr port, double timeStamp, const unsigned char* message, unsigned int nBytes);

    // WinRT Midi Functions
    typedef WinRTMidiPtr(__cdecl *WinRTMidiInitializeFunc)(MidiPortChangedCallback callback);
    WINRTMIDI_API WinRTMidiPtr __cdecl winrt_initialize_midi(MidiPortChangedCallback callback);
 
    typedef void(__cdecl *WinRTMidiFreeFunc)(WinRTMidiPtr midi);
    WINRTMIDI_API void __cdecl winrt_free_midi(WinRTMidiPtr midi);

    // WinRT Midi In Port Functions
    typedef WinRTMidiInPortPtr(__cdecl *WinRTMidiInPortOpenFunc)(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback);
    WINRTMIDI_API WinRTMidiInPortPtr __cdecl winrt_open_midi_in_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback);

    typedef void(__cdecl *WinRTMidiInPortFreeFunc)(WinRTMidiInPortPtr port);
    WINRTMIDI_API void __cdecl winrt_free_midi_in_port(WinRTMidiInPortPtr port);

    // WinRT Midi Watcher Functions
    typedef unsigned int(__cdecl *WinRTWatcherPortCountFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API unsigned int __cdecl winrt_watcher_get_port_count(WinRTMidiPortWatcherPtr watcher);

    typedef const char*(__cdecl *WinRTWatcherPortNameFunc)(WinRTMidiPortWatcherPtr watcher, unsigned int index);
    WINRTMIDI_API const char* __cdecl winrt_watcher_get_port_name(WinRTMidiPortWatcherPtr watcher, unsigned int index);

    typedef WinRTMidiPortType(__cdecl *WinRTWatcherPortTypeFunc)(WinRTMidiPortWatcherPtr watcher);
    WINRTMIDI_API WinRTMidiPortType __cdecl winrt_watcher_get_port_type(WinRTMidiPortWatcherPtr watcher);
}
 