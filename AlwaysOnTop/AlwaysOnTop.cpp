// AlwaysOnTop.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <vector>
#include <algorithm>


void ShowInstructions()
{
    std::wcout << L"Usage:\rn"
        L"AlwaysOnTop.exe[window - name]\r\n"
        L"[window - name] should be either the\r\n"
        L"window or the name of the window class. " << std::endl;
}


std::vector<HWND> windowList;
std::vector<WCHAR> windowFound_className(1024);

BOOL CALLBACK WindowFound(HWND hWnd, LPARAM lParam)
{
    std::wstring* targetClassName = reinterpret_cast<std::wstring*>(lParam);
    size_t classNameLength = GetClassName(hWnd, windowFound_className.data(), windowFound_className.size());
    if (classNameLength > 0)
    {
        std::wstring foundClassName = std::wstring(windowFound_className.data(), classNameLength);        
        if (foundClassName == *targetClassName)
        {
            windowList.push_back(hWnd);
        }
    }
    return TRUE;
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
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);

    while (true)
    {
        windowHandle = NULL;
        while (windowHandle == NULL)
        {
            std::wcout << "Attempting to acquire windows with class name [" << windowName << L"]" << std::endl;
            windowHandle = FindWindow(windowName.c_str(), nullptr);
            if (windowHandle == nullptr)
            {
                std::wcout << "Window not found by class name. Attempting to find window with title " << windowName << L"]" << std::endl;
                windowHandle = FindWindow(nullptr, windowName.c_str());
                if (windowHandle != nullptr)
                {
                    windowList.clear();
                    windowList.push_back(windowHandle);
                }
            } 
            else
            {
                std::wcout << "Window found by class name. Checking for other top level windows with the same class name" << std::endl;
                std::shared_ptr<std::wstring> classNamePointer = std::make_shared<std::wstring>(windowName);
                windowList.clear();
                EnumWindows(WindowFound, reinterpret_cast<LPARAM>(classNamePointer.get()));
                std::wcout << "A total of ";
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                std::wcout << windowList.size();
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
                std::wcout << " windows were found in that class name." << std::endl;
            }
            if (windowHandle == nullptr)
            {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                std::wcout << "No window found. Trying again in 3.5 seconds." << std::endl;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
                Sleep(3500);
            }
        }


        std::for_each(windowList.begin(), windowList.end(), [](HWND hWnd)
            {
                std::wcout << L"Setting window 0x" << std::hex << hWnd << L"to top most window and foreground" << std::endl;
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);                
            }
        );
        

        std::wcout << L"Waiting 7.5 seconds" << std::endl;
        Sleep(7500);
        
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN| FOREGROUND_INTENSITY);
        std::wcout << "Wait expired. Looping." << std::endl;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
        
    } 
    
}

