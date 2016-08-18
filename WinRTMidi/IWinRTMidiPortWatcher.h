// MathLibrary.h - Contains declaration of Function class
#pragma once

#include <functional>
#include <string>


namespace WinRT
{
    class IWinRTMidiPortWatcher;

    public enum class WinRTMidiPortType : int { In, Out };
    public enum class WinRTMidiPortUpdateType : int { PortAdded, PortRemoved, EnumerationComplete };

    // C++ Midi port changed callback
    typedef std::function<void(const IWinRTMidiPortWatcher* watcher, WinRTMidiPortUpdateType update)> IMidiPortChangedCallbackType;

	class IWinRTMidiPortWatcher {
	public:
        virtual unsigned int GetPortCount() const = 0;
        virtual std::string GetPortName(unsigned int portNumber) const = 0;
        virtual WinRTMidiPortType GetPortType() const = 0;
	};

    typedef void(__cdecl *SetIMidiPortChangedCallbackFunc)(const IMidiPortChangedCallbackType& callback);
    typedef IWinRTMidiPortWatcher*(__cdecl *GetIMidiPortWatcherFunc)(WinRTMidiPortType portType);
}


