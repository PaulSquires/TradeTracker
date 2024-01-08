/*

MIT License

Copyright(c) 2023-2024 Paul Squires

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
#include <unordered_map>

#include "Utilities/UserMessages.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/AfxWin.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "Config/Config.h"
#include "ActiveTrades/ActiveTrades.h"
#include "MainWindow/MainWindow.h"
#include "TabPanel/TabPanel.h"
#include "Reconcile/Reconcile.h"
#include "tws-api/IntelDecimal/IntelDecimal.h"
#include "CustomLabel/CustomLabel.h"
#include "Database/trade.h"
#include "Utilities/ListBoxData.h"
#include "Reconcile/Reconcile.h"

#include "tws-api/EClientSocket.h"
#include "tws-api/EPosixClientSocketPlatform.h"
#include "tws-api/CommonDefs.h"
#include "tws-api/Utils.h"

#include "tws-client.h"

std::atomic<bool> is_monitor_thread_active = false;
std::atomic<bool> is_ping_thread_active = false;
std::atomic<bool> is_ticker_update_thread_active = false;

bool market_data_subscription_error = false;

std::jthread monitoring_thread;
std::jthread ticker_update_thread;
std::jthread ping_thread;

std::unordered_map<TickerId, TickerData> mapTickerData;
std::unordered_map<int, PortfolioData> mapPortfolioData;

TwsClient client;



//
// If not connected to TWS or after hours and no market data then attmept to scrape yahoo finance
// to get the closing price of the stock.
//
double GetScrapedClosingPrice(std::wstring ticker_symbol) {
	static std::wstring curl_command = L"C:/Windows/System32/curl.exe";
	static bool curl_exists = AfxFileExists(curl_command);
	if (!curl_exists) return 0;

	std::wstring date1 = AfxCurrentDate();

	// If this is a Saturday or Sunday then we need to get the epoch timestamp for the preceeding Friday.
	static int day_of_week = AfxLocalDayOfWeek();
	if (day_of_week == 0) date1 = AfxDateAddDays(date1, -2);   // Sunday
	if (day_of_week == 6) date1 = AfxDateAddDays(date1, -1);   // Saturday

	std::wstring date2 = AfxDateAddDays(date1, 1);

	std::wstring cmd = L"C:/Windows/System32/curl.exe \"https://query1.finance.yahoo.com/v7/finance/download/" 
		+ ticker_symbol + 
		L"?interval=1d" + 
		L"&period1=" + std::to_wstring(AfxUnixTime(date1)) + 
		L"&period2=" + std::to_wstring(AfxUnixTime(date2)) + 
		L"&events=history\"";
	std::wstring text = AfxExecCmd(cmd);

	std::wstring closing_price = L"";

	// Get the last line and parse for the closing price
	if (text.size()) {
		std::vector<std::wstring> lines = AfxSplit(text, L'\n');
		std::wstring last_line = lines.at(lines.size() - 1);
		std::vector<std::wstring> prices = AfxSplit(last_line, L',');
		try { closing_price = prices.at(4); }
		catch (...) {}
	}

	return AfxValDouble(closing_price);
}


//
// Could not connect to TWS so we don't have any market data. Allow user the opportunity to try to get scraped data.
//
#include <wininet.h>
#pragma comment(lib,"Wininet.lib")
void tws_UpdateTickersWithScrapedData() {
	// Do a simple check to see if connected to internet. If fail then simply
	// advise the user via a messgebox.
	std::wstring url = L"https://www.google.com";
	bool is_internet_available = InternetCheckConnection(url.c_str(), FLAG_ICC_FORCE_CONNECTION, 0);
	
	if (!is_internet_available) {
		CustomMessageBox.Show(MainWindow.hWindow,
			L"No Internet connection exists.\n\nCan not retrieve scraped ticker data.",
			L"Connection", MB_ICONWARNING);
		return;
	}

	SetCursor(LoadCursor(0, IDC_WAIT));

	std::unordered_map<std::wstring, double> map_prices;

	HWND hListBox = GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_LISTBOX);

	int item_count = ListBox_GetCount(hListBox);

	for (int index = 0; index < item_count; index++) {
		ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
		if (ld == nullptr) continue;

		if ((ld->trade != nullptr) && (ld->line_type == LineType::ticker_line)) {

			if (ld->trade->ticker_last_price == 0 && ld->trade->ticker_close_price == 0) {

				std::wstring ticker_symbol = ld->trade->ticker_symbol;
				if (ticker_symbol == L"SPX") ticker_symbol = L"^SPX";
				if (ticker_symbol == L"NDX") ticker_symbol = L"^NDX";
				if (ticker_symbol == L"RUT") ticker_symbol = L"^RUT";
				if (ticker_symbol == L"VIX") ticker_symbol = L"^VIX";
				if (config.IsFuturesTicker(ticker_symbol)) {
					ticker_symbol = ticker_symbol.substr(1);
					if (ticker_symbol == L"AUD") ticker_symbol = L"6A";
					if (ticker_symbol == L"GBP") ticker_symbol = L"6B";
					if (ticker_symbol == L"EUR") ticker_symbol = L"6E";
					if (ticker_symbol == L"CAD") ticker_symbol = L"CD";
					if (ticker_symbol == L"JPY") ticker_symbol = L"JY";
					if (ticker_symbol == L"CHF") ticker_symbol = L"SF";
					if (ticker_symbol == L"INR") ticker_symbol = L"IR";
					if (ticker_symbol == L"MCL") ticker_symbol = L"CL";
					if (ticker_symbol == L"MES") ticker_symbol = L"ES";
					if (ticker_symbol == L"MNQ") ticker_symbol = L"NQ";
					if (ticker_symbol == L"M2K") ticker_symbol = L"2K";
					ticker_symbol = ticker_symbol + L"=F";
				}

				// Lookup our map to see if we have already retrieved the price from a previous scrap. If
				// yes, then use that value rather than doing yet another scrap.
				if (map_prices.count(ticker_symbol)) {
					ld->trade->ticker_close_price = map_prices.at(ticker_symbol);
				}
				else {
					ld->trade->ticker_close_price = GetScrapedClosingPrice(ticker_symbol);
					map_prices[ticker_symbol] = ld->trade->ticker_close_price;
				}
				ld->trade->ticker_last_price = ld->trade->ticker_close_price;

				std::wstring text = AfxMoney(ld->trade->ticker_last_price, false, ld->trade->ticker_decimals);
				ld->trade->ticker_last_price_text = text; 
				ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, text, COLOR_WHITELIGHT);  // current price

				ActiveTrades.PerformITMcalculation(ld->trade);
				ld->SetTextData(COLUMN_TICKER_ITM, ld->trade->itm_text, ld->trade->itm_color);  // ITM

				text = AfxMoney(ld->trade->ticker_close_price, false, ld->trade->ticker_decimals);
				ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, text, COLOR_WHITELIGHT);  // current price

				RECT rc{};
				ListBox_GetItemRect(hListBox, index, &rc);
				InvalidateRect(hListBox, &rc, true);
				UpdateWindow(hListBox);
			}
		}
	}

	// Do calculation to ensure column widths are wide enough to accommodate the new
	// price data that has just arrived.
	if (ListBoxData_ResizeColumnWidths(hListBox, TableType::active_trades)) {
		AfxRedrawWindow(hListBox);
	}

	SetCursor(LoadCursor(0, IDC_ARROW));
}


//
// Thread functions
//
void PingFunction(std::stop_token st) {
	std::cout << "Starting the ping thread" << std::endl;

	is_ping_thread_active = true;

	while (!st.stop_requested()) {

		// Sleep for 5 seconds
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		try {
			if (is_monitor_thread_active == true) {
				// Send a request time message (ping)
				client.PingTWS();
			}
			else {
				break;
			}
		}
		catch (...) {
			break;
		}
	}

	std::cout << "Ping Thread Terminated" << std::endl;
}


void TickerUpdateFunction(std::stop_token st) {
	std::cout << "Starting the TickerUpdate thread" << std::endl;

	is_ticker_update_thread_active = true;

	while (!st.stop_requested()) {

		// Sleep for 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		try {
			if (is_monitor_thread_active == true) {
				ActiveTrades.UpdateTickerPrices();
			}
			else {
				break;
			}
		}
		catch (...) {
			break;
		}
	}

	std::cout << "TickerUpdate Thread Terminated" << std::endl;
}


void MonitoringFunction(std::stop_token st) {
	std::cout << "Starting the monitoring thread" << std::endl;
	
	is_monitor_thread_active = true;

	while (!st.stop_requested()) {

		try {
			if (tws_IsConnected()) {
				client.WaitForSignal();
				client.ProcessMsgs();
			}
			else {
				break;
			}
		}
		catch (...) {
			PostMessage(HWND_TABPANEL, MSG_TWS_WARNING_EXCEPTION, 0, 0);
			break;
		}
	}

	is_monitor_thread_active = false;

	// Request to shut down the TickerUpdate thread
	ticker_update_thread.request_stop();

	// Request to shut down the Ping thread also
	ping_thread.request_stop();

	std::cout << "Requesting TickerUpdate Thread to Terminate" << std::endl;
	std::cout << "Requesting Ping Thread to Terminate" << std::endl;
	std::cout << "Monitoring Thread Terminated" << std::endl;
	PostMessage(HWND_TABPANEL, MSG_TWS_CONNECT_DISCONNECT, 0, 0);
}


void tws_StartMonitorThread() {
	if (is_monitor_thread_active) return;
	monitoring_thread = std::jthread(MonitoringFunction);
}


void tws_EndMonitorThread() {
	if (is_monitor_thread_active) {
		std::cout << "Monitoring thread will be stopped soon...." << std::endl;
		monitoring_thread.request_stop();
	}
}

void tws_StartTickerUpdateThread() {
	if (is_monitor_thread_active) return;
	ticker_update_thread = std::jthread(TickerUpdateFunction);
}

void tws_EndTickerUpdateThread() {
	if (is_ticker_update_thread_active) {
		std::cout << "TickerUpdate thread will be stopped soon...." << std::endl;
		ticker_update_thread.request_stop();
	}
}

void tws_StartPingThread() {
	if (is_ping_thread_active) return;
	ping_thread = std::jthread(PingFunction);
}


void tws_ConnectionSuccessful() {
	// This function is called when TWS has successfully connected and has
	// sent the nextValidId callback. This signals that the connection is
	// ready to start to receive messages from us, etc.
	// This function is called from nextValidId();
	if (tws_IsConnected()) {
		// Load all local positions in vector
		Reconcile_LoadAllLocalPositions();

		// Request Account Summary in order to get liquidity amounts
		tws_RequestAccountSummary();

		ActiveTrades.ShowActiveTrades();
	}
}


bool tws_Connect() {
    if (tws_IsConnected()) return false;

    const char* host = "";

	int port = 0;
	if (client.connection_type == ConnectionType::tws_data_live) port = 7496;
	if (client.connection_type == ConnectionType::tws_data_paper) port = 7497;
	
	SendMessage(HWND_TABPANEL, MSG_TWS_CONNECT_START, 0, 0);

	bool res = false;

	try {

		if (client.had_previous_socket_exception) {
			// Try to recover from a previous socket exception when client disconnected.
			// The client class elements below were changed from Private to Public and a const
			// removed so that the elements could be deleted and re-instantiated.
			
			// destroy the reader before the client
			if (client.m_pReader)
				client.m_pReader.reset();

			delete client.m_pClient;

			client.m_pClient = new EClientSocket(&client, &client.m_osSignal);
		}

		res = client.Connect(host, port, client.client_id);
		
		is_monitor_thread_active = false;
		is_ticker_update_thread_active = false;
		is_ping_thread_active = false;

		if (res) {
			// Start thread that will start messaging polling
			// and poll if TWS remains connected. Also start thread
			// that updates the ActiveTrades list every defined interval.
			SendMessage(HWND_TABPANEL, MSG_TWS_CONNECT_SUCCESS, 0, 0);

			// Display message if Paper Trading is enabled.
			if (client.connection_type == ConnectionType::tws_data_paper) {
				MainWindow.DisplayPaperTradingWarning();
			}

			tws_StartMonitorThread();
			tws_StartTickerUpdateThread();
			tws_StartPingThread();
			client.had_previous_socket_exception = false;
		}

	}
	catch (...) {
		SendMessage(HWND_TABPANEL, MSG_TWS_CONNECT_FAILURE, 0, 0);
		std::wstring text =
			L"Socket exception error trying to connect to TWS.\n\nPlease try to connect again or restart the application if the problem persists.";
		CustomMessageBox.Show(MainWindow.hWindow, text, L"Connection Failed", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	
	if (res == false) {
        //SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_FAILURE, 0, 0);
		std::wstring text =
			L"Could not connect to TWS.\n\n" \
			"Confirm in TWS, File->Global Configuration->API->Settings menu that 'Enable ActiveX and Client Sockets' is enabled and connection port is set to 7496. (Paper Trading connection port is 7497).\n";
		
		CustomMessageBox.Show(MainWindow.hWindow, text, L"Connection Failed", MB_OK | MB_ICONEXCLAMATION);
		return false;
    }

    return res;
}


bool tws_Disconnect() {
    if (tws_IsConnected() == false) return true;

    client.Disconnect();
    tws_EndMonitorThread();

    return (tws_IsConnected(), false, true);
}


bool tws_IsConnected() {
	bool res = false;
    if ((client.IsSocketOK()) && (client.IsConnected())) {
        res = true;
    }
    return res;
}


void tws_CancelMarketData(TickerId ticker_id) {
	if (!tws_IsConnected()) return;
	if (ticker_id < 1) return;
	client.CancelMarketData(ticker_id);
}


void tws_RequestMarketUpdates() {
	ListBoxData_RequestMarketData(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_LISTBOX));
}


void tws_RequestMarketData(ListBoxData* ld) {
	if (!tws_IsConnected()) return;
	if (!ld->trade) return;
	client.RequestMarketData(ld);
}


void tws_RequestAccountSummary() {
	if (!tws_IsConnected()) return;
	client.RequestAccountSummary();
}


void tws_CancelPositions() {
	if (!tws_IsConnected()) return;
	client.CancelPositions();
}


void tws_RequestPositions() {
	if (!tws_IsConnected()) return;
	client.RequestPositions();
}

void tws_RequestWshMetaData(int req_id) {
	if (!tws_IsConnected()) return;
	client.RequestWshMetaData(req_id);
}

void tws_RequestWshEventData(int req_id, const WshEventData& wshEventData)
{
	if (!tws_IsConnected()) return;
	client.RequestWshEventData(req_id, wshEventData);
}


void tws_PerformReconciliation() {
	if (!tws_IsConnected()) {
		CustomMessageBox.Show(
			MainWindow.hWindow,
			L"Must be connected to TWS to perform a reconciliation.",
			L"Error",
			MB_ICONINFORMATION);
		return;
	}

	// Prevent Reconcile from running prior to previous invocation completing
	static bool is_running = false;
	if (is_running == true) return;

	is_running = true;

	// Show the results
	Reconcile_Show();

	is_running = false;
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
TwsClient::~TwsClient() {
	// destroy the reader before the client
	if (m_pReader)
		m_pReader.reset();

	delete m_pClient;
}

bool TwsClient::Connect(const char* host, int port, int clientId) {
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

void TwsClient::PingTWS() const {
	m_pClient->reqCurrentTime();
	// printf("ping\n");
}

void TwsClient::Disconnect() const {
	m_pClient->eDisconnect();

	printf("Disconnected\n");
}

bool TwsClient::IsConnected() const {
	return m_pClient->isConnected();
}

bool TwsClient::IsSocketOK() const {
	return m_pClient->isSocketOK();
}

void TwsClient::WaitForSignal() {
	m_osSignal.waitForSignal();
}

void TwsClient::ProcessMsgs() {
	m_pReader->processMsgs();
}

void TwsClient::CancelPortfolioUpdates() {
	m_pClient->reqAccountUpdates(false, "");
}

void TwsClient::RequestPortfolioUpdates() {
	m_pClient->reqAccountUpdates(true, "");
}

void TwsClient::CancelMarketData(TickerId ticker_id) {
	m_pClient->cancelMktData(ticker_id);
}


void TwsClient::RequestMarketData(ListBoxData* ld) {
	// If the ticker_id has already been previously requested then no need to request it again.
	if (ld->trade->ticker_data_requested) return;

	// Convert the unicode symbol to regular string type
	std::string symbol = unicode2ansi(ld->trade->ticker_symbol);

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
			contract.lastTradeDateOrContractMonth = AfxFormatFuturesDateMarketData(ld->trade->future_expiry);   // YYYYMMDD

			std::string futExchange = config.GetFuturesExchange(symbol);
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

	ld->trade->ticker_data_requested = true;
	m_pClient->reqMktData(ld->ticker_id, contract, "", false, false, TagValueListSPtr());
}


void TwsClient::CancelPositions() {
	m_pClient->cancelPositions();
}

void TwsClient::RequestPositions() {
	m_pClient->reqPositions();
}

void TwsClient::RequestWshMetaData(int req_id) {
	m_pClient->reqWshMetaData(req_id);
}

void TwsClient::RequestWshEventData(int req_id, const WshEventData& wshEventData) {
	m_pClient->reqWshEventData(req_id, wshEventData);
}

void TwsClient::RequestAccountSummary() {
	m_pClient->reqAccountSummary(1000, "All", "NetLiquidation,ExcessLiquidity");
}



//////////////////////////////////////////////////////////////////
// methods
void TwsClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
		m_pClient->startApi();
}


void TwsClient::tickGeneric(TickerId ticker_id, TickType tickType, double value) {
	// Handle any HALTED tickers (Halted notifications only arrive via tickGeneric).
	//Value	Description
	//	- 1	Halted status not available.Usually returned with frozen data.
	//	0	Not halted.This value will only be returned if the contract is in a TWS watchlist.
	//	1	General halt.Trading halt is imposed for purely regulatory reasons with / without volatility halt.
	//	2	Volatility halt.Trading halt is imposed by the exchange to protect against extreme volatility.

	if (tickType == HALTED && value == 1 || value == 2) {  // 49
		HWND hListBox = GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_LISTBOX);
		int item_count = ListBox_GetCount(hListBox);
		if (item_count == 0) return;

		for (int index = 0; index < item_count; index++) {
			ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, index);
			if (ld == nullptr) continue;

			if ((ld->ticker_id == ticker_id) && (ld->trade != nullptr) && (ld->line_type == LineType::ticker_line)) {
				ld->SetTextData(COLUMN_TICKER_CHANGE, L"HALTED", COLOR_RED);
				if (ListBoxData_ResizeColumnWidths(hListBox, TableType::active_trades)) {
					AfxRedrawWindow(hListBox);
				}
				break;
			}
		}
	}
}


void TwsClient::tickByTickAllLast(int reqId, int tickType, time_t time, 
	double price, Decimal size, const TickAttribLast& tickAttribLast, 
	const std::string& exchange, const std::string& specialConditions) 
{
	//printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %s, Size: %s, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n",
	//	reqId, (tickType == 1 ? "Last" : "AllLast"), ctime(&time), Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str(), tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
}


void TwsClient::tickPrice(TickerId ticker_id, TickType field, double price, const TickAttrib& attribs) {

	if (price == -1) return;   // no data currently available

	// Market data tick price callback. Handles all price related ticks. Every tickPrice callback is followed 
	// by a tickSize. A tickPrice value of - 1 or 0 followed by a tickSize of 0 indicates there is no data for 
	// this field currently available, whereas a tickPrice with a Green tickSize indicates an active 
	// quote of 0 (typically for a combo contract).

	// Parameters
	// ticker_id	the request's unique identifier.
	// field	the type of the price being received(i.e.ask price)(TickType)
	// price	the actual price.
	// attribs	an TickAttrib object that contains price attributes such as TickAttrib::CanAutoExecute, TickAttrib::PastLimit and TickAttrib::PreOpen.

	// Most pertinent TickType fields for our use would be the following:
	// enum TickType { LAST, CLOSE, OPEN }
	// Just dealing with these 3 fields cuts out a **LOT** of tickPrice notifications.
	if (field == LAST || field == OPEN || field == CLOSE) {

		//if (field == LAST) std::cout << "tickPrice LAST " << ticker_id << " " << price << std::endl;
		//if (field == OPEN) std::cout << "tickPrice OPEN " << ticker_id << " " << price << std::endl;
		//if (field == CLOSE) std::cout << "tickPrice CLOSE " << ticker_id << " " << price << std::endl;

		TickerData td{};

		if (mapTickerData.count(ticker_id)) {
			td = mapTickerData.at(ticker_id);
		}

		if (field == OPEN) {
			td.open_price = price;
			if (td.close_price == 0) td.close_price = price;
		}
		if (field == CLOSE) td.close_price = price;
		if (field == LAST) td.last_price = price;

		mapTickerData[ticker_id] = td;
	}  
}


void TwsClient::updatePortfolio(const Contract& contract, Decimal position,
	double market_price, double market_value, double average_cost,
	double unrealized_PNL, double realized_PNL, const std::string& account_name) 
{
	//printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: %s, AccountName: %s\n",
	//	(contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), decimalStringToDisplay(position).c_str(),
	//	Utils::doubleMaxString(market_price).c_str(), Utils::doubleMaxString(market_value).c_str(), Utils::doubleMaxString(average_cost).c_str(),
	//	Utils::doubleMaxString(unrealized_PNL).c_str(), Utils::doubleMaxString(realized_PNL).c_str(), account_name.c_str());

	PortfolioData pd{};

	if (mapPortfolioData.count(contract.conId)) {
		pd = mapPortfolioData.at(contract.conId);
	}

	pd.average_cost = average_cost;
	pd.market_price = market_price;

	mapPortfolioData[contract.conId] = pd;
}


void TwsClient::connectionClosed() {
	printf("Connection Closed\n");
	// TWS must have shut down while our application was still running.
	had_previous_socket_exception = true;
}

void TwsClient::error(int id, int error_code, 
	const std::string& error_string, const std::string& advanced_order_reject_json)
{
	switch (error_code) {
	case 509:  // socket exception error
		if (id == -1) {   

			printf("Error. Id: %d, Code: %d, Msg: %s\n", id, error_code, error_string.c_str());
			
			// Ensure that threads that I created get terminated.
			Disconnect();

			// Set flag so that the class socket and reader can be recreated if the user attempts
			// to re-connect again. See tws_connect() function for that code.
			had_previous_socket_exception = true;
			return;
		}
	}

	switch (error_code) {
	case 2104:
	case 2106:
	case 2158:
		return;
	}
	printf("Error. Id: %d, Code: %d, Msg: %s\n", id, error_code, error_string.c_str());

	// If error codes 10091 or 10089 then we are connected most likely after hours and we do not have
	// access to streaming data. In this case we will attempt scrap for the closing price.
	switch (error_code) {
	case 10089:
	case 10091:
		market_data_subscription_error = true;
		return;
	}

	switch (error_code) {
	case 1100:   // 'Connectivity between IB and Trader Workstation has been lost.'
	{
		// This normally occurs when the internet connection is lost. It refers to the 
		// connection between TWS and IBKR, and not TWS and TradeTracker. The connection
		// between TradeTracker and TWS will resume as soon as TWS reconnects back to the
		// IBKR servers (ie. TradeTrackers sockets remain open).
		//SendMessage(HWND_SIDEMENU, MSG_TWS_CONNECT_WAIT_RECONNECTION, 0, 0);
		std::wstring text =
			L"TWS has lost connection to the IBKR servers (Internet connection down?).\n\nTradeTracker will resume automatically when TWS reconnects to IBKR.";
		CustomMessageBox.Show(MainWindow.hWindow, text, L"Connection Failed", MB_OK | MB_ICONEXCLAMATION);
	}
	break;

	case 1102:   // 'Connectivity between IB and Trader Workstation reestablished.'
	{
		SendMessage(HWND_TABPANEL, MSG_TWS_CONNECT_SUCCESS, 0, 0);
	}
	break;
	
	}
}

static bool positionEnd_fired = false;

void TwsClient::position(const std::string& account, const Contract& contract, Decimal position, double avg_cost) {
	// This callback is initiated by the reqPositions().
	Reconcile_position(contract, position);
	
	if (positionEnd_fired) {
		Reconcile_doPositionMatching();
	}
}


void TwsClient::positionEnd() {
	if (!positionEnd_fired) {
		Reconcile_doPositionMatching();
		positionEnd_fired = true;
	}

	// Send notification to ActiveTrades window that positions have all been loaded
	// thereby allowing the loading of portfolio values.
	SendMessage(ActiveTrades.hWindow, MSG_POSITIONS_READY, 0, 0);

	// We have finished requesting positions. It is possible that some position closing prices were 
	// not retrieved because maybe we are connected but it is after hours and we need additional
	// subscriptions to access the data.
	// Check global variable for the market subscription error and then attempt scraping from yahoo finance.
	if (market_data_subscription_error) {
		tws_UpdateTickersWithScrapedData();
		market_data_subscription_error = false;
	}
}


void TwsClient::updateAccountValue(const std::string& key, const std::string& val,
	const std::string& currency, const std::string& accountName) 
{
	// This callback will fire when PortfolioValue updates are initiated. We look for the
	// string "AccountReady". 
	// Note: An important key passed back in IBApi.EWrapper.updateAccountValue after a call 
	// to IBApi.EClient.reqAccountUpdates is a boolean value 'AccountReady'. If an AccountReady 
	// value of false is returned that means that the IB server is in the process of resetting 
	// at that moment, i.e. the account is 'not ready'. When this occurs subsequent key values 
	// returned to IBApi::EWrapper::updateAccountValue in the current update can be out of date or incorrect.
	//if (key == "AccountReady" && val == "true") {
	//	std::cout << p << "  updateAccountValue.  key: " << key << "  value: " << val << std::endl;
	//}
}


void TwsClient::updateAccountTime(const std::string& timeStamp) {
	// printf("****** UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}


void TwsClient::accountSummary(int reqId, const std::string& account, 
	const std::string& tag, const std::string& value, const std::string& currency) 
{
	// Values will return immediately when first requested and then everytime they change or 3 minutes.

	//std::cout << "Account Summary  reqId: " << reqId << "  account: " << account << "  tag: " << tag << "  value: " << value << "  currency: " << currency << std::endl;

	std::wstring account_value = ansi2unicode(value);

	// Get the value and convert it into Million(M) or Thousands(K) amounts
	double amount = AfxValDouble(account_value);
	if (amount > 1000000) {
		amount = amount / 1000000;
		account_value = AfxMoney(amount, false, 2) + L"M";
	}
	else if (amount > 1000) {
		amount = amount / 1000;
		account_value = AfxMoney(amount, false, 1) + L"K";
	}
	else {
		account_value = AfxMoney(amount, false);
	}

	int nShow = (config.GetAllowPortfolioDisplay()) ? SW_SHOW : SW_HIDE;

	if (AfxStringCompareI(tag, "NetLiquidation")) {
		CustomLabel_SetText(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_NETLIQUIDATION_VALUE), account_value);
		ShowWindow(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE), nShow);
	}

	if (AfxStringCompareI(tag, "ExcessLiquidity")) {
		CustomLabel_SetText(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE), account_value);
		ShowWindow(GetDlgItem(ActiveTrades.hWindow, IDC_ACTIVETRADES_EXCESSLIQUIDITY_VALUE), nShow);
	}
}


void TwsClient::wshMetaData(int reqId, const std::string& dataJson) {
	printf("WSH Meta Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}


void TwsClient::wshEventData(int reqId, const std::string& dataJson) {
	printf("WSH Event Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}


void TwsClient::accountSummaryEnd(int reqId) {
	// std::cout << "account summary end" << std::endl;
}

void TwsClient::tickSize(TickerId ticker_id, TickType field, Decimal size) {
	//std::cout << "tickSize  id: " << ticker_id << "  size: " << (int)intelDecimalToDouble(size) << std::endl;
}

void TwsClient::tickString(TickerId ticker_id, TickType tickType, const std::string& value) {
	// printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", ticker_id, (int)tickType, value.c_str());
}

void TwsClient::winError(const std::string& str, int lastError) {}

void TwsClient::nextValidId(OrderId orderId) {
	// We have made a successful connection to start our threads and load market data.
	//std::cout << "orderId: " << orderId << std::endl;
	tws_ConnectionSuccessful();
}

void TwsClient::currentTime(long time) {
	// called from ping
	// std::cout << "current time " << time << std::endl;
}

