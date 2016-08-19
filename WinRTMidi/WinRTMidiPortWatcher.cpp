#include "WinRTMidiPortWatcher.h"
#include <algorithm>
#include <collection.h>
#include <cvt/wstring>
#include <codecvt>

using namespace Windows::Devices::Midi;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;

namespace WinRT
{
    std::string PlatformStringToString(Platform::String^ s)
    {
        std::wstring w(s->Data());
        std::string result(w.begin(), w.end());
        return result;
    }

    std::string PlatformStringToString2(Platform::String^ s)
    {
        stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        std::string stringUtf8 = convert.to_bytes(s->Data());
        return stringUtf8;
    }

    WinRTMidiPortWatcher::WinRTMidiPortWatcher(WinRTMidiPortType type, MidiPortChangedCallback callback)
        : mPortEnumerationComplete(false)
        , mPortChangedCallback(callback)
        , mPortType(type)
    {
        switch (type)
        {
        case WinRTMidiPortType::In:
            mPortWatcher = DeviceInformation::CreateWatcher(MidiInPort::GetDeviceSelector());
            break;
        case WinRTMidiPortType::Out:
            mPortWatcher = DeviceInformation::CreateWatcher(MidiOutPort::GetDeviceSelector());
            break;
        }

        // wire up event handlers
        mPortWatcher->Added += ref new TypedEventHandler<DeviceWatcher ^, DeviceInformation ^>(this, &WinRTMidiPortWatcher::OnDeviceAdded);
        mPortWatcher->Removed += ref new TypedEventHandler<DeviceWatcher ^, DeviceInformationUpdate ^>(this, &WinRTMidiPortWatcher::OnDeviceRemoved);
        mPortWatcher->Updated += ref new TypedEventHandler<DeviceWatcher ^, DeviceInformationUpdate ^>(this, &WinRTMidiPortWatcher::OnDeviceUpdated);
        mPortWatcher->EnumerationCompleted += ref new Windows::Foundation::TypedEventHandler<DeviceWatcher ^, Platform::Object ^>(this, &WinRTMidiPortWatcher::OnDeviceEnumerationCompleted);

        // start enumeration
        mPortEnumerationComplete = false;
        mPortWatcher->Start();
    }

    // blocks if port enumeration is not complete
    void WinRTMidiPortWatcher::CheckForEnumeration()
    {
        while (!mPortEnumerationComplete)
        {
            std::unique_lock<std::mutex> lock(mEnumerationMutex);
            mSleepCondition.wait(lock);
        }
    }

   const std::string& WinRTMidiPortWatcher::GetPortName(unsigned int portNumber)
    {
        CheckForEnumeration();
        return mPortInfo[portNumber].get()->mName;
    }

    Platform::String^ WinRTMidiPortWatcher::GetPortId(unsigned int portNumber)
    {
        CheckForEnumeration();
        if (portNumber >= mPortInfo.size())
        {
            return "";
        }
        else
        {
            return mPortInfo[portNumber].get()->mID;
        }
    }

    unsigned int WinRTMidiPortWatcher::GetPortCount()
    {
        CheckForEnumeration();
        return static_cast<int>(mPortInfo.size());
    }

    void WinRTMidiPortWatcher::OnDeviceAdded(DeviceWatcher^ sender, DeviceInformation^ args)
    {
        mPortInfo.emplace_back(new WinRTMidiPortInfo(PlatformStringToString(args->Name), args->Id));
        if (mPortEnumerationComplete)
        {
            OnMidiPortUpdated(WinRTMidiPortUpdateType::PortAdded);
        }
    }

    void WinRTMidiPortWatcher::OnDeviceRemoved(DeviceWatcher^ sender, DeviceInformationUpdate^ args)
    {
        for (unsigned int i = 0; i < mPortInfo.size(); i++)
        {
            if (mPortInfo[i]->mID == args->Id)
            {
                std::swap(mPortInfo[i], mPortInfo.back());
                mPortInfo.pop_back();
                break;
            }
        }

        if (mPortEnumerationComplete)
        {
            OnMidiPortUpdated(WinRTMidiPortUpdateType::PortRemoved);
        }
    }

    void WinRTMidiPortWatcher::OnDeviceUpdated(DeviceWatcher^ sender, DeviceInformationUpdate^ args)
    {
        // nothing to do here for now; MIDI devices don't really update at this point
    }

    void WinRTMidiPortWatcher::OnDeviceEnumerationCompleted(DeviceWatcher^ sender, Platform::Object^ args)
    {
        std::unique_lock<std::mutex> locker(mEnumerationMutex);
        mPortEnumerationComplete = true;
        mSleepCondition.notify_one();
        OnMidiPortUpdated(WinRTMidiPortUpdateType::EnumerationComplete);
    }

    void WinRTMidiPortWatcher::OnMidiPortUpdated(WinRTMidiPortUpdateType update)
    {
        MidiPortWatcherWrapper wrapper(this);

        if (mPortChangedCallback != nullptr)
        {
            mPortChangedCallback(&wrapper, update);
        }

        // ...then fire the event:
        mMidiPortUpdateEventHander(this, update);
    }


# if 0
    extern "C" __declspec(dllexport) const IWinRTMidiPortWatcher* __cdecl GetIMidiPortWatcher(WinRTMidiPortType portType)
    {
        if (portType == WinRTMidiPortType::In)
        {
            return WinRTMidiInPortWatcher::getInstance()->GetIMidiPortWatcher();
        }
        else if (portType == WinRTMidiPortType::Out)
        {
            return WinRTMidiOutPortWatcher::getInstance()->GetIMidiPortWatcher();
        }
        return nullptr;
    }
#endif
};

