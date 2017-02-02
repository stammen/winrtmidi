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

#include <vector>
#include <memory>
#include <string>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "WinRTMidi.h"

namespace WinRT
{
    std::string PlatformStringToString(Platform::String^ s);
    ref class WinRTMidiPortWatcher;

    class WinRTMidiPortInfo
    {
    public:
        WinRTMidiPortInfo(std::string& name, Platform::String^ id)
            : mName(name)
            , mID(id)
        {
        };

        virtual ~WinRTMidiPortInfo() {
        };

        std::string mName;
        Platform::String^ mID;
    };

    // C++ Midi port changed callback
    typedef std::function<void(WinRTMidiPortWatcher^ watcher, WinRTMidiPortUpdateType update)> MidiPortChangedCallbackType;

    // WinRT Delegate
    delegate void MidiPortUpdateHandler(WinRTMidiPortWatcher^ sender, WinRTMidiPortUpdateType update);

    ref class WinRTMidiPortWatcher
    {
    public:

    internal:
        WinRTMidiErrorType Initialize();
        unsigned int GetPortCount();
        Platform::String^ GetPortId(unsigned int portNumber);

        void RemoveMidiPortChangedCallback() {
            mPortChangedCallback = nullptr;
        };

        WinRTMidiPortType GetPortType() { return mPortType; };
        event MidiPortUpdateHandler^ mMidiPortUpdateEventHander;
        void OnMidiPortUpdated(WinRTMidiPortUpdateType update);
        
        // needs to be internal as MidiPortChangedCallbackType is not a WinRT type
        void SetMidiPortChangedCallback(const MidiPortChangedCallback callback) {
            mPortChangedCallback = callback;
        };
       
        // Constructor needs to be internal as this is an unsealed ref base class
        WinRTMidiPortWatcher(WinRTMidiPortType type, MidiPortChangedCallback callback = nullptr, void* context = nullptr);

        // needs to be internal as std::string is not a WinRT type
        const std::string& WinRTMidiPortWatcher::GetPortName(unsigned int portNumber);

    private:
        void OnDeviceAdded(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformation^ args);
        void OnDeviceRemoved(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformationUpdate^ args);
        void OnDeviceUpdated(Windows::Devices::Enumeration::DeviceWatcher^ sender, Windows::Devices::Enumeration::DeviceInformationUpdate^ args);
        void OnDeviceEnumerationCompleted(Windows::Devices::Enumeration::DeviceWatcher^ sender, Platform::Object^ args);

        void WaitForEnumeration();

        Windows::Devices::Enumeration::DeviceWatcher^ mPortWatcher;
        std::vector<std::unique_ptr<WinRTMidiPortInfo>> mPortInfo;
        std::mutex mEnumerationMutex;
        std::condition_variable mSleepCondition;

        MidiPortChangedCallback mPortChangedCallback;
        WinRTMidiPortType mPortType;
        bool mPortEnumerationComplete;
        void* mContext;
    };


    class MidiPortWatcherWrapper
    {
    public:
        MidiPortWatcherWrapper(WinRTMidiPortWatcher ^ watcher)
            : mWatcher(watcher)
        {
        }

        WinRTMidiPortWatcher^ GetWatcher() {
            return mWatcher;
        }

    private:
        WinRTMidiPortWatcher^ mWatcher;
    };
};




