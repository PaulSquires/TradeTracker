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

#include "tws-api/linux/EWrapper.h"
#include <string>
#include <thread>
#include <chrono>

#include <iostream>

#ifdef __WXMSW__
	#include "tws-api/windows/EClientSocket.h"
	#include "tws-api/windows/CommonDefs.h"
#else
	#include "tws-api/linux/EClientSocket.h"
	#include "tws-api/linux/CommonDefs.h"
#endif

#include "tws-client.h"

#include "appstate.h"
#include "utilities.h"
#include "active_trades_actions.h"
#include "import_dialog.h"
#include "reconcile.h"
#include "messagebox.h"


// Unfortunately the following data structures have to
// be made global because the data needs to be updated in the TwsClient::tickPrice
// and TwsClient::updatePortfolio functions which are callbacks from the
// Interactive Brokers library (therefore I can't pass AppState into it).
std::unordered_map<TickerId, TickerData> mapTickerData;
std::unordered_map<int, PortfolioData> mapPortfolioData;
bool market_data_subscription_error = false;
bool positionEnd_fired = false;
bool is_connection_ready_for_data = false;
bool is_positions_ready_for_data = false;
double netliq_value = 0;
double excessliq_value = 0;
double maintenance_value = 0;


//
// Thread functions
//
void TickerUpdateFunction(AppState* state) {
	std::cout << "Starting the TickerUpdate thread" << std::endl;

	state->is_ticker_update_thread_active = true;

    TwsClient* client = static_cast<TwsClient*>(state->client);

	while (!state->stop_ticker_update_thread_requested) {

		// Sleep for 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (state->is_monitor_thread_active == true && client->IsConnected()) {
			UpdateTickerPrices(*state);
		}
	}

	std::cout << "TickerUpdate Thread Terminated" << std::endl;
}


void MonitoringFunction(AppState* state) {
	std::cout << "Starting the monitoring thread" << std::endl;

	state->is_monitor_thread_active = true;

    TwsClient* client = static_cast<TwsClient*>(state->client);

	while (!state->stop_monitor_thread_requested) {
		if (state->is_monitor_thread_active) {
			if (tws_IsConnected(*state)) {
				client->WaitForSignal();
			}
			if (tws_IsConnected(*state)) {
				client->ProcessMsgs();
			}
		}
	}

	state->is_monitor_thread_active = false;

	// Request to shut down the TickerUpdate thread
	std::cout << "Requesting TickerUpdate Thread to Terminate" << std::endl;
	state->stop_ticker_update_thread_requested = true;

	std::cout << "Monitoring Thread Terminated" << std::endl;
}


#if defined(_WIN32) // win32 and win64
#include <filesystem>
#include <fstream>
#include <wininet.h>
#pragma comment(lib,"Wininet.lib")
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

std::string fetchVersionFromWebsite(const std::string& url) {
	std::string curl_command = "C:/Windows/System32/curl.exe";
	bool curl_exists = AfxFileExists(curl_command);

	// Do a simple check to see if connected to internet. If fail then simply
	// advise the user via a messgebox.
	std::wstring wurl = L"https://www.google.com";
	bool is_internet_available = InternetCheckConnection(wurl.c_str(), FLAG_ICC_FORCE_CONNECTION, 0);

	std::string version_available = "";

	if (!is_internet_available) {
		std::cout << "Update Check failed: No internet connection." << std::endl;
	}
	else if (!curl_exists) {
		std::cout << "Update Check failed: Curl does not exist." << std::endl;
	}
	else {
		std::string local_file = GetDataFilesFolder() + "/_versioncheck.txt";
		std::string cmd = "C:/Windows/System32/curl.exe -o " +
			local_file + " " + url;
		std::string text = AfxExecCmd(cmd);

		if (AfxFileExists(local_file)) {
			std::ifstream db;
			db.open(local_file, std::ios::in);
			if (db.is_open()) {

				while (!db.eof()) {
					std::string line;
					std::getline(db, line);

					line = AfxTrim(line);

					if (line.length() == 0) continue;

					if (line.length()) {
						version_available = line;
						break;
					}
				}

			}
			db.close();
		}
		std::filesystem::path filename = local_file;
		std::filesystem::remove(filename);
	}
	return version_available;
}

