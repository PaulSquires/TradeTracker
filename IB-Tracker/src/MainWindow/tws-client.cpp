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

#include "pch.h"

#include <chrono>

#include "Utilities/UserMessages.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/AfxWin.h"
#include "Config/Config.h"
#include "ActiveTrades/ActiveTrades.h"
#include "Reconcile/Reconcile.h"
#include "Utilities/IntelDecimal.h"
#include "SideMenu/SideMenu.h"

#include "tws-api/EClientSocket.h"
#include "tws-api/EPosixClientSocketPlatform.h"
#include "tws-api/CommonDefs.h"
#include "tws-api/Utils.h"

#include "tws-client.h"


std::atomic<bool> isThreadPaused = false;
std::atomic<bool> isMonitorThreadActive = false;

bool MarketDataSubscriptionError = false;

TwsClient client;



//
// Perform the ITM calculation and update the Trade pointer with the values.
//
void PerformITMcalculation(std::shared_ptr<Trade>& trade)
{

	bool isITMred = false;
	bool isITMgreen = false;
	bool inLongSpread = false;

	for (const auto& leg : trade->openLegs) {
		if (leg->underlying == L"OPTIONS") {
			if (leg->PutCall == L"P") {
				if (trade->tickerLastPrice < AfxValDouble(leg->strikePrice)) {
					if (leg->openQuantity < 0) {
						isITMred = (inLongSpread == true) ? false : true;
					}
					if (leg->openQuantity > 0) {
						isITMgreen = true; isITMred = false; inLongSpread = true;
					}
				}
			}
			else if (leg->PutCall == L"C") {
				if (trade->tickerLastPrice > AfxValDouble(leg->strikePrice)) {
					if (leg->openQuantity < 0) {
						isITMred = (inLongSpread == true) ? false : true;
					}
					if (leg->openQuantity > 0) {
						isITMgreen = true; isITMred = false; inLongSpread = true;
					}
				}
			}
		}
	}

	std::wstring wszText = L"";

	DWORD themeEl = COLOR_WHITELIGHT;
	if (isITMred) {
		wszText = L"ITM";
		themeEl = COLOR_RED;
	}
	if (isITMgreen) {
		wszText = L"ITM";
		themeEl = COLOR_GREEN;
	}

	trade->wszITM = wszText;
	trade->clrITM = themeEl;

}



//
// If not connected to TWS or after hours and no market data then attmept to scrape yahoo finance
// to get the closing price of the stock.
//
double GetScrapedClosingPrice(std::wstring wszTickerSymbol)
{
	static std::wstring curlCommand = L"C:/Windows/System32/curl.exe";
	static bool curlExists = AfxFileExists(curlCommand);
	if (!curlExists) return 0;

	std::wstring wszDate1 = AfxCurrentDate();

	// If this is a Saturday or Sunday then we need to get the epoch timestamp for the preceeding Friday.
	static int DayOfWeek = AfxLocalDayOfWeek();
	if (DayOfWeek == 0) wszDate1 = AfxDateAddDays(wszDate1, -2);   // Sunday
	if (DayOfWeek == 6) wszDate1 = AfxDateAddDays(wszDate1, -1);   // Saturday

	std::wstring wszDate2 = AfxDateAddDays(wszDate1, 1);

	std::wstring cmd = L"C:/Windows/System32/curl.exe \"https://query1.finance.yahoo.com/v7/finance/download/" 
		+ wszTickerSymbol + 
		L"?interval=1d" + 
		L"&period1=" + std::to_wstring(AfxUnixTime(wszDate1)) + 
		L"&period2=" + std::to_wstring(AfxUnixTime(wszDate2)) + 
		L"&events=history\"";
	std::wstring wszText = AfxExecCmd(cmd);

	std::wstring wszClosingPrice = L"";

	// Get the last line and parse for the closing price
	if (wszText.size()) {
		std::vector<std::wstring> lines = AfxSplit(wszText, L"\n");
		std::wstring wszLastLine = lines.at(lines.size() - 1);
		std::vector<std::wstring> prices = AfxSplit(wszLastLine, L",");
		try { wszClosingPrice = prices.at(4); }
		catch (...) {}
	}

	return AfxValDouble(wszClosingPrice);
}


