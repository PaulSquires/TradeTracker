/*

MIT License

Copyright(c) 2023 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "tws-api/EWrapper.h"
#include "tws-api/EReaderOSSignal.h"
#include "tws-api/EReader.h"

#include "Utilities/ListBoxData.h"


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
	void requestPortfolioUpdates();

public:
	// events
#include "tws-api/EWrapper_prototypes.h"


private:
	//! [socket_declare]
	EReaderOSSignal m_osSignal;
	EClientSocket* const m_pClient;
	//! [socket_declare]

	std::unique_ptr<EReader> m_pReader;
	bool m_extraAuth;
	std::string m_bboExchange;

};

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
void tws_requestPortfolioUpdates();

