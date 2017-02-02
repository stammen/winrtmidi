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

#include "stdafx.h"
#include "WinRTMidi.h"
#include "MidiClient.h"
#include <iostream>
#include <conio.h>
#include <memory>

#define USING_APP_MANIFEST
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;
using namespace WinRT;

std::unique_ptr<MidiClient> gMidiClient;

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
        if (gMidiClient)
        {
            gMidiClient.reset();
            gMidiClient = nullptr;
        }
        return TRUE;

    case CTRL_BREAK_EVENT:
        return FALSE;

    default:
        return FALSE;
    }
}

int main()
{
    // add a handler to clean up DnssdClient for various console exit scenarios
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    gMidiClient = std::unique_ptr<MidiClient>(new MidiClient());
    WinRTMidiErrorType result = gMidiClient->initialize();

    if (result != WINRT_NO_ERROR)
    {
        cout << "Error: " << result << " initializing winrtmidi" << endl;
    }

    // process midi until user presses key on keyboard
    cout << "Press any key to exit..." << endl;
    char c = _getch();

    gMidiClient.reset();
    gMidiClient = nullptr;

    return 0;
}