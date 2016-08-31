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

#include "WindowsVersionHelper.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>

// needed to use GetFileVersionInfo functions 
#pragma comment(lib, "version.lib")

namespace WinRT
{
    /**********************************************************************************
        Windows 8.1 and higher make it difficult to detect Windows OS version.
        Without an App Manifest indicating Windows 10 support, the Win32
        Version functions will return 6.2.0.0 for Windows 8.1 and above.
        This method requires adding an app manifest to your application
        to ensure correct Windows 10 version numbers.

        See https://github.com/stammen/winrtmidi/blob/master/README.md for info on how
        to add an application manifest to your application.
    **********************************************************************************/
    bool windows10orGreaterWithManifest()
    {
        OSVERSIONINFOEX  osvi;
        DWORDLONG dwlConditionMask = 0;
        int op = VER_GREATER_EQUAL;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.dwMajorVersion = 10;
        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);

        BOOL result = VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask);
        return result ? true : false;
    }

    /**********************************************************************************
    Windows 8.1 and higher make it difficult to access Windows OS version.
        Without an App Manifest indicating Windows 10 support, the Win32
        Version functions will return 6.2.0.0 for Windows 8.1 and above.
        This function checks OS version by getting version of kernel32.dll.
        This method does not require adding an app manifest to ensure correct
        Windows 10 version numbers.
    **********************************************************************************/
    bool windows10orGreater()
    {
        static const wchar_t kernel32[] = L"\\kernel32.dll";
        wchar_t path[MAX_PATH];

        unsigned int n = GetSystemDirectory(path, MAX_PATH);
        memcpy_s(path + n, MAX_PATH, kernel32, sizeof(kernel32));

        unsigned int size = GetFileVersionInfoSize(path, NULL);
        if (size == 0)
        {
            return false;
        }

        std::vector<char> verionInfo;
        verionInfo.resize(size);
        BOOL result = GetFileVersionInfo(path, 0, size, verionInfo.data());
        if (!result || GetLastError() != S_OK)
        {
            return false;
        }

        VS_FIXEDFILEINFO *vinfo;
        result = VerQueryValue(verionInfo.data(), L"\\", (LPVOID *)&vinfo, &size);
        if (!result || size < sizeof(VS_FIXEDFILEINFO))
        {
            return false;
        }

        return HIWORD(vinfo->dwProductVersionMS) >= 10;
    }
};