#else  // Linux & Apple using static library
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

std::string fetchVersionFromWebsite(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error fetching version: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return AfxTrim(readBuffer);
}
#endif


void CheckForUpdateFunction(AppState* state) {
	std::cout << "Starting the check for update thread" << std::endl;

	state->is_checkforupdate_thread_active = true;

    TwsClient* client = static_cast<TwsClient*>(state->client);
    state->version_available_display = fetchVersionFromWebsite("https://planetsquires.com/tradetracker_version.txt");

	state->is_checkforupdate_thread_active = false;
}


void tws_StartCheckForUpdateThread(AppState& state) {
	if (state.is_checkforupdate_thread_active) return;
	AppState* ptr = &state;  // Convert reference to pointer
	state.check_for_update_thread = std::thread(CheckForUpdateFunction, ptr);
}


void tws_StartMonitorThread(AppState& state) {
	if (state.is_monitor_thread_active) return;
	AppState* ptr = &state;  // Convert reference to pointer
	state.monitoring_thread = std::thread(MonitoringFunction, ptr);
}


void tws_EndMonitorThread(AppState& state) {
	if (state.is_monitor_thread_active) {
		state.is_monitor_thread_active = false;
		std::cout << "Monitoring thread will be stopped soon...." << std::endl;
		state.stop_monitor_thread_requested = true;
	}
}

void tws_StartTickerUpdateThread(AppState& state) {
	if (state.is_monitor_thread_active) return;
	AppState* ptr = &state;  // Convert reference to pointer
	state.ticker_update_thread = std::thread(TickerUpdateFunction, ptr);
}

void tws_EndTickerUpdateThread(AppState& state) {
	if (state.is_ticker_update_thread_active) {
		std::cout << "TickerUpdate thread will be stopped soon...." << std::endl;
		state.stop_ticker_update_thread_requested = true;
	}
}

bool tws_Connect(AppState& state) {
    if (tws_IsConnected(state)) return false;

    const char* host = "";

	int port = 7496;

	// If paper trading is enabled via command line then use different port
	if (state.is_paper_trading) port = 7497;

	bool res = false;
    TwsClient* client = static_cast<TwsClient*>(state.client);

	try {

		if (client->had_previous_socket_exception) {
			// Try to recover from a previous socket exception when client disconnected.
			// The client class elements below were changed from Private to Public and a const
			// removed so that the elements could be deleted and re-instantiated.

			// destroy the reader before the client
			if (client->m_pReader)
				client->m_pReader.reset();

			delete client->m_pClient;

			client->m_pClient = new EClientSocket(client, &client->m_osSignal);
		}

		res = client->Connect(host, port, client->client_id);

		state.is_monitor_thread_active = false;
		state.is_ticker_update_thread_active = false;

		if (res) {
			// Start thread that will start messaging polling
			// and poll if TWS remains connected. Also start thread
			// that updates the ActiveTrades list every defined interval.
			tws_StartMonitorThread(state);
			tws_StartTickerUpdateThread(state);
			tws_StartCheckForUpdateThread(state);
			client->had_previous_socket_exception = false;
		}

	}
	catch (...) {
		std::string text =
		 	"Socket exception error trying to connect to TWS.\n\nPlease try to connect again or restart the application if the problem persists.";
		CustomMessageBox(state, "Connection Failed", text);
		return false;
	}

	if (res == false) {
		std::string text =
		 	"Could not connect to TWS.\n\n" \
		 	"Confirm in TWS, File->Global Configuration->API->Settings menu that 'Enable ActiveX and Client Sockets' is enabled and connection port is set to 7496 for live trading or port 7497 for paper trading.\n\n" \
		 	"Socket exception error trying to connect to TWS.\n\nPlease try to connect again or restart the application if the problem persists.";
		CustomMessageBox(state, "Connection Failed", text);
		return false;
    }

    return res;
}


bool tws_Disconnect(AppState& state) {
    if (tws_IsConnected(state) == false) return true;

    TwsClient* client = static_cast<TwsClient*>(state.client);
    client->Disconnect();
    tws_EndMonitorThread(state);

    state.monitoring_thread.join();
    state.ticker_update_thread.join();
    state.check_for_update_thread.join();

    return (tws_IsConnected(state) ? false : true);
}


