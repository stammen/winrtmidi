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

#include "WinRTMidi.h"
#include "WinRTMidiImpl.h"
#include "WinRTMidiportWatcher.h"
#include <wrl\wrappers\corewrappers.h>

namespace WinRT
{
    WinRTMidiErrorType winrt_initialize_midi(MidiPortChangedCallback callback, WinRTMidiPtr* winrtMidi)
    {
        *winrtMidi = nullptr;

        // Initialize the Windows Runtime.
        static Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);

        if (!SUCCEEDED(initialize.operator HRESULT()))
        {
            return WINRT_WINDOWS_RUNTIME_ERROR;
        }

        // Check if Windows 10 midi api is supported
        if (!Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.Devices.Midi.MidiInPort"))
        {
            return WINRT_WINDOWS_VERSION_ERROR;
        }

        // attempt to initialize the Midi Portwatchers
        WinRTMidi* midi = new WinRTMidi(callback);
        WinRTMidiErrorType result = midi->Initialize();
        if(result != WINRT_NO_ERROR)
        {
            delete midi;
            *winrtMidi = nullptr;
        }
        else
        {
            *winrtMidi = (WinRTMidiPtr)midi;
        }

        return result;
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

    WinRTMidiErrorType winrt_open_midi_in_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiInCallback callback, WinRTMidiInPortPtr* midiPort)
    {
        *midiPort = nullptr;
        WinRTMidiErrorType result = WINRT_NO_ERROR;

        WinRTMidi* midiPtr = (WinRTMidi*)midi;

        if (midiPtr == nullptr)
        {
            return WINRT_INVALID_PARAMETER_ERROR;
        }

        auto id = midiPtr->getPortId(WinRTMidiPortType::In, index);
        if (id == nullptr)
        {
            return WINRT_INVALID_PORT_INDEX_ERROR;
        }

        auto port = ref new WinRTMidiInPort;
        result = port->OpenPort(id);
        if (result == WINRT_NO_ERROR)
        {
            *midiPort = (WinRTMidiInPortPtr) new MidiInPortWrapper(port, callback);
        }
        return result;
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
    WinRTMidiErrorType winrt_open_midi_out_port(WinRTMidiPtr midi, unsigned int index, WinRTMidiOutPortPtr* midiPort)
    {
        *midiPort = nullptr;
        WinRTMidiErrorType result = WINRT_NO_ERROR;

        WinRTMidi* midiPtr = (WinRTMidi*)midi;

        if (midiPtr == nullptr)
        {
            return WINRT_INVALID_PARAMETER_ERROR;
        }

        auto id = midiPtr->getPortId(WinRTMidiPortType::Out, index);
        if (id == nullptr)
        {
            return WINRT_INVALID_PORT_INDEX_ERROR;
        }

        auto port = ref new WinRTMidiOutPort;
        result = port->OpenPort(id);
        if (result == WINRT_NO_ERROR)
        {
            *midiPort = (WinRTMidiOutPortPtr) new MidiOutPortWrapper(port);
        }
        return result;
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