//
// Could not connect to TWS so we don't have any market data. Allow user the opportunity to try to get scraped data.
//
void UpdateTickersWithScrapedData()
{
	SetCursor(LoadCursor(0, IDC_WAIT));

	std::unordered_map<std::wstring, double> mapPrices;

	HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);

	int lbCount = ListBox_GetCount(hListBox);

	for (int nIndex = 0; nIndex < lbCount; nIndex++) {
		ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);
		if (ld == nullptr) continue;

		if ((ld->trade != nullptr) && (ld->lineType == LineType::TickerLine)) {

			if (ld->trade->tickerLastPrice == 0 && ld->trade->tickerClosePrice == 0) {

				std::wstring wszTickerSymbol = ld->trade->tickerSymbol;
				if (wszTickerSymbol == L"SPX") wszTickerSymbol = L"^SPX";
				if (wszTickerSymbol == L"NDX") wszTickerSymbol = L"^NDX";
				if (wszTickerSymbol == L"RUT") wszTickerSymbol = L"^RUT";
				if (wszTickerSymbol == L"VIX") wszTickerSymbol = L"^VIX";
				if (IsFuturesTicker(wszTickerSymbol)) {
					wszTickerSymbol = wszTickerSymbol.substr(1);
					if (wszTickerSymbol == L"AUD") wszTickerSymbol = L"6A";
					if (wszTickerSymbol == L"GBP") wszTickerSymbol = L"6B";
					if (wszTickerSymbol == L"EUR") wszTickerSymbol = L"6E";
					if (wszTickerSymbol == L"CAD") wszTickerSymbol = L"CD";
					if (wszTickerSymbol == L"JPY") wszTickerSymbol = L"JY";
					if (wszTickerSymbol == L"CHF") wszTickerSymbol = L"SF";
					if (wszTickerSymbol == L"INR") wszTickerSymbol = L"IR";
					if (wszTickerSymbol == L"MCL") wszTickerSymbol = L"CL";
					if (wszTickerSymbol == L"MES") wszTickerSymbol = L"ES";
					if (wszTickerSymbol == L"MNQ") wszTickerSymbol = L"NQ";
					if (wszTickerSymbol == L"M2K") wszTickerSymbol = L"2K";
					wszTickerSymbol = wszTickerSymbol + L"=F";
				}

				// Lookup our map to see if we have already retrieved the price from a previous scrap. If
				// yes, then use that value rather than doing yet another scrap.
				if (mapPrices.count(wszTickerSymbol)) {
					ld->trade->tickerClosePrice = mapPrices.at(wszTickerSymbol);
				}
				else {
					ld->trade->tickerClosePrice = GetScrapedClosingPrice(wszTickerSymbol);
					mapPrices[wszTickerSymbol] = ld->trade->tickerClosePrice;
				}
				ld->trade->tickerLastPrice = ld->trade->tickerClosePrice;

				PerformITMcalculation(ld->trade);
				ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->wszITM, ld->trade->clrITM);  // ITM

				std::wstring wszText = AfxMoney(ld->trade->tickerClosePrice, false, ld->trade->tickerDecimals);
				ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, wszText, COLOR_WHITELIGHT);  // current price

				RECT rc{};
				ListBox_GetItemRect(hListBox, nIndex, &rc);
				InvalidateRect(hListBox, &rc, TRUE);
				UpdateWindow(hListBox);
			}
		}
	}

	// Do calculation to ensure column widths are wide enough to accommodate the new
	// price data that has just arrived.
	if (ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1)) {
		AfxRedrawWindow(hListBox);
	}

	SetCursor(LoadCursor(0, IDC_ARROW));
}



//
// Thread function
//
std::jthread monitoring_thread;
std::jthread ping_thread;


void pingFunction(std::stop_token st) {
	std::cout << "Starting the ping thread" << std::endl;

	while (!st.stop_requested()) {

		try {
			if (isMonitorThreadActive == true) {
				// Send a request time message (ping)
				client.pingTWS();
			}
			else {
				break;
			}
		}
		catch (...) {
			break;
		}
		// Sleep for 10 seconds
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}

	std::cout << "Ping Thread Terminated" << std::endl;
}


