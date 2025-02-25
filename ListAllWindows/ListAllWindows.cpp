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