bool tws_IsConnected(AppState& state) {
	bool res = false;
    TwsClient* client = static_cast<TwsClient*>(state.client);
    if (!client) return false;
    if ((client->IsSocketOK()) && (client->IsConnected())) {
        res = true;
    }
    return res;
}


void tws_CancelMarketData(AppState& state, TickerId ticker_id) {
	if (!tws_IsConnected(state)) return;
	if (ticker_id < 1) return;
    TwsClient* client = static_cast<TwsClient*>(state.client);
	client->CancelMarketData(ticker_id);
}


void tws_RequestAccountSummary(AppState& state) {
	if (!tws_IsConnected(state)) return;
    TwsClient* client = static_cast<TwsClient*>(state.client);
	client->RequestAccountSummary();
}


void tws_CancelPositions(AppState& state) {
	if (!tws_IsConnected(state)) return;
    TwsClient* client = static_cast<TwsClient*>(state.client);
	client->CancelPositions();
}


void tws_RequestPositions(AppState& state) {
	if (!tws_IsConnected(state)) return;
    TwsClient* client = static_cast<TwsClient*>(state.client);
	client->RequestPositions();
}


///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]    TwsClient* client = static_cast<TwsClient*>(state.client);

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

void TwsClient::RequestMarketData(AppState& state, CListPanelData* ld) {
	// If the ticker_id has already been previously requested then no need to request it again.
	TickerId ticker_id = -1;
	if (ld->line_type == LineType::ticker_line) ticker_id = ld->trade->ticker_id;
	if (ld->line_type == LineType::options_leg) ticker_id = ld->leg->ticker_id;
	if (ticker_id == -1) return;

	// Convert the unicode symbol to regular string type
	std::string symbol = ld->trade->ticker_symbol;

	//	struct Contract;
	Contract contract;

	bool is_option_position = (ld->line_type == LineType::options_leg);

	if (state.config.IsIndexTicker(symbol)) {
		contract.symbol = symbol;
		contract.secType = (is_option_position) ? "OPT" : "IND";
		contract.currency = "USD";
		contract.exchange = "SMART";           // Use IB's SmartRouting
		contract.primaryExchange = "CBOE";     // disambiguate the listing exchange when multiple securities have the same symbol.
		contract.lastTradeDateOrContractMonth = (is_option_position) ? AfxRemoveDateHyphens(ld->leg->expiry_date) : "";
	}
	else {
		if (state.config.IsFuturesTicker(symbol)) {
			contract.symbol = symbol.substr(1);
			contract.secType =  (is_option_position) ? "FOP" : "FUT";
			contract.currency = "USD";

			contract.lastTradeDateOrContractMonth = AfxFormatFuturesDateMarketData(ld->trade->future_expiry);

			std::string futExchange = state.config.GetFuturesExchange(symbol);
			if (futExchange.length() == 0) futExchange = "CME";

			contract.exchange = futExchange;
			contract.primaryExchange = futExchange;
		}
		else {
			contract.symbol = symbol;
			contract.secType = (is_option_position) ? "OPT" : "STK";
			contract.currency = "USD";
			contract.exchange = "SMART";
			contract.primaryExchange = "CBOE";
			contract.lastTradeDateOrContractMonth = (is_option_position) ? AfxRemoveDateHyphens(ld->leg->expiry_date) : "";
		}
	}

	if (is_option_position) {
		contract.conId = ld->leg->contract_id;
		contract.multiplier = std::to_string(ld->trade->multiplier);
		contract.strike = AfxValDouble(ld->leg->strike_price);
		contract.right = state.db.PutCallToString(ld->leg->put_call);
	}

	m_pClient->reqMktData(ticker_id, contract, "", false, false, TagValueListSPtr());
}


void TwsClient::CancelPositions() {
	m_pClient->cancelPositions();
}

void TwsClient::RequestPositions() {
	m_pClient->reqPositions();
}

