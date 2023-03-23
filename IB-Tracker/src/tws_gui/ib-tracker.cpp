// Defines the entry point for the test application.
//

//#include "pch.h"
#include "framework.h"

#include "CWindow.h"

#include "TwsWrapper.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <future>

#include "ib-tracker.h"
#include "NavPanel.h"


const int IDC_CONNECT = 1000;
const int IDC_DISCONNECT = 1001;
const int IDC_MESSAGE = 1002;


TwsClient client;

bool tws_isConnected();


void BindStdHandlesToConsole()
{
    //TODO: Add Error checking.

    // Redirect the CRT standard input, output, and error handles to the console
    std::ignore = freopen("CONIN$", "r", stdin);
    std::ignore = freopen("CONOUT$", "w", stderr);
    std::ignore = freopen("CONOUT$", "w", stdout);

    // Note that there is no CONERR$ file
    HANDLE hStdout = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hStdin = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStdout);
    SetStdHandle(STD_INPUT_HANDLE, hStdin);

    //Clear the error state for each of the C++ standard stream objects. 
    std::wclog.clear();
    std::clog.clear();
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}


//
// Thread function
//

std::promise<void> signal_exit; //create promise object
std::future<void> future;
std::thread my_thread;


void threadFunction(std::future<void> future) {
    std::cout << "Starting the thread" << std::endl;
    while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        std::cout << "Executing the thread....." << std::endl;
        if (tws_isConnected()) {
            client.waitForSignal();
            client.processMsgs();
        } else {
            break;
        }

        std::chrono::milliseconds(500); //wait for 500 milliseconds
    }
    std::cout << "Thread Terminated" << std::endl;
}


void StartMonitorThread()
{
    future = signal_exit.get_future();//create future objects
    my_thread = std::thread(&threadFunction, std::move(future)); //start thread, and move future
}

void EndMonitorThread()
{
    std::cout << "Threads will be stopped soon...." << std::endl;
    signal_exit.set_value(); //set value into promise
    my_thread.join(); //join the thread with the main thread
    std::cout << "Doing task in main function" << std::endl;
}


bool tws_connect()
{
    if (tws_isConnected()) return false;

    const char* host = "";
    int port = 7496;   // 7497 is paper trading account
    const char* connectOptions = "";
    int clientId = 0;

    if (connectOptions) {
        client.setConnectOptions(connectOptions);
    }

    bool res = client.connect(host, port, clientId);
    if (res) {
        // Start thread that will start messaging polling
        // and poll if TWS remains connected.
        
        client.processMessages();

        StartMonitorThread();
    }

    return res;
}


bool tws_disconnect()
{
    if (client.isConnected() == false) return true;

    EndMonitorThread();

    client.disconnect();
    bool res = (tws_isConnected(), false, true);
    
    return res;
}


bool tws_isConnected()
{
    bool res = false;
    if ((client.isSocketOK()) && (client.isConnected())) 
        res = true;
    return res;
}


//' ========================================================================================
//' Position all child windows. Called manually and/or by WM_SIZE
//' ========================================================================================
LRESULT Main_PositionWindows(HWND hWnd)
{
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    HWND hWndNavPanel = GetDlgItem(hWnd, IDC_FRMNAVPANEL);
   
    int nNavPanelWidth = AfxGetWindowWidth(hWndNavPanel);
    
    dp(hWndNavPanel);

    SetWindowPos(hWndNavPanel, 0,
        0, 0, nNavPanelWidth, rcClient.bottom,
        SWP_NOZORDER | SWP_SHOWWINDOW);

    return 0;
}

//' ========================================================================================
//' Windows callback function.
//' ========================================================================================
LRESULT CALLBACK Main_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_SIZE:
        // Position all of the child windows
        if (wParam != SIZE_MINIMIZED)
            Main_PositionWindows(hWnd);
    break;


    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        
        case IDC_CONNECT:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
                bool res = tws_connect();
                printf("Connect: %ld\n", res);
                printf("isConnected: %ld\n", tws_isConnected());
                return 0;
            }
            break;

        case IDC_DISCONNECT:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
                bool res = tws_disconnect();
                printf("Disconnect: %ld\n", res);
                printf("isConnected: %ld\n", tws_isConnected());
                return 0;
            }
            break;

        }


    case WM_DESTROY:
        tws_disconnect();
        PostQuitMessage(0);
        break;


    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;

}



int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    
    // I want to create console terminal for GUI application
    // Allocate new console for app:
    AllocConsole();
    
    // Redirect stderr/stdout/stdin to new console
    BindStdHandlesToConsole();


    CWindow* pWindowMain = new CWindow();
    if (pWindowMain == nullptr) return FALSE;

    HWND hWndMain = pWindowMain->Create(NULL, L"IB-Tracker tester", Main_WindowProc, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600);

    //HWND hCtl;
    //hCtl = pWindowMain->AddControl(Controls::Button, hWndMain, IDC_CONNECT, L"Connect", 10, 10, 100, 30);
    //hCtl = pWindowMain->AddControl(Controls::Button, hWndMain, IDC_DISCONNECT, L"Disconnect", 10, 50, 100, 30);

    
    // Load the child windows
    CWindow* pWindowNavPanel = NavPanel_Show(hWndMain);
    
    // Center the main window within the desktop taking into account the actual work area.
    AfxCenterWindow(hWndMain, HWND_DESKTOP);

    pWindowMain->DoEvents(nCmdShow);

    GdiplusShutdown(gdiplusToken);

    if (pWindowNavPanel) delete(pWindowNavPanel);
    if (pWindowMain) delete(pWindowMain);

    return 0;
}
