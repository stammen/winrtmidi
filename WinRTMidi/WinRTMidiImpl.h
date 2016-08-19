#pragma once

#include <vector>

#include "WinRTMidi.h"
#include "WinRTMidiPortWatcher.h"

namespace WinRT
{
    public ref class WinRTMidiInPort sealed
    {
    public:
        WinRTMidiInPort();
        virtual ~WinRTMidiInPort();

        void OpenPort(Platform::String^ id);
        void ClosePort(void);

        void RemoveMidiInCallback() {
            mMessageReceivedCallback = nullptr;
        };

    internal:
        // needs to be internal as MidiInMessageReceivedCallbackType is not a WinRT type
        void SetMidiInCallback(WinRTMidiInCallback callback) {
            mMessageReceivedCallback = callback;
        };

    private:
        void OnMidiInMessageReceived(Windows::Devices::Midi::MidiInPort^ sender, Windows::Devices::Midi::MidiMessageReceivedEventArgs^ args);
        Windows::Devices::Midi::MidiInPort^ mMidiInPort;
        long long mLastMessageTime;
        bool mFirstMessage;
        WinRTMidiInCallback mMessageReceivedCallback;
        std::vector<unsigned char> mMidiMessage;
    };

    public ref class WinRTMidiOutPort sealed
    {
    public:
        WinRTMidiOutPort();
        virtual ~WinRTMidiOutPort();
        
        void OpenPort(Platform::String^ id);
        void ClosePort(void);

    internal:

    private:
        Windows::Devices::Midi::IMidiOutPort^ mMidiOutPort;
    };

    class WinRTMidi
    {
    public:
        WinRTMidi(MidiPortChangedCallback callback)
        {
            mMidiInPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::In, callback);
            mMidiOutPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::Out, callback);
        }

        WinRTMidiPortWatcher^ GetWatcher(WinRTMidiPortType type)
        {
            switch (type)
            {
            case WinRTMidiPortType::In:
                return mMidiInPortWatcher;
                break;
            case WinRTMidiPortType::Out:
                return mMidiOutPortWatcher;
                break;
            }

            return nullptr;
        }

        Platform::String^ getPortId(WinRTMidiPortType type, unsigned int index)
        {
            auto watcher = GetWatcher(type);
            if (watcher)
            {
                return watcher->GetPortId(index);
            }

            return nullptr;
        }

    private:

        WinRTMidiPortWatcher^ mMidiInPortWatcher;
        WinRTMidiPortWatcher^ mMidiOutPortWatcher;
    };

    class MidiInPortWrapper
    {
    public:
        MidiInPortWrapper(WinRTMidiInPort^ port, WinRTMidiInCallback callback)
            : mPort(port)
        {
            mPort->SetMidiInCallback(callback);
        }

        WinRTMidiInPort^ mPort;
    };
};