void TwsClient::RequestAccountSummary() {
	m_pClient->reqAccountSummary(1000, "All", "NetLiquidation,ExcessLiquidity,MaintMarginReq ");
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

		// if (field == LAST) std::cout << "tickPrice LAST " << ticker_id << " " << price << std::endl;
		// if (field == OPEN) std::cout << "tickPrice OPEN " << ticker_id << " " << price << std::endl;
		// if (field == CLOSE) std::cout << "tickPrice CLOSE " << ticker_id << " " << price << std::endl;

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
	PortfolioData pd{};

	if (mapPortfolioData.count(contract.conId)) {
		pd = mapPortfolioData.at(contract.conId);
	}

	pd.position = position;
	pd.market_price = market_price;
	pd.market_value = market_value;
	pd.average_cost = average_cost;
	pd.unrealized_pnl = unrealized_PNL;
	pd.realized_pnl = realized_PNL;

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
		// std::string text =
		// 	"TWS has lost connection to the IBKR servers (Internet connection down?).\n\nTradeTracker will resume automatically when TWS reconnects to IBKR.\n\n";
		// CustomMessageBox(state, "Connection Failed", text);
	}
	break;

	case 1102:   // 'Connectivity between IB and Trader Workstation reestablished.'
	{
//		SendMessage(HWND_TABPANEL, MSG_TWS_CONNECT_SUCCESS, 0, 0);
	}
	break;

	}
}


void TwsClient::position(const std::string& account, const Contract& contract, Decimal position, double avg_cost) {

std::cout << "ImportTrades_position" << std::endl;

	// This callback is initiated by the reqPositions().
	Reconcile_position(contract, position, avg_cost);

	ImportTrades_position(contract, position, avg_cost);

 	if (positionEnd_fired) {
 		Reconcile_doPositionMatching();
 	}
}


void TwsClient::positionEnd() {
	Reconcile_doPositionMatching();
	positionEnd_fired = true;

	// Send notification to ActiveTrades window that positions have all been loaded
	// thereby allowing the loading of portfolio values.
	is_positions_ready_for_data = true;
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
	const std::string& tag, const std::string& value, const std::string& currency) {
	// Values will return immediately when first requested and then everytime they change or 3 minutes.

    double amount = AfxValDouble(value);

    if (tag == "NetLiquidation")  netliq_value = amount;
	if (tag == "ExcessLiquidity") excessliq_value = amount;
	if (tag == "MaintMarginReq")  maintenance_value = amount;
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
	is_connection_ready_for_data = true;
}

void TwsClient::currentTime(long time) {
	// called from ping
	// std::cout << "current time " << time << std::endl;
}

void TwsClient::tickOptionComputation(TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
                           double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice) {
	if (mapTickerData.count(tickerId)) {
		if (delta > -1.0f && delta < 1.0f) {
			mapTickerData.at(tickerId).delta = delta;
		}
	}
}


//void TwsClient::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) { }
//void TwsClient::tickSize( TickerId tickerId, TickType field, Decimal size) { }
// void TwsClient::tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
// 	   double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice) {}
//void TwsClient::tickGeneric(TickerId tickerId, TickType tickType, double value) { }
//void TwsClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) { }
void TwsClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
	   double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) { }
void TwsClient::orderStatus( OrderId orderId, const std::string& status, Decimal filled,
	   Decimal remaining, double avgFillPrice, int permId, int parentId,
	   double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) { }
void TwsClient::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&) { }
void TwsClient::openOrderEnd() { }
//void TwsClient::winError( const std::string& str, int lastError) { }
//void TwsClient::connectionClosed() { }
//void TwsClient::updateAccountValue(const std::string& key, const std::string& val,
//   const std::string& currency, const std::string& accountName) { }
//void TwsClient::updatePortfolio( const Contract& contract, Decimal position,
//      double marketPrice, double marketValue, double averageCost,
//      double unrealizedPNL, double realizedPNL, const std::string& accountName) { }
//void TwsClient::updateAccountTime(const std::string& timeStamp) { }
void TwsClient::accountDownloadEnd(const std::string& accountName) { }
//void TwsClient::nextValidId( OrderId orderId) { }
void TwsClient::contractDetails( int reqId, const ContractDetails& contractDetails) { }
void TwsClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) { }
void TwsClient::contractDetailsEnd( int reqId) { }
void TwsClient::execDetails( int reqId, const Contract& contract, const Execution& execution) { }
void TwsClient::execDetailsEnd( int reqId) { }
//void TwsClient::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) { }
void TwsClient::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, Decimal size) { }
void TwsClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
      int side, double price, Decimal size, bool isSmartDepth) { }
void TwsClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) { }
void TwsClient::managedAccounts( const std::string& accountsList) { }
void TwsClient::receiveFA(faDataType pFaDataType, const std::string& cxml) { }
void TwsClient::historicalData(TickerId reqId, const Bar& bar) { }
void TwsClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) { }
void TwsClient::scannerParameters(const std::string& xml) { }
void TwsClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
	   const std::string& distance, const std::string& benchmark, const std::string& projection,
	   const std::string& legsStr) { }
void TwsClient::scannerDataEnd(int reqId) { }
void TwsClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	   Decimal volume, Decimal wap, int count) { }
//void TwsClient::currentTime(long time) { }
void TwsClient::fundamentalData(TickerId reqId, const std::string& data) { }
void TwsClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) { }
void TwsClient::tickSnapshotEnd( int reqId) { }
void TwsClient::marketDataType( TickerId reqId, int marketDataType) { }
void TwsClient::commissionReport( const CommissionReport& commissionReport) { }
//void TwsClient::position( const std::string& account, const Contract& contract, Decimal position, double avgCost) { }
//void TwsClient::positionEnd() { }
//void TwsClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency) { }
//void TwsClient::accountSummaryEnd( int reqId) { }
void TwsClient::verifyMessageAPI( const std::string& apiData) { }
void TwsClient::verifyCompleted( bool isSuccessful, const std::string& errorText) { }
void TwsClient::displayGroupList( int reqId, const std::string& groups) { }
void TwsClient::displayGroupUpdated( int reqId, const std::string& contractInfo) { }
void TwsClient::verifyAndAuthMessageAPI( const std::string& apiData, const std::string& xyzChallange) { }
void TwsClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) { }
//void TwsClient::connectAck() { }
void TwsClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) { }
void TwsClient::positionMultiEnd( int reqId) { }
void TwsClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) { }
void TwsClient::accountUpdateMultiEnd( int reqId) { }
void TwsClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
	const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) { }
void TwsClient::securityDefinitionOptionalParameterEnd(int reqId) { }
void TwsClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) { }
void TwsClient::familyCodes(const std::vector<FamilyCode> &familyCodes) { }
void TwsClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) { }
void TwsClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) { }
void TwsClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) { }
void TwsClient::smartComponents(int reqId, const SmartComponentsMap& theMap) { }
void TwsClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) { }
void TwsClient::newsProviders(const std::vector<NewsProvider> &newsProviders) { }
void TwsClient::newsArticle(int requestId, int articleType, const std::string& articleText) { }
void TwsClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) { }
void TwsClient::historicalNewsEnd(int requestId, bool hasMore) { }
void TwsClient::headTimestamp(int reqId, const std::string& headTimestamp) { }
void TwsClient::histogramData(int reqId, const HistogramDataVector& data) { }
void TwsClient::historicalDataUpdate(TickerId reqId, const Bar& bar) { }
void TwsClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) { }
void TwsClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) { }
void TwsClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) { }
void TwsClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) { }
void TwsClient::pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) { }
void TwsClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) { }
void TwsClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) { }
void TwsClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) { }
//void TwsClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) { }
void TwsClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) { }
void TwsClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) { }
void TwsClient::orderBound(long long orderId, int apiClientId, int apiOrderId) { }
void TwsClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) { }
void TwsClient::completedOrdersEnd() { }
void TwsClient::replaceFAEnd(int reqId, const std::string& text) { }
//void TwsClient::wshMetaData(int reqId, const std::string& dataJson) { }
//void TwsClient::wshEventData(int reqId, const std::string& dataJson) { }
void TwsClient::historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) { }
void TwsClient::userInfo(int reqId, const std::string& whiteBrandingId) { }