void monitoringFunction(std::stop_token st) {
	std::cout << "Starting the monitoring thread" << std::endl;
	
	isMonitorThreadActive = true;

	// Create and start the ping thread
	ping_thread = std::jthread(pingFunction);


	while (!st.stop_requested()) {

		try {
			if (tws_isConnected()) {
				client.waitForSignal();
				client.processMsgs();
			}
			else {
				break;
			}
		}
		catch (...) {
			PostMessage(HWND_SIDEMENU, MSG_TWS_WARNING_EXCEPTION, 0, 0);
			break;
		}

	}

	isMonitorThreadActive = false;

	// Shut down the Ping thread prior to ending this Monitoring thread
	ping_thread.request_stop();


	std::cout << "Requesting Ping Thread to Terminate" << std::endl;
	std::cout << "Monitoring Thread Terminated" << std::endl;
	PostMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_DISCONNECT, 0, 0);
}


void StartMonitorThread()
{
	if (isMonitorThreadActive) return;
	isThreadPaused = false;   // allow processing TickData
	monitoring_thread = std::jthread(monitoringFunction);
}


void EndMonitorThread()
{
	if (isMonitorThreadActive) {
		isThreadPaused = true;   // prevent processing TickData, etc while thread is shutting down
		std::cout << "Threads will be stopped soon...." << std::endl;
		monitoring_thread.request_stop();
	}
}


bool tws_connect()
{
    if (tws_isConnected()) return false;

    const char* host = "";
    int port = 7496;   // 7497 is paper trading account
    int clientId = 0;

	
	SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_START, 0, 0);

	bool res = false;

	try {
		res = client.connect(host, port, clientId);
		if (res) {
			// Start thread that will start messaging polling
			// and poll if TWS remains connected.
			SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_SUCCESS, 0, 0);
			
			StartMonitorThread();

			// Destroy any existing ListBox line data
			// This will also clear the LineData pointers and cancel any previous market data
			PrevMarketDataLoaded = false;
			ListBoxData_DestroyItemData(GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX));
			ActiveTrades_ShowActiveTrades();

		}

	}
	catch (...) {
		SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_FAILURE, 0, 0);
		std::wstring wszText =
			L"Socket exception error trying to connect to TWS.\n\nPlease try to connect again or restart the application if the problem persists.";
		MessageBox(HWND_ACTIVETRADES, wszText.c_str(), L"Connection Failed", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	
	if (res == false) {
        SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_FAILURE, 0, 0);
		std::wstring wszText =
			L"Could not connect to TWS.\n\n" \
			"Confirm in TWS, File->Global Configuration->API->Settings menu that 'Enable ActiveX and Client Sockets' is enabled and connection port is set to 7496.\n\n" \
			"Do you wish to retrieve closing price quotes from scraped Yahoo Finance data?";
		if (MessageBox(HWND_ACTIVETRADES, wszText.c_str(), L"Connection Failed", MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDYES) {
			UpdateTickersWithScrapedData();
		}
		return false;
    }

    return res;
}


bool tws_disconnect()
{
    if (tws_isConnected() == false) return true;
	isThreadPaused = true;

    EndMonitorThread();
    client.disconnect();

    bool res = (tws_isConnected(), false, true);

    return res;
}


bool tws_isConnected()
{
	bool res = false;
    if ((client.isSocketOK()) && (client.isConnected())) {
        res = true;
    }
    return res;
}


void tws_cancelMktData(TickerId tickerId)
{
	if (!tws_isConnected()) return;
	if (tickerId < 1) return;
	client.cancelMktData(tickerId);
}


void tws_requestMktData(ListBoxData* ld)
{
	if (!tws_isConnected()) return;
	if (ld->trade == nullptr) return;
	client.requestMktData(ld);
}


void tws_PauseTWS()
{
	if (!tws_isConnected()) return;
	isThreadPaused = true;
}


void tws_ResumeTWS()
{
	if (!tws_isConnected()) return;
	isThreadPaused = false;
}


void tws_requestPortfolioUpdates()
{
	// Load the IBKR and Local positions into the vectors and do the matching. This is
	// important because we need get the contract id's loaded into each option leg
	// in irder for the Portfolio Value updates to match it.
	client.cancelPositions();
	client.requestPositions();

	// When requestPositions completes, it sends a notification to the Active Trades
	// window that it is now okay to request the Portfolio Updates. We make those
	// portfolio update calls there rather than here.
}


