
#include "pch.h"
#include "..\Utilities\UserMessages.h"
#include "..\Utilities\ListBoxData.h"
#include "..\Utilities\AfxWin.h"
#include "..\TradesPanel\TradesPanel.h"
#include "tws-client.h"

#include "tws-api\EClientSocket.h"
#include "tws-api\EPosixClientSocketPlatform.h"
#include "tws-api\CommonDefs.h"
#include "tws-api\Utils.h"



// The NavPanel window is exposed external because other
// areas of the application need to send messages to the
// "messages" label to display. e.g TWS connection status.
extern HWND HWND_MENUPANEL;

// The TradesPanel window is exposed external because we
// call the ListBox on that panel to display updated
// real time price data.
extern HWND HWND_TRADESPANEL;


bool isThreadFinished = false;
bool isThreadPaused = false;

bool isMonitorThreadActive = false;

TwsClient client;


//
// Thread function
//

std::promise<void> signal_exit; //create promise object
std::future<void> future;
std::thread my_thread;


void threadFunction(std::future<void> future) {
    std::cout << "Starting the thread" << std::endl;
	isMonitorThreadActive = true;
    while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
       // std::cout << "Executing the thread....." << std::endl;
        if (tws_isConnected()) {
            client.waitForSignal();
            client.processMsgs();
        }
        else {
            break;
        }

		std::chrono::milliseconds(250);   // wait for 250 milliseconds
    }
	isMonitorThreadActive = false;
	std::cout << "Thread Terminated" << std::endl;
}


void StartMonitorThread()
{
	if (isMonitorThreadActive) return;
	future = signal_exit.get_future();//create future objects
    my_thread = std::thread(&threadFunction, std::move(future)); //start thread, and move future
}

void EndMonitorThread()
{
	if (!isMonitorThreadActive) return;
	isThreadPaused = true;   // prevent processing TickData, etc while thread is shutting down
	std::cout << "Threads will be stopped soon...." << std::endl;
    signal_exit.set_value(); //set value into promise
    my_thread.join(); //join the thread with the main thread
}


