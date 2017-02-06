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
#include "WinRTMidiimpl.h"
#include <ppltasks.h>
#include <robuffer.h> 

using namespace WinRT;
using namespace Windows::Devices::Midi;
using namespace Windows::Storage::Streams;
using namespace std::placeholders;
using namespace concurrency;
using namespace Microsoft::WRL;


/*****************************************************
    WinRTMidiInPort
*****************************************************/

WinRTMidi::WinRTMidi(MidiPortChangedCallback callback, void* context)
{
    mContext = context;
    mMidiInPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::In, callback, context);
    mMidiOutPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::Out, callback, context);
    mMidiInPortWatcherWrapper = std::make_shared<MidiPortWatcherWrapper>(mMidiInPortWatcher);
    mMidiOutPortWatcherWrapper = std::make_shared<MidiPortWatcherWrapper>(mMidiOutPortWatcher);
}

WinRTMidiErrorType WinRTMidi::Initialize()
{
    WinRTMidiErrorType result = mMidiInPortWatcher->Initialize();
    if (result == WINRT_NO_ERROR)
    {
        result = mMidiOutPortWatcher->Initialize();
    }

    return result;
}

WinRTMidiPortWatcher^ WinRTMidi::GetPortWatcher(WinRTMidiPortType type)
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

WinRTMidiPortWatcherPtr WinRTMidi::GetPortWatcherWrapper(WinRTMidiPortType type)
{
    switch (type)
    {
    case WinRTMidiPortType::In:
        return (WinRTMidiPortWatcherPtr)mMidiInPortWatcherWrapper.get();
        break;
    case WinRTMidiPortType::Out:
        return (WinRTMidiPortWatcherPtr)mMidiOutPortWatcherWrapper.get();
        break;
    }

    return nullptr;
}

Platform::String^ WinRTMidi::getPortId(WinRTMidiPortType type, unsigned int index)
{
    auto watcher = GetPortWatcher(type);
    if (watcher)
    {
        return watcher->GetPortId(index);
    }

    return nullptr;
}

WinRTMidiPort::WinRTMidiPort()
    : mErrorMessage("")
    , mError(WINRT_NO_ERROR)
    , mPortWrapper(nullptr)
{

}

WinRTMidiPort::~WinRTMidiPort()
{

}

void WinRTMidiPort::SetError(WinRTMidiErrorType error, const std::string& message)
{
    mError = error;
    mErrorMessage = message;
}

const std::string& WinRTMidiPort::GetErrorMessage()
{
    return mErrorMessage;
}

WinRTMidiErrorType WinRTMidiPort::GetError()
{
    return mError;
}

WinRTMidiInPort::WinRTMidiInPort(void* context)
    : mMessageReceivedCallback(nullptr)
    , mContext(context)
{
}

WinRTMidiInPort::~WinRTMidiInPort()
{
    ClosePort();
}

//Blocks until port is open
WinRTMidiErrorType WinRTMidiInPort::OpenPort(Platform::String^ id)
{
    WinRTMidiErrorType result = WINRT_NO_ERROR;
    mLastMessageTime = 0;
    mFirstMessage = true;
    auto task = create_task(MidiInPort::FromIdAsync(id));

    // get results
    try
    {
        // block until port is created
        mMidiInPort = task.get();
		if (mMidiInPort != nullptr)
		{
			mMidiInPort->MessageReceived += ref new Windows::Foundation::TypedEventHandler<MidiInPort ^, MidiMessageReceivedEventArgs ^>(this, &WinRTMidiInPort::OnMidiInMessageReceived);
		}
		else
		{
			// no exception but we didn't get a valid port
			result = WINRT_OPEN_PORT_ERROR;
		}
    }
    catch (Platform::Exception^ ex)
    {
        result = WINRT_OPEN_PORT_ERROR;
    }

    return result;
}

