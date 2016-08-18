#pragma once

#include <functional>
#include <vector>

#include "IWinRTMidiPort.h"

namespace WinRT
{
    // Midi In Message callback
    typedef std::function<void(double timestamp, std::vector<unsigned char>* message)> MidiInMessageReceivedCallbackType;

    public ref class WinRTMidiInPort sealed
    {
    public:
        WinRTMidiInPort();
        virtual ~WinRTMidiInPort();

        void OpenPort(int index);
        void ClosePort(void);

        void RemoveMidiInCallback() {
            mMessageReceivedCallback = nullptr;
        };

    internal:
        // needs to be internal as MidiInMessageReceivedCallbackType is not a WinRT type
        void SetMidiInCallback(const MidiInMessageReceivedCallbackType& callback) {
            mMessageReceivedCallback = callback;
        };

    private:
        void OpenPort(Platform::String^ id);
        void OnMidiInMessageReceived(Windows::Devices::Midi::MidiInPort^ sender, Windows::Devices::Midi::MidiMessageReceivedEventArgs^ args);
        Windows::Devices::Midi::MidiInPort^ mMidiInPort;
        long long mLastMessageTime;
        bool mFirstMessage;
        MidiInMessageReceivedCallbackType mMessageReceivedCallback;
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

    class MidiInPortImpl : public IWinRTMidiInPort {
    public:
        MidiInPortImpl::MidiInPortImpl();
        virtual MidiInPortImpl::~MidiInPortImpl();
        virtual void OpenPort(unsigned int index) override;
        virtual void ClosePort() override;
        virtual void RemoveCallback() override;
        virtual void SetCallback(const IMidiInCallbackType& callback) override;
        virtual void Destroy() override;

    private:
        void MidiInCallback(double timestamp, std::vector<unsigned char>* message);

        WinRTMidiInPort^ mPort;
        IMidiInCallbackType mCallback;
    };

    extern "C" __declspec(dllexport) IWinRTMidiInPort* __cdecl IWinRTMidiInPortFactory()
    {
        return new MidiInPortImpl;
    }
};