bool tws_connect()
{
    if (tws_isConnected()) return false;

    const char* host = "";
    int port = 7496;   // 7497 is paper trading account
    int clientId = 0;

    SendMessage(HWND_MENUPANEL, MSG_TWS_CONNECT_START, 0, 0);

    bool res = client.connect(host, port, clientId);
    if (res) {
        // Start thread that will start messaging polling
        // and poll if TWS remains connected.
        SendMessage(HWND_MENUPANEL, MSG_TWS_CONNECT_SUCCESS, 0, 0);

        StartMonitorThread();
    }
    else {
        SendMessage(HWND_MENUPANEL, MSG_TWS_CONNECT_FAILURE, 0, 0);
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

	//! [connect]
	bool bRes = m_pClient->eConnect(host, port, clientId, m_extraAuth);
	//! [connect]

	if (bRes) {
		printf("Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
		//! [ereader]
		m_pReader = std::unique_ptr<EReader>(new EReader(m_pClient, &m_osSignal));
		m_pReader->start();
		//! [ereader]

	}
	else {
		printf("Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
	}
	return bRes;
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
	contract.symbol = symbol;
	contract.secType = "STK";
	contract.currency = "USD";
	contract.exchange = "SMART";  //"SMART" source code says not to use SMART but it seems to work;

	m_pClient->reqMktData(ld->tickerId, contract, "", false, false, TagValueListSPtr());

	//if (trade->tickerSymbol.startsWith("/")) {
	//	C.symbol = trade->tickerSymbol.toStdString().substr(1);
	//	C.secType = *TwsApi::SecType::FUT;
	//	C.exchange = *TwsApi::Exchange::CME;
	//	C.primaryExchange = *TwsApi::Exchange::CME;
	//	C.expiry = trade->futureExpiry.toStdString();
	//}
	//else {
	//	C.symbol = trade->tickerSymbol.toStdString();
	//	C.secType = *TwsApi::SecType::STK;
	//	C.exchange = *TwsApi::Exchange::IB_SMART;
	//	C.primaryExchange = *TwsApi::Exchange::ISLAND;
	//}
}
	



//////////////////////////////////////////////////////////////////
// methods
void TwsClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
		m_pClient->startApi();
}


void TwsClient::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
	if (isThreadPaused) return; 

	// Market data tick price callback. Handles all price related ticks.Every tickPrice callback is followed 
	// by a tickSize.A tickPrice value of - 1 or 0 followed by a tickSize of 0 indicates there is no data for 
	// this field currently available, whereas a tickPrice with a positive tickSize indicates an active 
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
		printf("Tick Price. Ticker Id: %ld, Field: %d, Price: %s, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, Utils::doubleMaxString(price).c_str(), attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);

		// These columns in the table are updated in real time when connected
		// to TWS. The LineData pointer is updated via a call to SetColumnData
		// and the correct ListBox line is invalidated/redrawn in order to force
		// display of the new price data. 

		HWND hListBox = GetDlgItem(HWND_TRADESPANEL, IDC_TRADES_LISTBOX);
		int lbCount = ListBox_GetCount(hListBox);
		
		for (int nIndex = 0; nIndex < lbCount; nIndex++) {

			ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hListBox, nIndex);
			if (ld == nullptr) continue;

			if ((ld->tickerId == tickerId) && (ld->trade != nullptr) && (ld->isTickerLine == TRUE)) {

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
				}


				// Calculate the price change
				double delta = 0;
				if (ld->trade->tickerClosePrice != 0) {
					delta = (ld->trade->tickerLastPrice - ld->trade->tickerClosePrice);
				}

				// Calculate if any of the option legs are ITM in a good (green) or bad (red) way.
				bool isITMred = false;
				bool isITMgreen = false;
				for (const auto& leg : ld->trade->openLegs) {
					if (leg->underlying == L"OPTIONS") {
						if (leg->PutCall == L"P") {
							if (ld->trade->tickerLastPrice < std::stod(leg->strikePrice)) {
								leg->openQuantity < 0 ? isITMred = true : isITMred = false;
								leg->openQuantity > 0 ? isITMgreen = true : isITMgreen = false;
								break;
							}
						}
						else if (leg->PutCall == L"C") {
							if (ld->trade->tickerLastPrice > std::stod(leg->strikePrice)) {
								leg->openQuantity < 0 ? isITMred = true : isITMred = false;
								leg->openQuantity > 0 ? isITMgreen = true : isITMgreen = false;
								break;
							}
						}
					}
				}


				std::wstring wszText = L"";

				ThemeElement themeEl = ThemeElement::TradesPanelText;
				if (isITMred) {
					wszText = L"ITM";
					themeEl = ThemeElement::valueNegative;
				}
				else if (isITMgreen) {
					wszText = L"ITM";
					themeEl = ThemeElement::valuePositive;
				}

				ld->SetTextData(COLUMN_TICKER_ITM, wszText, themeEl);  // ITM

				wszText = AfxMoney(delta);
				themeEl = (delta >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
				ld->SetTextData(COLUMN_TICKER_CHANGE, wszText, themeEl);  // price change

				wszText = AfxMoney(ld->trade->tickerLastPrice);
				ld->SetTextData(COLUMN_TICKER_CURRENTPRICE, wszText, ThemeElement::TradesPanelText);  // current price

				wszText = (delta >= 0 ? L"+" : L"") + AfxMoney((delta / ld->trade->tickerLastPrice) * 100) + L"%";
				themeEl = (delta >= 0) ? ThemeElement::valuePositive : ThemeElement::valueNegative;
				ld->SetTextData(COLUMN_TICKER_PERCENTAGE, wszText, themeEl);  // price percentage change


				// Do calculation to ensure column widths are wide enough to accommodate the new
				// price data that has just arrived.
				ListBoxData_ResizeColumnWidths(hListBox, nIndex);

				// Only update/repaint the line containing the new price data rather than the whole ListBox.
				RECT rc{};
				ListBox_GetItemRect(hListBox, nIndex, &rc);
				InvalidateRect(hListBox, &rc, TRUE);
				UpdateWindow(hListBox);
				
				break;

			}  // if
		}  // for
	}  // if
}

void TwsClient::tickSize(TickerId tickerId, TickType field, Decimal size) {
	if (isThreadPaused) return;
	// printf("Tick Size. Ticker Id: %ld, Field: %d, Size: %s\n", tickerId, (int)field, decimalStringToDisplay(size).c_str());
}

void TwsClient::tickOptionComputation(TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
	double optPrice, double pvDividend,
	double gamma, double vega, double theta, double undPrice) {
	if (isThreadPaused) return;
}

void TwsClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
	// printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, Utils::doubleMaxString(value).c_str());
	if (isThreadPaused) return;
}

void TwsClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	// printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
	if (isThreadPaused) return;
}

void TwsClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
	double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
	if (isThreadPaused) return;
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
}

void TwsClient::position(const std::string& account, const Contract& contract, Decimal position, double avgCost) {
	//    printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), decimalStringToDisplay(position).c_str(), Utils::doubleMaxString(avgCost).c_str());
}

void TwsClient::positionEnd() {
	printf("PositionEnd\n");
}

void TwsClient::winError(const std::string& str, int lastError) {}

void TwsClient::connectionClosed() {
	printf("Connection Closed\n");
}






void TwsClient::nextValidId(OrderId orderId) {
}

void TwsClient::currentTime(long time) {
}

void TwsClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
	Decimal remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
}

void TwsClient::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
}

void TwsClient::openOrderEnd() {
}

void TwsClient::updateAccountValue(const std::string& key, const std::string& val,
	const std::string& currency, const std::string& accountName) {
}

void TwsClient::updatePortfolio(const Contract& contract, Decimal position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const std::string& accountName) {
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
