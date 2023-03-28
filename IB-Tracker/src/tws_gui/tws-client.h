#pragma once

#include "pch.h"


#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EReader.h"

#include <memory>
#include <vector>

#include "TradesPanel.h"



class EClientSocket;


class TwsClient : public EWrapper
{
public:

	TwsClient();
	~TwsClient();

public:

	bool connect(const char* host, int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;
	bool isSocketOK() const;
	void waitForSignal();
	void processMsgs();
	void cancelMktData(TickerId tickerId);
	void requestMktData(LineData* ld);

public:
	// events
#include "EWrapper_prototypes.h"


private:
	//! [socket_declare]
	EReaderOSSignal m_osSignal;
	EClientSocket* const m_pClient;
	//! [socket_declare]

	std::unique_ptr<EReader> m_pReader;
	bool m_extraAuth;
	std::string m_bboExchange;

};

void threadFunction(std::future<void> future);
void StartMonitorThread();
void EndMonitorThread();
bool tws_connect();
bool tws_disconnect();
bool tws_isConnected();
void tws_cancelMktData(TickerId tickerId);
void tws_requestMktData(LineData* ld);
void tws_PauseTWS();
void tws_ResumeTWS();

