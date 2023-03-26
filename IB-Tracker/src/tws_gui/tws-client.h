#pragma once

#include "pch.h"

#include "TwsWrapper.h"


void threadFunction(std::future<void> future);
void StartMonitorThread();
void EndMonitorThread();
bool tws_connect();
bool tws_disconnect();
bool tws_isConnected();

