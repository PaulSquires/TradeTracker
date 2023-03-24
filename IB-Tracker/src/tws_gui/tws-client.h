#pragma once

#include "framework.h"

#include "TwsWrapper.h"

#include <chrono>
#include <thread>
#include <future>

void threadFunction(std::future<void> future);
void StartMonitorThread();
void EndMonitorThread();
bool tws_connect();
bool tws_disconnect();
bool tws_isConnected();