void WinRTMidiInPort::ClosePort(void) 
{
    if (mMidiInPort)
    {
        delete mMidiInPort;
    }
    mMidiInPort = nullptr;
}

void WinRTMidiInPort::OnMidiInMessageReceived(MidiInPort^ sender, MidiMessageReceivedEventArgs^ args)
{
    if (mMessageReceivedCallback)
    {
        if (mFirstMessage)
        {
            mFirstMessage = false;
            mLastMessageTime = args->Message->Timestamp.Duration;
        }

#ifdef WINRTMIDI_REPORT_DELTA_TIME
        double timestamp = (args->Message->Timestamp.Duration - mLastMessageTime) * .0001;
        mLastMessageTime = args->Message->Timestamp.Duration;
#else
        double timestamp = args->Message->Timestamp.Duration * .0001;
#endif

        auto buffer = args->Message->RawData;

        // Obtain IBufferByteAccess 
        ComPtr<IBufferByteAccess> pBufferByteAccess;
        ComPtr<IUnknown> pBuffer((IUnknown*)buffer);
        pBuffer.As(&pBufferByteAccess);

        // Get pointer to iBuffer bytes 
        byte* pData;
        pBufferByteAccess->Buffer(&pData);
        mMessageReceivedCallback(mPortWrapper, timestamp, pData, buffer->Length, mContext);
    }
}


/*****************************************************
    WinRTMidiOutPort
*****************************************************/

#define kOutputMidiBufferSize 128
WinRTMidiOutPort::WinRTMidiOutPort(void* context)
    : mContext(context)
{
    mBuffer = ref new Buffer(kOutputMidiBufferSize);
    mBufferData = getIBufferDataPtr(mBuffer);
}

WinRTMidiOutPort::~WinRTMidiOutPort()
{
    ClosePort();
}

//Blocks until port is open
WinRTMidiErrorType WinRTMidiOutPort::OpenPort(Platform::String^ id)
{
    WinRTMidiErrorType result = WINRT_NO_ERROR;
    auto task = create_task(MidiOutPort::FromIdAsync(id));

    try
    {
        // blocks until port is created
        mMidiOutPort = task.get();
		if (mMidiOutPort == nullptr)
		{
			// no exception but we didn't get a valid port
			result = WINRT_OPEN_PORT_ERROR;
		}
    }
    catch (Platform::Exception^ ex)
    {
        result = WINRT_OPEN_PORT_ERROR;
    }

    return result;
}

void WinRTMidiOutPort::ClosePort(void)
{
    if (mMidiOutPort)
    {
        delete mMidiOutPort;
    }
    mMidiOutPort = nullptr;
}

byte* WinRTMidiOutPort::getIBufferDataPtr(IBuffer^ buffer)
{
    // Obtain IBufferByteAccess 
    ComPtr<IBufferByteAccess> pBufferByteAccess;
    ComPtr<IUnknown> pBuffer((IUnknown*)buffer);
    pBuffer.As(&pBufferByteAccess);

    // Get pointer to iBuffer bytes 
    byte* pData;
    pBufferByteAccess->Buffer(&pData);
    return pData;
}

byte* getIBufferDataPtr2(IBuffer^ buffer)
{
    // Query the IBufferByteAccess interface.
    ComPtr<IBufferByteAccess> bufferByteAccess;
    reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

    // Retrieve the buffer data.
    byte* pData = nullptr;
    bufferByteAccess->Buffer(&pData);
    return pData;
}

void WinRTMidiOutPort::Send(const unsigned char* message, unsigned int nBytes)
{
    // check if preallocated IBuffer is big enough and reallocate if needed
    if (nBytes > mBuffer->Capacity)
    {
        mBuffer = ref new Buffer(nBytes);
        mBufferData = getIBufferDataPtr(mBuffer);
    }

    memcpy_s(mBufferData, nBytes, message, nBytes);
    mBuffer->Length = nBytes;
    mMidiOutPort->SendBuffer(mBuffer);
}


