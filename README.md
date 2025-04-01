# Setting another Application to Be Always On Top

When creating an application, if we want our own application to be the topmost window, many UI APIs have a call or setting that we can alter to ensure that is how our window displays. For a client, we were asked to make a third-party application that always appeared on top of other windows. Contacting the application vendor, we found that there was no way to do this within the range of settings that we have access to. Nor was there likely to be a method available on our timelines. This isn't a serious problem though; we can use some Win32 APIs to alter the window settings ourselves.

This is something that is only to be done as a last resort. Manipulating the internal settings of another application can come with risks. When doing something like this, it should be done with a significant amount of testing. To accomplish this task, we only need to get a handle of the window that we wish to affect and call SetWindowPos with the argument HWND_TOPMOST. That's the easy part. The less obvious part is how does get their hands on the handle of another window. The FindWindows API can be used to get the handle of a Window based either on the window title or the window class name. For the Notepad application on Windows 10, the name of the window class is simply Notepad. We could also get access to a Notepad window if we use the text string that shows up in its title bar. For flexibility, put this functionality into an application or have it use FindWindow up to 2 times so that I can attempt to find the window by the class name or the title. The value to be used here is passed as a command line parameter. In C++, we end up with an application that has the following source code. The application calls these Windows API in a loop. This allows it to have an effect if the target application hasn't presented a window or if the application closes and reopens.

```
// AlwaysOnTop.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <Windows.h>

void ShowInstructions()
{
    std::wcout << L"Usage:\rn"
        L"AlwaysOnTop.exe[window - name]\r\n"
        L"[window - name] should be either the\r\n"
        L"window or the name of the window class. " << std::endl;
}



int wmain(int argc, wchar_t** argv)
{
    HWND windowHandle = nullptr;
    std::wstring windowName ;
    if (argc < 2) {
        ShowInstructions();
        return -1;
    }

    windowName = std::wstring(argv[1]);

    while (true)
    {
        windowHandle = NULL;
        while (windowHandle == NULL)
        {
            windowHandle = FindWindow(windowName.c_str(), nullptr);
            if (windowHandle == nullptr)
            {
                windowHandle = FindWindow(nullptr, windowName.c_str());
            }
            if (windowHandle == nullptr)
            {
                Sleep(3500);
            }
        }
        std::wcout << "Window handle found for " <<windowName << " }. \r\nSetting to top most window";
        while (true) {

            SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetForegroundWindow(windowHandle);
            Sleep(7500);
        }
    }
}

```


I've found that native executables tend to set off alarms for a security application that we use. The security application isn't as sensitive to .Net executables. I have the source code in .Net also. It calls the same Windows APIs in the same order.



```
using System.Runtime.InteropServices;

namespace AlwaysOnTop.Net
{
    internal class Program
    {
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);


        [DllImport("user32.dll")]
        static extern IntPtr SetFocus(IntPtr hWnd);

        [DllImport("User32.dll")]
        static extern int SetForegroundWindow(IntPtr hWnd);

        // Constants for nCmdShow
        const int SW_HIDE = 0;
        const int SW_SHOW = 5;
        const uint SWP_NOSIZE = 0x0001;
        const uint SWP_NOZORDER = 0x0004;
        const uint SWP_NOMOVE = 0x002;
        const int HWND_TOPMOST = -1;
        static readonly IntPtr HWND_TOP = IntPtr.Zero;



        static void ShowInstructions()
        {
            Console.WriteLine(
@"Usage:

AlwaysOnTop.Net.exe [window-name]

[window-name] should be either the 
window name or window class.
"
            );
        }

        static void Main(string[] args)
        {
            if(args.Length < 1)
            {
                ShowInstructions();
                return;
            }            
            string windowName = args[0];



            IntPtr windowHandle = IntPtr.Zero;

            while(true)
            {
                while (windowHandle == IntPtr.Zero)
                {
                    windowHandle = FindWindow(windowName, null);
                    if (windowHandle == IntPtr.Zero)
                    {
                        windowHandle = FindWindow(null, windowName);
                    }
                    if(windowHandle == null)
                    {
                        Thread.Sleep(3500);
                    }
                }
                Console.WriteLine($"Window handle found for {windowName}. \r\nSetting to top most window");
                while(true){

                    SetWindowPos(windowHandle,  HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    SetForegroundWindow(windowHandle);
                    Thread.Sleep(7500);
                }
            }
        }
    }
}
```