void tws_performReconciliation()
{
	if (!tws_isConnected()) {
		MessageBox(
			HWND_SIDEMENU,
			(LPCWSTR)L"Must be connected to TWS to perform a reconciliation.",
			(LPCWSTR)L"Error",
			MB_ICONINFORMATION);
		return;
	}

	// Prevent Reconcile from running prior to previous invocation completing
	static bool isRunning = false;
	if (isRunning == true) return;

	isRunning = true;

	// Load the IBKR and Local positions into the vectors and do the matching
	// The results will be saved into module global resultsText which will be displayed
	// when Reconcile_Show() is called and Reconcile_positionEnd() has SendMessage notification
	// to the dialog to say that text is ready.
	client.cancelPositions();
	client.requestPositions();

	// Show the results
	Reconcile_Show();

	isRunning = false;
}


///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]
TwsClient::TwsClient() :
	m_osSignal(2000)//2-seconds timeout
	, m_pClient(new EClientSocket(this, &m_osSignal))
	, m_extraAuth(false)
{
}
//! [socket_init]
TwsClient::~TwsClient()
{
	// destroy the reader before the client
	if (m_pReader)
		m_pReader.reset();

	delete m_pClient;
}

bool TwsClient::connect(const char* host, int port, int clientId)
{
	// trying to connect
	printf("Connecting to %s:%d clientId:%d\n", !(host && *host) ? "127.0.0.1" : host, port, clientId);

	bool bRes = m_pClient->eConnect(host, port, clientId, m_extraAuth);

	if (bRes) {
		printf("Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
		m_pReader = std::unique_ptr<EReader>(new EReader(m_pClient, &m_osSignal));
		m_pReader->start();

	}
	else {
		printf("Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
	}
	return bRes;
}

void TwsClient::pingTWS() const
{
	m_pClient->reqCurrentTime();

	printf("ping\n");
}

void TwsClient::disconnect() const
{
	m_pClient->eDisconnect();

	printf("Disconnected\n");
}

bool TwsClient::isConnected() const
{
	return m_pClient->isConnected();
}

bool TwsClient::isSocketOK() const
{
	return m_pClient->isSocketOK();
}

void TwsClient::waitForSignal()
{
	m_osSignal.waitForSignal();

}

void TwsClient::processMsgs()
{
	m_pReader->processMsgs();
}


void TwsClient::cancelPortfolioUpdates()
{
	m_pClient->reqAccountUpdates(false, "");
}

void TwsClient::requestPortfolioUpdates()
{
	m_pClient->reqAccountUpdates(true, "");
}


void TwsClient::cancelMktData(TickerId tickerId)
{
	m_pClient->cancelMktData(tickerId);
}


void TwsClient::requestMktData(ListBoxData* ld)
{
	// Convert the unicode symbol to regular string type
	std::string symbol = unicode2ansi(ld->trade->tickerSymbol);

	//	struct Contract;
	Contract contract;

	if (symbol == "SPX" || symbol == "DJX" || symbol == "VIX") {
		contract.symbol = symbol;
		contract.secType = "IND";
		contract.currency = "USD";
		contract.exchange = "CBOE";
		contract.primaryExchange = "NASDAQ";   // TWS is moving from ISLAND -> NASDAQ naming.
	}
	else {

		if (symbol.substr(0,1) == "/") {
			contract.symbol = symbol.substr(1);
			contract.secType = "FUT";
			contract.currency = "USD";
			contract.lastTradeDateOrContractMonth = AfxFormatFuturesDateMarketData(ld->trade->futureExpiry);   // YYYYMMDD

			std::string futExchange = GetFuturesExchange(symbol);
			if (futExchange.length() == 0) futExchange = "CME";

			contract.exchange = futExchange;
			contract.primaryExchange = futExchange;
		}
		else {
			contract.symbol = symbol;
			contract.secType = "STK";
			contract.currency = "USD";
			contract.exchange = "SMART";
			contract.primaryExchange = "NASDAQ";   // TWS is moving from ISLAND -> NASDAQ naming.
		}

	}
	m_pClient->reqMktData(ld->tickerId, contract, "", false, false, TagValueListSPtr());
}
	
void TwsClient::cancelPositions()
{
	m_pClient->cancelPositions();
}

void TwsClient::requestPositions()
{
	m_pClient->reqPositions();
}



//////////////////////////////////////////////////////////////////
// methods
void TwsClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
		m_pClient->startApi();
}


void TwsClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
	if (isThreadPaused) return;

	// Handle any HALTED tickers (Halted notifications only arrive via tickGeneric).
	//Value	Description
	//	- 1	Halted status not available.Usually returned with frozen data.
	//	0	Not halted.This value will only be returned if the contract is in a TWS watchlist.
	//	1	General halt.Trading halt is imposed for purely regulatory reasons with / without volatility halt.
	//	2	Volatility halt.Trading halt is imposed by the exchange to protect against extreme volatility.

	if (tickType == HALTED && value == 1 || value == 2) {  // 49
		HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
		int lbCount = ListBox_GetCount(hListBox);

		for (int nIndex = 0; nIndex < lbCount; nIndex++) {
			ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);
			if (ld == nullptr) continue;

			if ((ld->tickerId == tickerId) && (ld->trade != nullptr) && (ld->lineType == LineType::TickerLine)) {
				ld->SetTextData(COLUMN_TICKER_CHANGE, L"HALTED", COLOR_RED);
				if (ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1)) {
					AfxRedrawWindow(hListBox);
				}
				break;
			}
		}
	}
}




