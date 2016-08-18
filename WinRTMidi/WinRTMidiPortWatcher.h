#pragma once

#include <vector>
#include <memory>
#include <string>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "IWinRTMidiPortWatcher.h"

namespace WinRT
{
    std::string PlatformStringToString(Platform::String^ s);
    ref class WinRTMidiPortWatcher;
    class MidiPortWatcherImp;

    class WinRTMidiPortInfo
    {
    public:
        WinRTMidiPortInfo(Platform::String^ name, Platform::String^ id)
            : mName(name)
            , mID(id)
        {
        };

        virtual ~WinRTMidiPortInfo() {
        };

        Platform::String^ mName;
        Platform::String^ mID;
    };

    // C++ Midi port changed callback
    typedef std::function<void(WinRTMidiPortWatcher^ watcher, WinRTMidiPortUpdateType update)> MidiPortChangedCallbackType;

    // WinRT Delegate
    delegate void MidiPortUpdateHandler(WinRTMidiPortWatcher^ sender, WinRTMidiPortUpdateType update);

    ref class WinRTMidiPortWatcher
    {
    public:

        WinRTMidiPortType GetPortType() { return mPortType; };
        unsigned int GetPortCount();
        Platform::String^ GetPortId(unsigned int portNumber);

        event MidiPortUpdateHandler^ mMidiPortUpdateEventHander;
        void OnMidiPortUpdated(WinRTMidiPortUpdateType update);

        void RemoveMidiPortChangedCallback() {
            mPortChangedCallback = nullptr;
        };

    internal:
        // needs to be internal as MidiPortChangedCallbackType is not a WinRT type
        void SetMidiPortChangedCallback(const IMidiPortChangedCallbackType& callback) {
            mPortChangedCallback = callback;
        };
       
        // Constructor needs to be internal as this is an unsealed ref base class
        WinRTMidiPortWatcher(WinRTMidiPortType type);

        // needs to be internal as std::string is not a WinRT type
        std::string WinRTMidiPortWatcher::GetPortName(unsigned int portNumber);

        const IWinRTMidiPortWatcher* GetIMidiPortWatcher();

    private:
        void OnDeviceAdded(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformation^ args);
        void OnDeviceRemoved(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformationUpdate^ args);
        void OnDeviceUpdated(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformationUpdate^ args);
        void OnDeviceEnumerationCompleted(Windows::Devices::Enumeration::DeviceWatcher^ sender, Platform::Object^ args);

        void CheckForEnumeration();

        Windows::Devices::Enumeration::DeviceWatcher^ mPortWatcher;
        std::vector<std::unique_ptr<WinRTMidiPortInfo>> mPortInfo;
        std::mutex mEnumerationMutex;
        std::condition_variable mSleepCondition;

        IMidiPortChangedCallbackType mPortChangedCallback;
        WinRTMidiPortType mPortType;
        bool mPortEnumerationComplete;
        std::shared_ptr<MidiPortWatcherImp> mIPortWatcher;
    };

    // Midi In Port Watcher (Singleton)
    ref class WinRTMidiInPortWatcher sealed : public WinRTMidiPortWatcher
    {
    public:
        // C++11 thread-safe Singleton pattern (VS2015 or later)
        static WinRTMidiInPortWatcher^ getInstance() {
            static WinRTMidiInPortWatcher^ instance = ref new WinRTMidiInPortWatcher();
            return instance;
        }

    private:
        WinRTMidiInPortWatcher()
            : WinRTMidiPortWatcher(WinRTMidiPortType::In)
        {
        }
    };

    // Midi Out Port Watcher (Singleton)
    ref class WinRTMidiOutPortWatcher sealed : public WinRTMidiPortWatcher
    {
    public:
        // C++11 thread-safe Singleton pattern (VS2015 or later)
        static WinRTMidiOutPortWatcher^ getInstance() {
            static WinRTMidiOutPortWatcher^ instance = ref new WinRTMidiOutPortWatcher();
            return instance;
        }

    private:
        WinRTMidiOutPortWatcher()
            : WinRTMidiPortWatcher(WinRTMidiPortType::Out)
        {}
    };

    class MidiPortWatcherImp : public IWinRTMidiPortWatcher
    {
    public:

        MidiPortWatcherImp()
        {}

        virtual ~MidiPortWatcherImp() {};

        virtual unsigned int GetPortCount() const override {
            return mWatcher->GetPortCount();
        };

        virtual std::string GetPortName(unsigned int portNumber) const override
        {
            return mWatcher->GetPortName(portNumber);
        }

        virtual WinRTMidiPortType GetPortType() const override {
            return mWatcher->GetPortType();
        };

        void SetWatcher(WinRTMidiPortWatcher^ watcher) { mWatcher = watcher; };

    private:
        WinRTMidiPortWatcher^ mWatcher;
    };

    extern "C" __declspec(dllexport) void __cdecl SetMidiPortChangedCallback(IMidiPortChangedCallbackType& callback);
    extern "C" __declspec(dllexport) const IWinRTMidiPortWatcher* __cdecl GetIMidiPortWatcher(WinRTMidiPortType portType);
};




