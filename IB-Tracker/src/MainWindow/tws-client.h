#pragma once

#include "..\tws-api\EWrapper.h"
#include "..\tws-api\EReaderOSSignal.h"
#include "..\tws-api\EReader.h"

#include "..\Utilities\ListBoxData.h"


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
	void requestMktData(ListBoxData* ld);
	void requestPositions();

public:
	// events
#include "..\tws-api\EWrapper_prototypes.h"


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
void tws_requestMktData(ListBoxData* ld);
void tws_performReconciliation();
void tws_PauseTWS();
void tws_ResumeTWS();

