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

#include "WinRTMidiPortWatcher.h"
#include <algorithm>
#include <memory>
#include <collection.h>
#include <cvt/wstring>
#include <codecvt>
#include <ppltasks.h>

using namespace Windows::Devices::Midi;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace concurrency;

namespace WinRT
{
    std::string PlatformStringToString(Platform::String^ s)
    {
        int bufferSize = WideCharToMultiByte(CP_UTF8, 0, s->Data(), -1, nullptr, 0, NULL, NULL);
        auto utf8 = std::make_unique<char[]>(bufferSize);
        if (0 == WideCharToMultiByte(CP_UTF8, 0, s->Data(), -1, utf8.get(), bufferSize, NULL, NULL))
            throw std::exception("Can't convert string to UTF8");

        return std::string(utf8.get());
    }

    std::string PlatformStringToString2(Platform::String^ s)
    {
        stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        std::string stringUtf8 = convert.to_bytes(s->Data());
        return stringUtf8;
    }

    WinRTMidiPortWatcher::WinRTMidiPortWatcher(WinRTMidiPortType type, MidiPortChangedCallback callback, void* context)
        : mPortEnumerationComplete(false)
        , mPortChangedCallback(callback)
        , mPortType(type)
        , mContext(context)
    {
 
    }

    WinRTMidiErrorType WinRTMidiPortWatcher::Initialize()
    {
        auto task = create_task(create_async([this]
        {
            switch (mPortType)
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

            // wait until port enumeration is complete
            WaitForEnumeration();

            // report enumerated ports
            OnMidiPortUpdated(WinRTMidiPortUpdateType::EnumerationComplete);

        }));

        try
        {
            // wait for MIDI port enumeration to complete
            task.get(); // will throw any exceptions from above task
            return WINRT_NO_ERROR;
        }
        catch (Platform::Exception^ ex)
        {
            return WINRT_PORTWATCHER_INITIALIZATION_ERROR;
        }
    }

    // blocks if port enumeration is not complete
    void WinRTMidiPortWatcher::WaitForEnumeration()
    {
        while (!mPortEnumerationComplete)
        {
            std::unique_lock<std::mutex> lock(mEnumerationMutex);
            mSleepCondition.wait(lock);
        }
    }

    const std::string& WinRTMidiPortWatcher::GetPortName(unsigned int portNumber)
    {
        return mPortInfo[portNumber].get()->mName;
    }

    Platform::String^ WinRTMidiPortWatcher::GetPortId(unsigned int portNumber)
    {
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
    }

    void WinRTMidiPortWatcher::OnMidiPortUpdated(WinRTMidiPortUpdateType update)
    {
        MidiPortWatcherWrapper wrapper(this);

        if (mPortChangedCallback != nullptr)
        {
            mPortChangedCallback(&wrapper, update, mContext);
        }

        mMidiPortUpdateEventHander(this, update);
    }
}


