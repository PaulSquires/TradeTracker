/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once
#ifndef TWS_API_SAMPLES_TESTCPPCLIENT_TESTCPPCLIENT_H
#define TWS_API_SAMPLES_TESTCPPCLIENT_TESTCPPCLIENT_H

#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EReader.h"

#include <memory>
#include <vector>


class EClientSocket;

enum State {
	ST_CONNECT,
	ST_TICKDATAOPERATION,
	ST_TICKDATAOPERATION_ACK,
	ST_PING,
	ST_PING_ACK,
    ST_REQTICKBYTICKDATA,
    ST_REQTICKBYTICKDATA_ACK,
	ST_IDLE,
	ST_WSH,
	ST_WSH_ACK
};

//! [ewrapperimpl]
class TwsClient : public EWrapper
{
//! [ewrapperimpl]
public:

	TwsClient();
	~TwsClient();

	void setConnectOptions(const std::string&);
	void processMessages();

public:

	bool connect(const char * host, int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;
	bool isSocketOK() const;
	void waitForSignal();
	void processMsgs();


private:
	void tickDataOperation();
    void reqTickByTickData();
	void reqCurrentTime();

public:
	// events
	#include "EWrapper_prototypes.h"


private:
	//! [socket_declare]
	EReaderOSSignal m_osSignal;
	EClientSocket * const m_pClient;
	//! [socket_declare]
	State m_state;
	time_t m_sleepDeadline;

	OrderId m_orderId;
	std::unique_ptr<EReader> m_pReader;
    bool m_extraAuth;
	std::string m_bboExchange;

};

#endif