void TwsClient::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {

	if (isThreadPaused) return;

	// Market data tick price callback. Handles all price related ticks. Every tickPrice callback is followed 
	// by a tickSize. A tickPrice value of - 1 or 0 followed by a tickSize of 0 indicates there is no data for 
	// this field currently available, whereas a tickPrice with a Green tickSize indicates an active 
	// quote of 0 (typically for a combo contract).

	// Parameters
	// tickerId	the request's unique identifier.
	// field	the type of the price being received(i.e.ask price)(TickType)
	// price	the actual price.
	// attribs	an TickAttrib object that contains price attributes such as TickAttrib::CanAutoExecute, TickAttrib::PastLimit and TickAttrib::PreOpen.

	// Most pertinent TickType fields for our use would be the following:
	// enum TickType { LAST, CLOSE, OPEN }
	// Just dealing with these 3 fields cuts out a **LOT** of tickPrice notifications.
	if (field == LAST || field == OPEN || field == CLOSE) {

		// These columns in the table are updated in real time when connected
		// to TWS. The LineData pointer is updated via a call to SetColumnData
		// and the correct ListBox line is invalidated/redrawn in order to force
		// display of the new price data. 

		HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);

		int lbCount = ListBox_GetCount(hListBox);
		
		for (int nIndex = 0; nIndex < lbCount; nIndex++) {
			
			ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);
			if (ld == (void*)-1) continue;
			if (ld == nullptr) continue;

			if ((ld->tickerId == tickerId) && (ld->trade != nullptr) && (ld->lineType == LineType::TickerLine)) {

				if (field == LAST) {
					ld->trade->tickerLastPrice = price;
				}

				if (field == CLOSE) {
					ld->trade->tickerClosePrice = price;
					if (ld->trade->tickerLastPrice == 0)
						ld->trade->tickerLastPrice = price;
				}

				if (field == OPEN) {
					if (ld->trade->tickerLastPrice == 0)
						ld->trade->tickerLastPrice = price;
					if (ld->trade->tickerClosePrice == 0)
						ld->trade->tickerClosePrice = price;
				}



				// Calculate the price change
				double delta = 0;
				if (ld->trade->tickerClosePrice != 0) {
					delta = (ld->trade->tickerLastPrice - ld->trade->tickerClosePrice);
				}

				
				std::wstring wszText;
				DWORD themeEl = COLOR_WHITELIGHT;

				// Calculate if any of the option legs are ITM in a good (green) or bad (red) way.
				// We use a separate function call because scrapped data will need acces to the 
				// ITM calculation also.
				PerformITMcalculation(ld->trade);


				ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->wszITM, ld->trade->clrITM);  // ITM


				wszText = AfxMoney(delta, true, ld->trade->tickerDecimals);
				ld->trade->wszTickerChange = wszText;
				themeEl = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
				ld->trade->clrTickerChange = themeEl;
				ld->SetTextData(COLUMN_TICKER_CHANGE, wszText, themeEl);  // price change

				wszText = AfxMoney(ld->trade->tickerLastPrice, false, ld->trade->tickerDecimals);
				ld->trade->wszTickerLastPrice = wszText;
				ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, wszText, COLOR_WHITELIGHT);  // current price

				wszText = (delta >= 0 ? L"+" : L"") + AfxMoney((delta / ld->trade->tickerLastPrice) * 100, true) + L"%";
				ld->trade->wszTickerPercentChange = wszText;
				themeEl = (delta >= 0) ? COLOR_GREEN : COLOR_RED;
				ld->trade->clrTickerPercentChange = themeEl;
				ld->SetTextData(COLUMN_TICKER_PERCENTCHANGE, wszText, themeEl);  // price percentage change


				RECT rc{};
				ListBox_GetItemRect(hListBox, nIndex, &rc);
				InvalidateRect(hListBox, &rc, TRUE);
				UpdateWindow(hListBox);

				break;

			}  // if
		}  // for


		// Do calculation to ensure column widths are wide enough to accommodate the new
		// price data that has just arrived.
		if (ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1) == true) {
			AfxRedrawWindow(hListBox);
		}

	}  // if
}



