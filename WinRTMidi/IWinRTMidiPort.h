// MathLibrary.h - Contains declaration of Function class
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "IWinRTMidiPortWatcher.h"

namespace WinRT
{
    class IWinRTMidiInPort;

    // C++ Midi In callback
    typedef std::function<void(const IWinRTMidiInPort* port, double timestamp, std::vector<unsigned char>* message)> IMidiInCallbackType;

	class IWinRTMidiInPort {
	public:
        virtual void OpenPort(unsigned int index) = 0;
        virtual void ClosePort() = 0;
        virtual void RemoveCallback() = 0;
        virtual void SetCallback(const IMidiInCallbackType& callback) = 0;
        virtual void Destroy() = 0;
	};

    typedef IWinRTMidiInPort* (__cdecl *IWinRTMidiInPortFactoryFunc)();
}


