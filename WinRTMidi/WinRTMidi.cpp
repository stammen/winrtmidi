#include "WinRTMidi.h"
#include "WinRTMidiportWatcher.h"

using namespace WinRT;
using namespace Windows::Devices::Midi;
using namespace Windows::Storage::Streams;
using namespace std::placeholders;

/*****************************************************
    MidiInPortImpl
*****************************************************/

MidiInPortImpl::MidiInPortImpl()
    : mCallback(nullptr)
{
    mPort = ref new WinRTMidiInPort;
    mPort->SetMidiInCallback(std::bind(&MidiInPortImpl::MidiInCallback, this, _1, _2));
}

MidiInPortImpl::~MidiInPortImpl()
{

}

void MidiInPortImpl::OpenPort(unsigned int index)
{
    mPort->OpenPort(index);
}

void MidiInPortImpl::ClosePort()
{
    mPort->ClosePort();
}

void MidiInPortImpl::MidiInCallback(double timestamp, std::vector<unsigned char>* message)
{
    if (mCallback)
    {
        mCallback(this, timestamp, message);
    }
}

void MidiInPortImpl::RemoveCallback()
{
    mCallback = nullptr;
}

void MidiInPortImpl::SetCallback(const IMidiInCallbackType& callback)
{
    mCallback = callback;
}

void MidiInPortImpl::Destroy()
{
    delete this;
}


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
void WinRTMidiInPort::OpenPort(int index)
{
    auto id = WinRTMidiInPortWatcher::getInstance()->GetPortId(index);
    OpenPort(id);
}


//Blocks until port is open
void WinRTMidiInPort::OpenPort(Platform::String^ id)
{
    try
    {
        mLastMessageTime = 0;
        mFirstMessage = true;
        auto task = MidiInPort::FromIdAsync(id);
        mMidiInPort = task->GetResults();
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
        DataReader^ reader = DataReader::FromBuffer(buffer);
        mMidiMessage.resize(buffer->Length);
        reader->ReadBytes(Platform::ArrayReference<unsigned char>(&mMidiMessage[0], buffer->Length));

        mMessageReceivedCallback((double)timestamp, &mMidiMessage);
        mMidiMessage.clear();
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