void TwsClient::updatePortfolio(const Contract& contract, Decimal position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const std::string& accountName) {

	if (isThreadPaused) return;

	//printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: %s, AccountName: %s\n",
	//	(contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), decimalStringToDisplay(position).c_str(),
	//	Utils::doubleMaxString(marketPrice).c_str(), Utils::doubleMaxString(marketValue).c_str(), Utils::doubleMaxString(averageCost).c_str(),
	//	Utils::doubleMaxString(unrealizedPNL).c_str(), Utils::doubleMaxString(realizedPNL).c_str(), accountName.c_str());

	// Match the incoming contractID with the contractId stored in the Leg.

	HWND hListBox = GetDlgItem(HWND_ACTIVETRADES, IDC_TRADES_LISTBOX);
	int lbCount = ListBox_GetCount(hListBox);

	std::wstring wszText = L"";
	DWORD themeEl = COLOR_WHITEDARK;

	int nIndexTrade = 0;

	for (int nIndex = 0; nIndex < lbCount; nIndex++) {
		ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);

		if (ld == (void*)-1) continue;
		if (ld == nullptr) continue;

		if (ld->lineType == LineType::TickerLine) nIndexTrade = nIndex;
		if (ld->leg == nullptr) continue;


		if (ld->leg->contractId == contract.conId) {
			themeEl = COLOR_WHITEDARK;

			double averagePrice = (averageCost / ld->trade->multiplier);
			ld->leg->averagePrice = averagePrice;
			wszText = AfxMoney(averagePrice, true, ld->trade->tickerDecimals);
			ld->leg->wszAveragePrice = wszText;
			ld->SetTextData(COLUMN_TICKER_AVGPX, wszText, themeEl);   // Book Value and average Price

			ld->leg->marketPrice = marketPrice;
			wszText = AfxMoney(marketPrice, true, ld->trade->tickerDecimals);
			ld->leg->wszMarketPrice = wszText;
			ld->SetTextData(COLUMN_TICKER_LASTPX, wszText, themeEl);   // Market Value and Last Price

			double percentage = (averagePrice - marketPrice) / averagePrice * 100;
			if (ld->leg->openQuantity > 0) percentage = percentage * -1;
			ld->leg->percentage = percentage;
			wszText = AfxMoney(percentage, true, 1) + L"%";
			ld->leg->wszPercentage = wszText;
			ld->SetTextData(COLUMN_TICKER_PERCENTCOMPLETE, wszText, themeEl);  // Percentage values for the previous two columns data
			
			ld->leg->unrealizedPNL = unrealizedPNL;
			themeEl = (unrealizedPNL < 0) ? COLOR_RED : COLOR_GREEN;
			wszText = AfxMoney(unrealizedPNL, false, 0);
			ld->leg->wszUnrealizedPNL = wszText;
			ld->leg->clrUnrealizedPNL = themeEl;
			ld->SetTextData(COLUMN_TICKER_UPNL, wszText, themeEl);    // Unrealized profit or loss

			RECT rc{};
			ListBox_GetItemRect(hListBox, nIndex, &rc);
			InvalidateRect(hListBox, &rc, TRUE);
			UpdateWindow(hListBox);


			// Update the Trade's tickerLine with the new totals
			ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndexTrade);
			if (ld != nullptr) {
				double percentage = 0;
				double uPNL = 0;
				int numLegs = 0;
				for (const auto& leg : ld->trade->openLegs)	{
					percentage += leg->percentage;
					uPNL += leg->unrealizedPNL;
					++numLegs;
				}

				themeEl = COLOR_WHITEDARK;
				wszText = AfxMoney(percentage / numLegs, true, 1) + L"%";
				ld->trade->wszPercentage = wszText;
				ld->SetTextData(COLUMN_TICKER_PERCENTCOMPLETE, wszText, themeEl);  // Percentage values

				themeEl = (uPNL < 0) ? COLOR_RED : COLOR_GREEN;
				wszText = AfxMoney(uPNL, false, 0);
				ld->trade->wszUnrealizedPNL = wszText;
				ld->trade->clrUnrealizedPNL = themeEl;
				ld->SetTextData(COLUMN_TICKER_UPNL, wszText, themeEl);    // Unrealized profit or loss

				RECT rc{};
				ListBox_GetItemRect(hListBox, nIndexTrade, &rc);
				InvalidateRect(hListBox, &rc, TRUE);
				UpdateWindow(hListBox);
			}


		}
	}

	if (ListBoxData_ResizeColumnWidths(hListBox, TableType::ActiveTrades, -1) == true) {
		AfxRedrawWindow(hListBox);
	}

}


