/*

MIT License

Copyright(c) 2023-2025 Paul Squires

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

#ifndef TWSCLIENT_H
#define TWSCLIENT_H

#if defined(_WIN32) // win32 and win64
    // Windows-specific code
	#include "tws-api/windows/EWrapper.h"
	#include "tws-api/windows/EReaderOSSignal.h"
	#include "tws-api/windows/EReader.h"
#else
// #elif defined(__WXGTK__)
// #elif defined(__WXMAC__)
	#include "tws-api/linux/EWrapper.h"
	#include "tws-api/linux/EReaderOSSignal.h"
	#include "tws-api/linux/EReader.h"
#endif

#include "list_panel_data.h"
#include "appstate.h"


class EClientSocket;


class TwsClient : public EWrapper
{
public:

	TwsClient();
	~TwsClient();

public:

	bool Connect(const char* host, int port, int clientId = 0);
	void Disconnect() const;
	bool IsConnected() const;
	bool IsSocketOK() const;
	void WaitForSignal();
	void ProcessMsgs();
	void CancelMarketData(TickerId ticker_id);
	void RequestMarketData(AppState& state, CListPanelData* ld);
	void RequestOpenLegData(AppState& state, CListPanelData* ld);
	void CancelPositions();
	void RequestPositions();
	void CancelPortfolioUpdates();
	void RequestPortfolioUpdates();
	void RequestAccountSummary();
	void PingTWS() const;

public:
	// events
#if defined(_WIN32) // win32 and win64
	#include "tws-api/windows/EWrapper_prototypes.h"
#else
	#include "tws-api/linux/EWrapper_prototypes.h"
#endif

	int client_id = 0;
	bool had_previous_socket_exception = false;

//private:
	//! [socket_declare]
	EReaderOSSignal m_osSignal{};
	//EClientSocket* const m_pClient{};
	EClientSocket* m_pClient{};
	//! [socket_declare]

	std::unique_ptr<EReader> m_pReader{};
	bool m_extraAuth;
	std::string m_bboExchange;
};



void tws_StartMonitorThread(AppState& state);
void tws_EndMonitorThread(AppState& state);
void tws_StartTickerUpdateThread(AppState& state);
void tws_EndTickerUpdateThread(AppState& state);
bool tws_Connect(AppState& state);
bool tws_Disconnect(AppState& state);
bool tws_IsConnected(AppState& state);
void tws_CancelMarketData(AppState& state, TickerId ticker_id);
void tws_PerformReconciliation(AppState& state);
void tws_RequestAccountSummary(AppState& state);
void tws_RequestPositions(AppState& state);
void tws_CancelPositions(AppState& state);

#endif //TWSCLIENT_H