For applications where the class of the top-most window is not known, what do we do? I threw together one other application to get that information. With this other application, I would start the application whose information I want to acquire, then run my command line utility, saving the CSV text that it outputs. The name of the application is ListAllWindows.exe (descriptive!). The Win32 function EnumWindows enumerates all top-level windows and passes a handle to them to a callback function. In the callback, I save the window handle. With a window handle, I can call GetWindowClass() function to get the class name as a WCHAR array. This gets packaged as a std::wstring (those are safer).



```
// ListAllWindows.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <tlhelp32.h>
#include <psapi.h>
#include <iomanip>
#include <sstream>


struct HANDLECloser
{
    void operator()(HANDLE handle) const
    {
        if (handle != INVALID_HANDLE_VALUE && handle != 0)
        {
            CloseHandle(handle);
        }
    }
};


struct WindowInformation {
    HWND handle;
    std::wstring className;
    std::wstring processName;
};

std::vector<WindowInformation> windowList;

BOOL CALLBACK WindowFound(HWND hWnd, LPARAM lParam)
{
    windowList.push_back(WindowInformation{hWnd, L"",L""});
    return TRUE;
}

int wmain()
{    
    EnumWindows(WindowFound, 0);
    std::wcout << "Number of top level Windows found :" << windowList.size() << std::endl << std::endl;

    std::for_each(windowList.begin(), windowList.end(), [](WindowInformation& info) 
    {
            std::vector<WCHAR> buffer(1024);
            size_t stringLength;
            DWORD processID = 0;
            if (SUCCEEDED(stringLength=GetClassName(info.handle, buffer.data(), buffer.size())))
            {
                info.className = std::wstring(buffer.data(), stringLength);
            }

            DWORD threadID = GetWindowThreadProcessId(info.handle, &processID);
            if (threadID != 0)
            {
                auto processHandleTemp = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processID);
                if (processHandleTemp != 0)
                {

                    auto processHandle = std::unique_ptr<void, HANDLECloser>(processHandleTemp);


                    std::vector<WCHAR> processName(1024);
                    auto processNameLength = GetModuleFileNameEx(processHandle.get(), NULL, processName.data(), processName.size());
                    info.processName = std::wstring(processName.data(), processNameLength);
                }
                else
                {
                    auto lastError = GetLastError();
                    std::wcerr << "Get Process failed " << lastError << std::endl;
                    info.processName = L"unknown";
                }                
            }
    });


    std::wcout <<  "Window Handle, Class Name, Process Executable" << std::endl;
    std::for_each(windowList.begin(), windowList.end(), [](WindowInformation& info)
        {
            std::wcout << info.handle << L", " << info.className << L", " << info.processName << std::endl;
        }
    );

    return 0;
}
```

Sample output from this program follows. I've not provided the full output since that would be more than 800 windows.

```

Sample output from this program follows. I've not provided the full output since that would be more than 800 windows.

Number of top level Windows found :868
0000000000030072, .NET-BroadcastEventWindow.21af1a5.0, C:\Program Files\WindowsApps\Microsoft.YourPhone_1.25022.70.0_x64__8wekyb3d8bbwe\PhoneExperienceHost.exe
00000000000716DA, PersonalizationThemeChangeListener, C:\Windows\ImmersiveControlPanel\SystemSettings.exe
00000000008514E4, Windows.UI.Core.CoreWindow, C:\Windows\ImmersiveControlPanel\SystemSettings.exe
0000000000950E12, WorkerW, C:\Windows\ImmersiveControlPanel\SystemSettings.exe
00000000003E16C2, ApplicationFrameWindow, C:\Windows\System32\ApplicationFrameHost.exe
00000000001B1660, ComboLBox, C:\Windows\System32\mstsc.exe
000000000065157E, TscShellContainerClass, C:\Windows\System32\mstsc.exe
00000000006014C4, WorkerW, C:\Windows\explorer.exe
00000000001E0D7E, WindowsForms10.Window.20808.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
0000000000190E8A, WindowsForms10.tooltips_class32.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
00000000000D10A0, WindowsForms10.Window.0.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
0000000000061732, WindowsForms10.Window.20808.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
00000000000C1778, WindowsForms10.tooltips_class32.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
000000000027125C, WindowsForms10.Window.20808.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
00000000002516D2, WindowsForms10.tooltips_class32.app.0.224edbf_r3_ad1, C:\Program Files\paint.net\paintdotnet.exe
```


In the second column of this CSV, the names of the Window classes show along with the path to the executable that they belong to. Oftentimes, an application may have more than one top-level window. Figuring out which don't to use comes down to experimentation. Be prepared to start the program several times. 