void TwsClient::connectionClosed() {
	printf("Connection Closed\n");
}

void TwsClient::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson)
{
	switch (errorCode) {
	case 2104:
	case 2106:
	case 2158:
		return;
	}
	printf("Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());


	// If error codes 10091 or 10089 then we are connected most likely after hours and we do not have
	// access to streaming data. In this case we will attempt scrap for the closing price.
	switch (errorCode) {
	case 10089:
	case 10091:
		MarketDataSubscriptionError = true;
		return;
	}

	switch (errorCode) {
	case 1100:   // 'Connectivity between IB and Trader Workstation has been lost.'
	{
		// This normally occurs when the internet connection is lost. It refers to the 
		// connection between TWS and IBKR, and not TWS and IB-Tracker. The connection
		// between IB-Tracker and TWS will resume as soon as TWS reconnects back to the
		// IBKR servers (ie. IB-Trackers sockets remain open).
		SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_WAIT_RECONNECTION, 0, 0);
		std::wstring wszText =
			L"TWS has lost connection to the IBKR servers (Internet connection down?).\n\nIB-Tracker will resume automatically when TWS reconnects to IBKR.";
		MessageBox(HWND_ACTIVETRADES, wszText.c_str(), L"Connection Failed", MB_OK | MB_ICONEXCLAMATION);
	}
	break;

	case 1102:   // 'Connectivity between IB and Trader Workstation reestablished.'
	{
		SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_SUCCESS, 0, 0);
	}
	break;
	
	}
}


void TwsClient::position(const std::string& account, const Contract& contract, Decimal position, double avgCost)
{
	// This callback is initiated by the reqPositions() call via the clicking on Reconcile button.
	Reconcile_position(contract, position);
}

void TwsClient::positionEnd()
{
	// This callback is automatically called the first time all positions have been sent through
	// the position callback.
	m_pClient->cancelPositions();
	Reconcile_positionEnd();

	// We have finished requesting positions. It is possible that some position closing prices were 
	// not retrieved because maybe we are connected but it is after hours and we need additional
	// subscriptions to access the data.
	// Check global variable for the market subscription error and then attempt scraping from yahoo finance.
	if (MarketDataSubscriptionError) {
		UpdateTickersWithScrapedData();
		MarketDataSubscriptionError = false;
	}

}

void TwsClient::updateAccountValue(const std::string& key, const std::string& val,
	const std::string& currency, const std::string& accountName) {
	// This callback will fire when PortfolioValue updates are initiated. We look for the
	// string "AccountReady". 
	//std::cout << "updateAccountValue: " << key << std::endl;
}


void TwsClient::tickOptionComputation(TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
	double optPrice, double pvDividend,
	double gamma, double vega, double theta, double undPrice) {
	if (isThreadPaused) return;
}

void TwsClient::tickSize(TickerId tickerId, TickType field, Decimal size) {
	if (isThreadPaused) return;
	//std::cout << "tickSize  id: " << tickerId << "  size: " << (int)intelDecimalToDouble(size) << std::endl;
}

void TwsClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	// printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
	if (isThreadPaused) return;
}

void TwsClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
	double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
	if (isThreadPaused) return;
}

void TwsClient::winError(const std::string& str, int lastError) {}

void TwsClient::nextValidId(OrderId orderId) {
}

void TwsClient::currentTime(long time) {
	std::cout << "current time " << time << std::endl;
}

void TwsClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
	Decimal remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
}

void TwsClient::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
}

void TwsClient::openOrderEnd() {
}

void TwsClient::updateAccountTime(const std::string& timeStamp) {
}

