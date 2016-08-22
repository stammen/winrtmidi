#include "WinRTMidi.h"
#include "WinRTMidiImpl.h"
#include "WinRTMidiportWatcher.h"
#include <wrl\wrappers\corewrappers.h>

namespace WinRT
{
    WinRTMidiPtr winrt_initialize_midi(MidiPortChangedCallback callback)
    {
        // Initialize the Windows Runtime.
        static Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
        return (WinRTMidiPtr) new WinRTMidi(callback);
    }

    void winrt_free_midi(WinRTMidiPtr midi)
    {
        if (midi)
        {
            delete midi;
        }
    }

    const WinRTMidiPortWatcherPtr winrt_get_portwatcher(WinRTMidiPtr midi, WinRTMidiPortType type)
    {
        WinRTMidi* midiPtr = (WinRTMidi*)midi;
        return midiPtr->GetPortWatcherWrapper(type);
    }

    WinRTMidiInPortPtr winrt_open_midi_in_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback)
    {
        WinRTMidi* midiPtr = (WinRTMidi*)midi;
        auto id = midiPtr->getPortId(WinRTMidiPortType::In, index);
        auto port = ref new WinRTMidiInPort;
        port->OpenPort(id);
        return (WinRTMidiInPortPtr) new MidiInPortWrapper(port, callback);
    }

    void winrt_free_midi_in_port(WinRTMidiInPortPtr port)
    {
        MidiInPortWrapper* wrapper = (MidiInPortWrapper*)port;
        if (wrapper)
        {
            delete wrapper;
        }
    }

    // WinRT Midi Out port functions
    WinRTMidiOutPortPtr winrt_open_midi_out_port(WinRTMidiPtr midi, unsigned int index)
    {
        WinRTMidi* midiPtr = (WinRTMidi*)midi;
        auto id = midiPtr->getPortId(WinRTMidiPortType::Out, index);
        auto port = ref new WinRTMidiOutPort;
        port->OpenPort(id);
        return (WinRTMidiOutPortPtr) new MidiOutPortWrapper(port);
    }

    void winrt_free_midi_out_port(WinRTMidiOutPortPtr port)
    {
        MidiOutPortWrapper* wrapper = (MidiOutPortWrapper*)port;
        if (wrapper)
        {
            delete wrapper;
        }
    }

    void winrt_midi_out_port_send(WinRTMidiOutPortPtr port, const unsigned char* message, unsigned int nBytes)
    {
        MidiOutPortWrapper* wrapper = (MidiOutPortWrapper*)port;
        wrapper->getPort()->Send(message, nBytes);
    }

    // WinRT Midi Watcher Functions
    unsigned int winrt_watcher_get_port_count(WinRTMidiPortWatcherPtr watcher)
    {
        MidiPortWatcherWrapper* wrapper = (MidiPortWatcherWrapper*)watcher;
        return wrapper->GetWatcher()->GetPortCount();
    }

    const char* winrt_watcher_get_port_name(WinRTMidiPortWatcherPtr watcher, unsigned int index)
    {
        MidiPortWatcherWrapper* wrapper = (MidiPortWatcherWrapper*)watcher;
        return wrapper->GetWatcher()->GetPortName(index).c_str();
    }

    WinRTMidiPortType winrt_watcher_get_port_type(WinRTMidiPortWatcherPtr watcher)
    {
        MidiPortWatcherWrapper* wrapper = (MidiPortWatcherWrapper*)watcher;
        return wrapper->GetWatcher()->GetPortType();
    }
}

