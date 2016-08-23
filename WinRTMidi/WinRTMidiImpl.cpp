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

WinRTMidi::WinRTMidi(MidiPortChangedCallback callback)
{
    mMidiInPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::In, callback);
    mMidiOutPortWatcher = ref new WinRTMidiPortWatcher(WinRTMidiPortType::Out, callback);
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
    , mError(S_OK)
{

}

WinRTMidiPort::~WinRTMidiPort()
{

}

void WinRTMidiPort::SetError(HRESULT error, const std::string&)
{

}

const std::string& WinRTMidiPort::GetErrorMessage()
{
    return mErrorMessage;
}
HRESULT WinRTMidiPort::GetError()
{
    return mError;
}

WinRTMidiInPort::WinRTMidiInPort()
    : mMessageReceivedCallback(nullptr)
{
}

WinRTMidiInPort::~WinRTMidiInPort()
{

}

//Blocks until port is open
void WinRTMidiInPort::OpenPort(Platform::String^ id)
{
    mLastMessageTime = 0;
    mFirstMessage = true;
    auto task = create_task(MidiInPort::FromIdAsync(id));

    // get results
    try
    {
        // block until port is created
        mMidiInPort = task.get();
        mMidiInPort->MessageReceived += ref new Windows::Foundation::TypedEventHandler<MidiInPort ^, MidiMessageReceivedEventArgs ^>(this, &WinRTMidiInPort::OnMidiInMessageReceived);
    }
    catch (Platform::Exception^ ex)
    {

    }
}

void WinRTMidiInPort::ClosePort(void) 
{
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

        double timestamp = (args->Message->Timestamp.Duration - mLastMessageTime) * .0001;
        mLastMessageTime = args->Message->Timestamp.Duration;

        auto buffer = args->Message->RawData;

        // Obtain IBufferByteAccess 
        ComPtr<IBufferByteAccess> pBufferByteAccess;
        ComPtr<IUnknown> pBuffer((IUnknown*)buffer);
        pBuffer.As(&pBufferByteAccess);

        // Get pointer to iBuffer bytes 
        byte* pData;
        pBufferByteAccess->Buffer(&pData);
        mMessageReceivedCallback((WinRTMidiInPortPtr) this, timestamp, pData, buffer->Length);
    }
}


/*****************************************************
    WinRTMidiOutPort
*****************************************************/

#define kOutputMidiBufferSize 128
WinRTMidiOutPort::WinRTMidiOutPort()
{
    mBuffer = ref new Buffer(kOutputMidiBufferSize);
    mBufferData = getIBufferDataPtr(mBuffer);
}

WinRTMidiOutPort::~WinRTMidiOutPort()
{
}

//Blocks until port is open
void WinRTMidiOutPort::OpenPort(Platform::String^ id)
{
    auto task = create_task(MidiOutPort::FromIdAsync(id));

    try
    {
        // blocks until port is created
        mMidiOutPort = task.get();
    }
    catch (Platform::Exception^ ex)
    {
        throw(ex);
    }
}

void WinRTMidiOutPort::ClosePort(void)
{
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