void TwsClient::accountDownloadEnd(const std::string& accountName) {
}

void TwsClient::contractDetails(int reqId, const ContractDetails& contractDetails) {
}

void TwsClient::bondContractDetails(int reqId, const ContractDetails& contractDetails) {
}

void TwsClient::contractDetailsEnd(int reqId) {
}

void TwsClient::execDetails(int reqId, const Contract& contract, const Execution& execution) {
}

void TwsClient::execDetailsEnd(int reqId) {
}

void TwsClient::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, Decimal size) {
}

void TwsClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
	int side, double price, Decimal size, bool isSmartDepth) {
}

void TwsClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
}

void TwsClient::managedAccounts(const std::string& accountsList) {
}

void TwsClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
}

void TwsClient::historicalData(TickerId reqId, const Bar& bar) {
}

void TwsClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
}

void TwsClient::scannerParameters(const std::string& xml) {
}

void TwsClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
	const std::string& distance, const std::string& benchmark, const std::string& projection,
	const std::string& legsStr) {
}

void TwsClient::scannerDataEnd(int reqId) {
}

void TwsClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	Decimal volume, Decimal wap, int count) {
}

void TwsClient::fundamentalData(TickerId reqId, const std::string& data) {
}

void TwsClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
}

void TwsClient::tickSnapshotEnd(int reqId) {
}

void TwsClient::marketDataType(TickerId reqId, int marketDataType) {
}

void TwsClient::commissionReport(const CommissionReport& commissionReport) {
}

void TwsClient::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
}

void TwsClient::accountSummaryEnd(int reqId) {
}

void TwsClient::verifyMessageAPI(const std::string& apiData) {
}

void TwsClient::verifyCompleted(bool isSuccessful, const std::string& errorText) {
}

void TwsClient::verifyAndAuthMessageAPI(const std::string& apiDatai, const std::string& xyzChallenge) {
}

void TwsClient::verifyAndAuthCompleted(bool isSuccessful, const std::string& errorText) {
}

void TwsClient::displayGroupList(int reqId, const std::string& groups) {
}

void TwsClient::displayGroupUpdated(int reqId, const std::string& contractInfo) {
}

void TwsClient::positionMulti(int reqId, const std::string& account, const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) {
}

void TwsClient::positionMultiEnd(int reqId) {
}

void TwsClient::accountUpdateMulti(int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
}

void TwsClient::accountUpdateMultiEnd(int reqId) {
}

void TwsClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
	const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {
}

void TwsClient::securityDefinitionOptionalParameterEnd(int reqId) {
}

void TwsClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier>& tiers) {
}

void TwsClient::familyCodes(const std::vector<FamilyCode>& familyCodes) {
}

void TwsClient::symbolSamples(int reqId, const std::vector<ContractDescription>& contractDescriptions) {
}

void TwsClient::mktDepthExchanges(const std::vector<DepthMktDataDescription>& depthMktDataDescriptions) {
	printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());
}

void TwsClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
}

void TwsClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
}

void TwsClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
}

void TwsClient::newsProviders(const std::vector<NewsProvider>& newsProviders) {
}

void TwsClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
}

void TwsClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
}

void TwsClient::historicalNewsEnd(int requestId, bool hasMore) {
}

void TwsClient::headTimestamp(int reqId, const std::string& headTimestamp) {
}

void TwsClient::histogramData(int reqId, const HistogramDataVector& data) {
}

void TwsClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
}

void TwsClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
}

void TwsClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
}

void TwsClient::marketRule(int marketRuleId, const std::vector<PriceIncrement>& priceIncrements) {
}

void TwsClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
}

void TwsClient::pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
}

void TwsClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
}

void TwsClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
}

void TwsClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
}

void TwsClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
}

void TwsClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) {
}

void TwsClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
}

void TwsClient::orderBound(long long orderId, int apiClientId, int apiOrderId) {
}

void TwsClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
}

void TwsClient::completedOrdersEnd() {
}

void TwsClient::replaceFAEnd(int reqId, const std::string& text) {
}

void TwsClient::wshMetaData(int reqId, const std::string& dataJson) {
}

void TwsClient::wshEventData(int reqId, const std::string& dataJson) {
}

void TwsClient::historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) {
}

void TwsClient::userInfo(int reqId, const std::string& whiteBrandingId) {
}
