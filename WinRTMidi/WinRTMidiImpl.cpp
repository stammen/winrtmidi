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

    // wait for port to be created
    task.wait();

    // get results
    try
    {
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

WinRTMidiOutPort::WinRTMidiOutPort()
{
}

WinRTMidiOutPort::~WinRTMidiOutPort()
{
}

//Blocks until port is open
void WinRTMidiOutPort::OpenPort(Platform::String^ id)
{
    try
    {
        auto task = MidiOutPort::FromIdAsync(id);
        mMidiOutPort = task->GetResults();
    }
    catch (Platform::Exception^ ex)
    {

    }
}

void WinRTMidiOutPort::ClosePort(void)
{
    mMidiOutPort = nullptr;
}

