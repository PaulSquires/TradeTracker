
#include "pch.h"
#include "tws-client.h"
#include "UserMessages.h"


TwsClient client;


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
        }
        else {
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

    SendMessage(HWND_NAVPANEL, MSG_TWS_CONNECT_START, 0, 0);

    bool res = client.connect(host, port, clientId);
    if (res) {
        // Start thread that will start messaging polling
        // and poll if TWS remains connected.
        SendMessage(HWND_NAVPANEL, MSG_TWS_CONNECT_SUCCESS, 0, 0);

        client.processMessages();

        StartMonitorThread();
    }
    else {
        SendMessage(HWND_NAVPANEL, MSG_TWS_CONNECT_FAILURE, 0, 0);
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

