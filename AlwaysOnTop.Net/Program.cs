using System;
using System.Runtime.InteropServices;
using System.Text;

namespace AlwaysOnTop.Net
{

    
    public delegate bool CallBack(int hwnd, [MarshalAs(UnmanagedType.LPWStr)] string lParam);

    internal class Program
    {
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);


        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern int GetClassName(IntPtr hWnd, StringBuilder lpClassName, int nMaxCount);


        [DllImport("user32.dll")]
        static extern IntPtr SetFocus(IntPtr hWnd);

        [DllImport("User32.dll")]
        static extern int SetForegroundWindow(IntPtr hWnd);
        [DllImport("user32", CharSet=CharSet.Unicode)]
        public static extern int EnumWindows(CallBack x, string y);

        // Constants for nCmdShow
        const int SW_HIDE = 0;
        const int SW_SHOW = 5;
        const uint SWP_NOSIZE = 0x0001;
        const uint SWP_NOZORDER = 0x0004;
        const uint SWP_NOMOVE = 0x002;
        const int HWND_TOPMOST = -1;
        static readonly IntPtr HWND_TOP = IntPtr.Zero;

        static List<IntPtr> WindowList = new List<IntPtr>();

        public static bool WindowFound(int hwnd, string lParam)
        {

            StringBuilder sb = new StringBuilder(1024);
            GetClassName(hwnd, sb, sb.Capacity);
            if(sb.ToString() == lParam)
            {
                WindowList.Add(hwnd);
            }
            return true;
        }

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
                windowHandle = IntPtr.Zero;
                WindowList.Clear();
                Console.WriteLine($"Attempting to acquire windows with class name [{windowName}]");
                while (windowHandle == IntPtr.Zero)                
                {
                    Console.WriteLine($"Window not found by class name. Attempting to find window with title {windowName}]");
                    windowHandle = FindWindow(windowName, null);
                    if (windowHandle == IntPtr.Zero)
                    {
                        windowHandle = FindWindow(null, windowName);
                        if(windowHandle != IntPtr.Zero)
                        {
                            WindowList.Add(windowHandle);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Window found by class name. Checking for other top level windows with the same class name");
                        WindowList.Clear();
                        EnumWindows(WindowFound, windowName);
                        Console.Write("A total of ");
                        Console.ForegroundColor = ConsoleColor.Green;
                        Console.Write(WindowList.Count);
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine(" windows were found in that class name.");
                    }

                    if(WindowList.Count == 0)
                    {
                        Console.ForegroundColor= ConsoleColor.Red;
                        Console.WriteLine("No window found. Trying again in 3.5 seconds.");
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Thread.Sleep(3500);
                    }
                }

                WindowList.ForEach(window =>
                {
                    SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    SetForegroundWindow(window);
                });
                Console.WriteLine($"Waiting 7.5 seconds");                   
                Thread.Sleep(7500);
                Console.ForegroundColor = ConsoleColor.Green ;
                Console.WriteLine("Wait expired. Looping.");
                Console.ForegroundColor = ConsoleColor.Gray;


            }
        }
    }
}
