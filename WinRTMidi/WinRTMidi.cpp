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

