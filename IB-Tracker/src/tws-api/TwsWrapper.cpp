/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "pch.h"

#include "TwsWrapper.h"

#include "EClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "CommonDefs.h"
#include "Utils.h"

#include <stdio.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <fstream>
#include <cstdint>
#include <chrono>


const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds



///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]
TwsClient::TwsClient() :
      m_osSignal(2000)//2-seconds timeout
    , m_pClient(new EClientSocket(this, &m_osSignal))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
    , m_extraAuth(false)
{
}
//! [socket_init]
TwsClient::~TwsClient()
{
	// destroy the reader before the client
	if( m_pReader )
		m_pReader.reset();

	delete m_pClient;
}

bool TwsClient::connect(const char *host, int port, int clientId)
{
	// trying to connect
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	
	//! [connect]
	bool bRes = m_pClient->eConnect( host, port, clientId, m_extraAuth);
	//! [connect]
	
	if (bRes) {
		printf( "Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
		//! [ereader]
		m_pReader = std::unique_ptr<EReader>( new EReader(m_pClient, &m_osSignal) );
		m_pReader->start();
		//! [ereader]
		
	} else {
		printf("Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
	}
	return bRes;
}

void TwsClient::disconnect() const
{
	m_pClient->eDisconnect();

	printf ( "Disconnected\n");
}

bool TwsClient::isConnected() const
{
	return m_pClient->isConnected();
}

bool TwsClient::isSocketOK() const
{
	return m_pClient->isSocketOK();
}

void TwsClient::setConnectOptions(const std::string& connectOptions)
{
	m_pClient->setConnectOptions(connectOptions);
}

void TwsClient::waitForSignal()
{
	m_osSignal.waitForSignal();

}

void TwsClient::processMsgs()
{
	m_pReader->processMsgs();
}


void TwsClient::processMessages()
{

	//time_t now = time(NULL);

	//switch (m_state) {
	//	case ST_PING:
	//		printf("PING\n");
	//		reqCurrentTime();
	//		break;

	//	case ST_PING_ACK:
	//		printf("PING ACK\n");
	//		if( m_sleepDeadline < now) {
	//			printf("PING ACK DISCONNECT\n");
	//			disconnect();
	//			return;
	//		}
	//		break;

	//	case ST_IDLE:
	//		if( m_sleepDeadline < now) {
	//			m_state = ST_PING;
	//			return;
	//		}
	//		break;
	//}

	//m_osSignal.waitForSignal();
	//errno = 0;
	//m_pReader->processMsgs();

}

//////////////////////////////////////////////////////////////////
// methods
//! [connectack]
void TwsClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
        m_pClient->startApi();
}
//! [connectack]

void TwsClient::reqCurrentTime()
{
	printf( "Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time( NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;

	m_pClient->reqCurrentTime();
}

void TwsClient::tickDataOperation()
{
	/*** Requesting real time market data ***/
    std::this_thread::sleep_for(std::chrono::seconds(1));
	
//	struct Contract;
		Contract contract;
		contract.symbol = "IBM";
		contract.secType = "STK";
		contract.currency = "USD";
		contract.exchange = "SMART";

		//! [reqmktdata]
	m_pClient->reqMktData(1001, contract, "", false, false, TagValueListSPtr());
//	m_pClient->reqMktData(1002, ContractSamples::OptionWithLocalSymbol(), "", false, false, TagValueListSPtr());
//	//! [reqmktdata]

//	//! [reqmktdata_genticks]
//	//Requesting RTVolume (Time & Sales) and shortable generic ticks
//	m_pClient->reqMktData(1004, ContractSamples::USStockAtSmart(), "233,236", false, false, TagValueListSPtr());
//	//! [reqmktdata_genticks]
//
	std::this_thread::sleep_for(std::chrono::seconds(1));
	/*** Canceling the market data subscription ***/
	//! [cancelmktdata]
	m_pClient->cancelMktData(1001);
//	m_pClient->cancelMktData(1002);
	//! [cancelmktdata]

	m_state = ST_TICKDATAOPERATION_ACK;
}


void TwsClient::reqTickByTickData() 
{
//    /*** Requesting tick-by-tick data (only refresh) ***/
//    
//    m_pClient->reqTickByTickData(20001, ContractSamples::EuropeanStock(), "Last", 0, false);
//    m_pClient->reqTickByTickData(20002, ContractSamples::EuropeanStock(), "AllLast", 0, false);
//    m_pClient->reqTickByTickData(20003, ContractSamples::EuropeanStock(), "BidAsk", 0, true);
//    m_pClient->reqTickByTickData(20004, ContractSamples::EurGbpFx(), "MidPoint", 0, false);
//
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//	//! [canceltickbytick]
//    m_pClient->cancelTickByTickData(20001);
//    m_pClient->cancelTickByTickData(20002);
//    m_pClient->cancelTickByTickData(20003);
//    m_pClient->cancelTickByTickData(20004);
//    //! [canceltickbytick]
//	
//    /*** Requesting tick-by-tick data (historical + refresh) ***/
//    //! [reqtickbytick]
//    m_pClient->reqTickByTickData(20005, ContractSamples::EuropeanStock(), "Last", 10, false);
//    m_pClient->reqTickByTickData(20006, ContractSamples::EuropeanStock(), "AllLast", 10, false);
//    m_pClient->reqTickByTickData(20007, ContractSamples::EuropeanStock(), "BidAsk", 10, false);
//    m_pClient->reqTickByTickData(20008, ContractSamples::EurGbpFx(), "MidPoint", 10, true);
//	//! [reqtickbytick]
//	
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//    m_pClient->cancelTickByTickData(20005);
//    m_pClient->cancelTickByTickData(20006);
//    m_pClient->cancelTickByTickData(20007);
//    m_pClient->cancelTickByTickData(20008);
//
    m_state = ST_REQTICKBYTICKDATA_ACK;
}


////! [nextvalidid]
void TwsClient::nextValidId( OrderId orderId)
{
	printf("Next Valid Id: %ld\n", orderId);
	m_orderId = orderId;
	//! [nextvalidid]
	m_state = ST_PING;
}


void TwsClient::currentTime( long time)
{
	if ( m_state == ST_PING_ACK) {
		time_t t = ( time_t)time;
		struct tm * timeinfo = localtime ( &t);
		printf( "The current date/time is: %s", asctime( timeinfo));

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_PING_ACK;
	}
}

//! [error]
void TwsClient::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson)
{
    if (!advancedOrderRejectJson.empty()) {
        printf("Error. Id: %d, Code: %d, Msg: %s, AdvancedOrderRejectJson: %s\n", id, errorCode, errorString.c_str(), advancedOrderRejectJson.c_str());
    } else {
        printf("Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
    }
}
//! [error]

//! [tickprice]
void TwsClient::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
    printf( "Tick Price. Ticker Id: %ld, Field: %d, Price: %s, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, Utils::doubleMaxString(price).c_str(), attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);
}
//! [tickprice]

//! [ticksize]
void TwsClient::tickSize( TickerId tickerId, TickType field, Decimal size) {
	printf( "Tick Size. Ticker Id: %ld, Field: %d, Size: %s\n", tickerId, (int)field, decimalStringToDisplay(size).c_str());
}
//! [ticksize]

//! [tickoptioncomputation]
void TwsClient::tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
                                          double optPrice, double pvDividend,
                                          double gamma, double vega, double theta, double undPrice) {
    printf( "TickOptionComputation. Ticker Id: %ld, Type: %d, TickAttrib: %s, ImpliedVolatility: %s, Delta: %s, OptionPrice: %s, pvDividend: %s, Gamma: %s, Vega: %s, Theta: %s, Underlying Price: %s\n", 
        tickerId, (int)tickType, Utils::intMaxString(tickAttrib).c_str(), Utils::doubleMaxString(impliedVol).c_str(), Utils::doubleMaxString(delta).c_str(), Utils::doubleMaxString(optPrice).c_str(), 
        Utils::doubleMaxString(pvDividend).c_str(), Utils::doubleMaxString(gamma).c_str(), Utils::doubleMaxString(vega).c_str(), Utils::doubleMaxString(theta).c_str(), Utils::doubleMaxString(undPrice).c_str());
}
//! [tickoptioncomputation]

//! [tickgeneric]
void TwsClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
    printf( "Tick Generic. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, Utils::doubleMaxString(value).c_str());
}
//! [tickgeneric]

//! [tickstring]
void TwsClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	printf( "Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}
//! [tickstring]

void TwsClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
                            double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
    printf( "TickEFP. %ld, Type: %d, BasisPoints: %s, FormattedBasisPoints: %s, Total Dividends: %s, HoldDays: %s, Future Last Trade Date: %s, Dividend Impact: %s, Dividends To Last Trade Date: %s\n", 
        tickerId, (int)tickType, Utils::doubleMaxString(basisPoints).c_str(), formattedBasisPoints.c_str(), Utils::doubleMaxString(totalDividends).c_str(), Utils::intMaxString(holdDays).c_str(), 
        futureLastTradeDate.c_str(), Utils::doubleMaxString(dividendImpact).c_str(), Utils::doubleMaxString(dividendsToLastTradeDate).c_str());
}

//! [orderstatus]
void TwsClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
		Decimal remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice){
    printf("OrderStatus. Id: %ld, Status: %s, Filled: %s, Remaining: %s, AvgFillPrice: %s, PermId: %s, LastFillPrice: %s, ClientId: %s, WhyHeld: %s, MktCapPrice: %s\n", 
        orderId, status.c_str(), decimalStringToDisplay(filled).c_str(), decimalStringToDisplay(remaining).c_str(), Utils::doubleMaxString(avgFillPrice).c_str(), Utils::intMaxString(permId).c_str(), 
        Utils::doubleMaxString(lastFillPrice).c_str(), Utils::intMaxString(clientId).c_str(), whyHeld.c_str(), Utils::doubleMaxString(mktCapPrice).c_str());
}
//! [orderstatus]

//! [openorder]
void TwsClient::openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
    //printf( "OpenOrder. PermId: %s, ClientId: %s, OrderId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType:%s, TotalQty: %s, CashQty: %s, "
    //    "LmtPrice: %s, AuxPrice: %s, Status: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s\n", 
    //    Utils::intMaxString(order.permId).c_str(), Utils::longMaxString(order.clientId).c_str(), Utils::longMaxString(orderId).c_str(), order.account.c_str(), contract.symbol.c_str(), 
    //    contract.secType.c_str(), contract.exchange.c_str(), order.action.c_str(), order.orderType.c_str(), decimalStringToDisplay(order.totalQuantity).c_str(), 
    //    Utils::doubleMaxString(order.cashQty).c_str(), Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(),
    //    Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(), 
    //    order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
    //    Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str());
}
//! [openorder]

//! [openorderend]
void TwsClient::openOrderEnd() {
	printf( "OpenOrderEnd\n");
}
//! [openorderend]

void TwsClient::winError( const std::string& str, int lastError) {}

void TwsClient::connectionClosed() {
	printf( "Connection Closed\n");
}

//! [updateaccountvalue]
void TwsClient::updateAccountValue(const std::string& key, const std::string& val,
                                       const std::string& currency, const std::string& accountName) {
	printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}
//! [updateaccountvalue]

//! [updateportfolio]
void TwsClient::updatePortfolio(const Contract& contract, Decimal position,
                                    double marketPrice, double marketValue, double averageCost,
                                    double unrealizedPNL, double realizedPNL, const std::string& accountName){
    //printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: %s, AccountName: %s\n", 
    //    (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), decimalStringToDisplay(position).c_str(), 
    //    Utils::doubleMaxString(marketPrice).c_str(), Utils::doubleMaxString(marketValue).c_str(), Utils::doubleMaxString(averageCost).c_str(), 
    //    Utils::doubleMaxString(unrealizedPNL).c_str(), Utils::doubleMaxString(realizedPNL).c_str(), accountName.c_str());
}
//! [updateportfolio]

//! [updateaccounttime]
void TwsClient::updateAccountTime(const std::string& timeStamp) {
	printf( "UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}
//! [updateaccounttime]

//! [accountdownloadend]
void TwsClient::accountDownloadEnd(const std::string& accountName) {
	printf( "Account download finished: %s\n", accountName.c_str());
}
//! [accountdownloadend]

//! [contractdetails]
void TwsClient::contractDetails( int reqId, const ContractDetails& contractDetails) {
	//printf( "ContractDetails begin. ReqId: %d\n", reqId);
	//printContractMsg(contractDetails.contract);
	//printContractDetailsMsg(contractDetails);
	//printf( "ContractDetails end. ReqId: %d\n", reqId);
}
//! [contractdetails]

//! [bondcontractdetails]
void TwsClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {
	//printf( "BondContractDetails begin. ReqId: %d\n", reqId);
	//printBondContractDetailsMsg(contractDetails);
	//printf( "BondContractDetails end. ReqId: %d\n", reqId);
}
//! [bondcontractdetails]

//! [contractdetailsend]
void TwsClient::contractDetailsEnd( int reqId) {
	printf( "ContractDetailsEnd. %d\n", reqId);
}
//! [contractdetailsend]

//! [execdetails]
void TwsClient::execDetails( int reqId, const Contract& contract, const Execution& execution) {
//	printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %s, %s, %s, %s\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), Utils::longMaxString(execution.orderId).c_str(), decimalStringToDisplay(execution.shares).c_str(), decimalStringToDisplay(execution.cumQty).c_str(), Utils::intMaxString(execution.lastLiquidity).c_str());
}
//! [execdetails]

//! [execdetailsend]
void TwsClient::execDetailsEnd( int reqId) {
	printf( "ExecDetailsEnd. %d\n", reqId);
}
//! [execdetailsend]

//! [updatemktdepth]
void TwsClient::updateMktDepth(TickerId id, int position, int operation, int side,
                                   double price, Decimal size) {
    //printf( "UpdateMarketDepth. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s\n", id, Utils::intMaxString(position).c_str(), operation, side, 
    //    Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str());
}
//! [updatemktdepth]

//! [updatemktdepthl2]
void TwsClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
                                     int side, double price, Decimal size, bool isSmartDepth) {
    //printf( "UpdateMarketDepthL2. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s, isSmartDepth: %d\n", id, Utils::intMaxString(position).c_str(), operation, side, 
    //    Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str(), isSmartDepth);
}
//! [updatemktdepthl2]

//! [updatenewsbulletin]
void TwsClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
	printf( "News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
}
//! [updatenewsbulletin]

//! [managedaccounts]
void TwsClient::managedAccounts( const std::string& accountsList) {
	printf( "Account List: %s\n", accountsList.c_str());
}
//! [managedaccounts]

//! [receivefa]
void TwsClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
	std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
}
//! [receivefa]

//! [historicaldata]
void TwsClient::historicalData(TickerId reqId, const Bar& bar) {
    printf( "HistoricalData. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, bar.time.c_str(), 
        Utils::doubleMaxString(bar.open).c_str(), Utils::doubleMaxString(bar.high).c_str(), Utils::doubleMaxString(bar.low).c_str(), Utils::doubleMaxString(bar.close).c_str(), 
        decimalStringToDisplay(bar.volume).c_str(), Utils::intMaxString(bar.count).c_str(), decimalStringToDisplay(bar.wap).c_str());
}
//! [historicaldata]

//! [historicaldataend]
void TwsClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	std::cout << "HistoricalDataEnd. ReqId: " << reqId << " - Start Date: " << startDateStr << ", End Date: " << endDateStr << std::endl;	
}
//! [historicaldataend]

//! [scannerparameters]
void TwsClient::scannerParameters(const std::string& xml) {
	printf( "ScannerParameters. %s\n", xml.c_str());
}
//! [scannerparameters]

//! [scannerdata]
void TwsClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
                                const std::string& distance, const std::string& benchmark, const std::string& projection,
                                const std::string& legsStr) {
	printf( "ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.contract.symbol.c_str(), contractDetails.contract.secType.c_str(), contractDetails.contract.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
}
//! [scannerdata]

//! [scannerdataend]
void TwsClient::scannerDataEnd(int reqId) {
	printf( "ScannerDataEnd. %d\n", reqId);
}
//! [scannerdataend]

//! [realtimebar]
void TwsClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                                Decimal volume, Decimal wap, int count) {
    printf( "RealTimeBars. %ld - Time: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, Utils::longMaxString(time).c_str(), 
        Utils::doubleMaxString(open).c_str(), Utils::doubleMaxString(high).c_str(), Utils::doubleMaxString(low).c_str(), Utils::doubleMaxString(close).c_str(), 
        decimalStringToDisplay(volume).c_str(), Utils::intMaxString(count).c_str(), decimalStringToDisplay(wap).c_str());
}
//! [realtimebar]

//! [fundamentaldata]
void TwsClient::fundamentalData(TickerId reqId, const std::string& data) {
	printf( "FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}
//! [fundamentaldata]

void TwsClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
    printf( "DeltaNeutralValidation. %d, ConId: %ld, Delta: %s, Price: %s\n", reqId, deltaNeutralContract.conId, Utils::doubleMaxString(deltaNeutralContract.delta).c_str(), Utils::doubleMaxString(deltaNeutralContract.price).c_str());
}

//! [ticksnapshotend]
void TwsClient::tickSnapshotEnd(int reqId) {
//	printf( "TickSnapshotEnd: %d\n", reqId);
}
//! [ticksnapshotend]

//! [marketdatatype]
void TwsClient::marketDataType(TickerId reqId, int marketDataType) {
//	printf( "MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
}
//! [marketdatatype]

//! [commissionreport]
void TwsClient::commissionReport( const CommissionReport& commissionReport) {
//    printf( "CommissionReport. %s - %s %s RPNL %s\n", commissionReport.execId.c_str(), Utils::doubleMaxString(commissionReport.commission).c_str(), commissionReport.currency.c_str(), Utils::doubleMaxString(commissionReport.realizedPNL).c_str());
}
//! [commissionreport]

//! [position]
void TwsClient::position( const std::string& account, const Contract& contract, Decimal position, double avgCost) {
//    printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), decimalStringToDisplay(position).c_str(), Utils::doubleMaxString(avgCost).c_str());
}
//! [position]

//! [positionend]
void TwsClient::positionEnd() {
	printf( "PositionEnd\n");
}
//! [positionend]

//! [accountsummary]
void TwsClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
	printf( "Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}
//! [accountsummary]

//! [accountsummaryend]
void TwsClient::accountSummaryEnd( int reqId) {
	printf( "AccountSummaryEnd. Req Id: %d\n", reqId);
}
//! [accountsummaryend]

void TwsClient::verifyMessageAPI( const std::string& apiData) {
	printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void TwsClient::verifyCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
}

void TwsClient::verifyAndAuthMessageAPI( const std::string& apiDatai, const std::string& xyzChallenge) {
	printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
}

void TwsClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
    if (isSuccessful)
        m_pClient->startApi();
}

//! [displaygrouplist]
void TwsClient::displayGroupList( int reqId, const std::string& groups) {
	printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}
//! [displaygrouplist]

//! [displaygroupupdated]
void TwsClient::displayGroupUpdated( int reqId, const std::string& contractInfo) {
	std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
}
//! [displaygroupupdated]

//! [positionmulti]
void TwsClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) {
    printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(avgCost).c_str());
}
//! [positionmulti]

//! [positionmultiend]
void TwsClient::positionMultiEnd( int reqId) {
	printf("Position Multi End. Request: %d\n", reqId);
}
//! [positionmultiend]

//! [accountupdatemulti]
void TwsClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
	printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
}
//! [accountupdatemulti]

//! [accountupdatemultiend]
void TwsClient::accountUpdateMultiEnd( int reqId) {
	printf("Account Update Multi End. Request: %d\n", reqId);
}
//! [accountupdatemultiend]

//! [securityDefinitionOptionParameter]
void TwsClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
                                                        const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {
	printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
}
//! [securityDefinitionOptionParameter]

//! [securityDefinitionOptionParameterEnd]
void TwsClient::securityDefinitionOptionalParameterEnd(int reqId) {
	printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}
//! [securityDefinitionOptionParameterEnd]

//! [softDollarTiers]
void TwsClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
	printf("Soft dollar tiers (%lu):", tiers.size());

	for (unsigned int i = 0; i < tiers.size(); i++) {
		printf("%s\n", tiers[i].displayName().c_str());
	}
}
//! [softDollarTiers]

//! [familyCodes]
void TwsClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
	printf("Family codes (%lu):\n", familyCodes.size());

	for (unsigned int i = 0; i < familyCodes.size(); i++) {
		printf("Family code [%d] - accountID: %s familyCodeStr: %s\n", i, familyCodes[i].accountID.c_str(), familyCodes[i].familyCodeStr.c_str());
	}
}
//! [familyCodes]

//! [symbolSamples]
void TwsClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {
	printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(), reqId);

	for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
		Contract contract = contractDescriptions[i].contract;
		std::vector<std::string> derivativeSecTypes = contractDescriptions[i].derivativeSecTypes;
		printf("Contract (%u): conId: %ld, symbol: %s, secType: %s, primaryExchange: %s, currency: %s, ", i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(), contract.primaryExchange.c_str(), contract.currency.c_str());
		printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
		for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
			printf(" %s", derivativeSecTypes[j].c_str());
		}
		printf(", description: %s, issuerId: %s", contract.description.c_str(), contract.issuerId.c_str());
		printf("\n");
	}
}
//! [symbolSamples]

//! [mktDepthExchanges]
void TwsClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
	printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

	for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
        printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s listingExch: %s serviceDataType: %s aggGroup: %s\n", i,
            depthMktDataDescriptions[i].exchange.c_str(),
            depthMktDataDescriptions[i].secType.c_str(),
            depthMktDataDescriptions[i].listingExch.c_str(),
            depthMktDataDescriptions[i].serviceDataType.c_str(),
            Utils::intMaxString(depthMktDataDescriptions[i].aggGroup).c_str());
	}
}
//! [mktDepthExchanges]

//! [tickNews]
void TwsClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
	printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, ArticleId: %s, Headline: %s, ExtraData: %s\n", tickerId, ctime(&(timeStamp /= 1000)), providerCode.c_str(), articleId.c_str(), headline.c_str(), extraData.c_str());
}
//! [tickNews]

//! [smartcomponents]]
void TwsClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
	printf("Smart components: (%lu):\n", theMap.size());

	for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end(); i++) {
		printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first, std::get<0>(i->second).c_str(), std::get<1>(i->second));
	}
}
//! [smartcomponents]

//! [tickReqParams]
void TwsClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
    printf("tickerId: %d, minTick: %s, bboExchange: %s, snapshotPermissions: %u\n", tickerId, Utils::doubleMaxString(minTick).c_str(), bboExchange.c_str(), snapshotPermissions);

	m_bboExchange = bboExchange;
}
//! [tickReqParams]

//! [newsProviders]
void TwsClient::newsProviders(const std::vector<NewsProvider> &newsProviders) {
	printf("News providers (%lu):\n", newsProviders.size());

	for (unsigned int i = 0; i < newsProviders.size(); i++) {
		printf("News provider [%d] - providerCode: %s providerName: %s\n", i, newsProviders[i].providerCode.c_str(), newsProviders[i].providerName.c_str());
	}
}
//! [newsProviders]

//! [newsArticle]
void TwsClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
	printf("News Article. Request Id: %d, Article Type: %d\n", requestId, articleType);
	if (articleType == 0) {
		printf("News Article Text (text or html): %s\n", articleText.c_str());
	} else if (articleType == 1) {
		//std::string path;
		//#if defined(IB_WIN32)
		//	TCHAR s[200];
		//	GetCurrentDirectory(200, s);
		//	path = s + std::string("\\MST$06f53098.pdf");
		//#elif defined(IB_POSIX)
		//	char s[1024];
		//	if (getcwd(s, sizeof(s)) == NULL) {
		//		printf("getcwd() error\n");
		//		return;
		//	}
		//	path = s + std::string("/MST$06f53098.pdf");
		//#endif
		//std::vector<std::uint8_t> bytes = Utils::base64_decode(articleText);
		//std::ofstream outfile(path, std::ios::out | std::ios::binary); 
		//outfile.write((const char*)bytes.data(), bytes.size());
		//printf("Binary/pdf article was saved to: %s\n", path.c_str());
	}
}
//! [newsArticle]

//! [historicalNews]
void TwsClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
	printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, ArticleId: %s, Headline: %s\n", requestId, time.c_str(), providerCode.c_str(), articleId.c_str(), headline.c_str());
}
//! [historicalNews]

//! [historicalNewsEnd]
void TwsClient::historicalNewsEnd(int requestId, bool hasMore) {
	printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId, (hasMore ? "true" : " false"));
}
//! [historicalNewsEnd]

//! [headTimestamp]
void TwsClient::headTimestamp(int reqId, const std::string& headTimestamp) {
	printf( "Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId, headTimestamp.c_str());

}
//! [headTimestamp]

//! [histogramData]
void TwsClient::histogramData(int reqId, const HistogramDataVector& data) {
	printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

    for (const HistogramEntry& entry : data) {
        printf("\t price: %s, size: %s\n", Utils::doubleMaxString(entry.price).c_str(), decimalStringToDisplay(entry.size).c_str());
	}
}
//! [histogramData]

//! [historicalDataUpdate]
void TwsClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
    printf( "HistoricalDataUpdate. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, bar.time.c_str(), 
        Utils::doubleMaxString(bar.open).c_str(), Utils::doubleMaxString(bar.high).c_str(), Utils::doubleMaxString(bar.low).c_str(), Utils::doubleMaxString(bar.close).c_str(), 
        decimalStringToDisplay(bar.volume).c_str(), Utils::intMaxString(bar.count).c_str(), decimalStringToDisplay(bar.wap).c_str());
}
//! [historicalDataUpdate]

//! [rerouteMktDataReq]
void TwsClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDataReq]

//! [rerouteMktDepthReq]
void TwsClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market depth request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDepthReq]

//! [marketRule]
void TwsClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
    printf("Market Rule Id: %s\n", Utils::intMaxString(marketRuleId).c_str());
    for (unsigned int i = 0; i < priceIncrements.size(); i++) {
        printf("Low Edge: %s, Increment: %s\n", Utils::doubleMaxString(priceIncrements[i].lowEdge).c_str(), Utils::doubleMaxString(priceIncrements[i].increment).c_str());
    }
}
//! [marketRule]

//! [pnl]
void TwsClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
    printf("PnL. ReqId: %d, daily PnL: %s, unrealized PnL: %s, realized PnL: %s\n", reqId, Utils::doubleMaxString(dailyPnL).c_str(), Utils::doubleMaxString(unrealizedPnL).c_str(), 
        Utils::doubleMaxString(realizedPnL).c_str());
}
//! [pnl]

//! [pnlsingle]
void TwsClient::pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
    printf("PnL Single. ReqId: %d, pos: %s, daily PnL: %s, unrealized PnL: %s, realized PnL: %s, value: %s\n", reqId, decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(dailyPnL).c_str(), 
        Utils::doubleMaxString(unrealizedPnL).c_str(), Utils::doubleMaxString(realizedPnL).c_str(), Utils::doubleMaxString(value).c_str());
}
//! [pnlsingle]

//! [historicalticks]
void TwsClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
    for (const HistoricalTick& tick : ticks) {
    std::time_t t = tick.time;
        std::cout << "Historical tick. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< Utils::doubleMaxString(tick.price).c_str()	<< ", size: " << decimalStringToDisplay(tick.size).c_str() << std::endl;
    }
}
//! [historicalticks]

//! [historicalticksbidask]
void TwsClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
    for (const HistoricalTickBidAsk& tick : ticks) {
    std::time_t t = tick.time;
        std::cout << "Historical tick bid/ask. ReqId: " << reqId << ", time: " << ctime(&t) << ", price bid: "<< Utils::doubleMaxString(tick.priceBid).c_str()	<<
            ", price ask: "<< Utils::doubleMaxString(tick.priceAsk).c_str() << ", size bid: " << decimalStringToDisplay(tick.sizeBid).c_str() << ", size ask: " << decimalStringToDisplay(tick.sizeAsk).c_str() <<
            ", bidPastLow: " << tick.tickAttribBidAsk.bidPastLow << ", askPastHigh: " << tick.tickAttribBidAsk.askPastHigh << std::endl;
    }
}
//! [historicalticksbidask]

//! [historicaltickslast]
void TwsClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
    for (HistoricalTickLast tick : ticks) {
	std::time_t t = tick.time;
        std::cout << "Historical tick last. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< Utils::doubleMaxString(tick.price).c_str() <<
            ", size: " << decimalStringToDisplay(tick.size).c_str() << ", exchange: " << tick.exchange << ", special conditions: " << tick.specialConditions <<
            ", unreported: " << tick.tickAttribLast.unreported << ", pastLimit: " << tick.tickAttribLast.pastLimit << std::endl;
    }
}
//! [historicaltickslast]

//! [tickbytickalllast]
void TwsClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
    printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %s, Size: %s, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n", 
        reqId, (tickType == 1 ? "Last" : "AllLast"), ctime(&time), Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str(), tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
}
//! [tickbytickalllast]

//! [tickbytickbidask]
void TwsClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) {
    printf("Tick-By-Tick. ReqId: %d, TickType: BidAsk, Time: %s, BidPrice: %s, AskPrice: %s, BidSize: %s, AskSize: %s, BidPastLow: %d, AskPastHigh: %d\n", 
        reqId, ctime(&time), Utils::doubleMaxString(bidPrice).c_str(), Utils::doubleMaxString(askPrice).c_str(), decimalStringToDisplay(bidSize).c_str(), decimalStringToDisplay(askSize).c_str(), tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
}
//! [tickbytickbidask]

//! [tickbytickmidpoint]
void TwsClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
    printf("Tick-By-Tick. ReqId: %d, TickType: MidPoint, Time: %s, MidPoint: %s\n", reqId, ctime(&time), Utils::doubleMaxString(midPoint).c_str());
}
//! [tickbytickmidpoint]

//! [orderbound]
void TwsClient::orderBound(long long orderId, int apiClientId, int apiOrderId) {
    printf("Order bound. OrderId: %s, ApiClientId: %s, ApiOrderId: %s\n", Utils::llongMaxString(orderId).c_str(), Utils::intMaxString(apiClientId).c_str(), Utils::intMaxString(apiOrderId).c_str());
}
//! [orderbound]

//! [completedorder]
void TwsClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
    //printf( "CompletedOrder. PermId: %s, ParentPermId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType: %s, TotalQty: %s, CashQty: %s, FilledQty: %s, "
    //    "LmtPrice: %s, AuxPrice: %s, Status: %s, CompletedTime: %s, CompletedStatus: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s\n",
    //    Utils::intMaxString(order.permId).c_str(), Utils::llongMaxString(order.parentPermId).c_str(), order.account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(),
    //    order.action.c_str(), order.orderType.c_str(), decimalStringToDisplay(order.totalQuantity).c_str(), Utils::doubleMaxString(order.cashQty).c_str(), decimalStringToDisplay(order.filledQuantity).c_str(),
    //    Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(), orderState.completedTime.c_str(), orderState.completedStatus.c_str(),
    //    Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(),
    //    order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
    //    Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str());
}
//! [completedorder]

//! [completedordersend]
void TwsClient::completedOrdersEnd() {
	printf( "CompletedOrdersEnd\n");
}
//! [completedordersend]

//! [replacefaend]
void TwsClient::replaceFAEnd(int reqId, const std::string& text) {
	printf("Replace FA End. Request: %d, Text:%s\n", reqId, text.c_str());
}
//! [replacefaend]

//! [wshMetaData]
void TwsClient::wshMetaData(int reqId, const std::string& dataJson) {
	printf("WSH Meta Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}
//! [wshMetaData]

//! [wshEventData]
void TwsClient::wshEventData(int reqId, const std::string& dataJson) {
	printf("WSH Event Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}
//! [wshEventData]

//! [historicalSchedule]
void TwsClient::historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) {
	printf("Historical Schedule. ReqId: %d, Start: %s, End: %s, TimeZone: %s\n", reqId, startDateTime.c_str(), endDateTime.c_str(), timeZone.c_str());
	for (unsigned int i = 0; i < sessions.size(); i++) {
		printf("\tSession. Start: %s, End: %s, RefDate: %s\n", sessions[i].startDateTime.c_str(), sessions[i].endDateTime.c_str(), sessions[i].refDate.c_str());
	}
}
//! [historicalSchedule]

//! [userInfo]
void TwsClient::userInfo(int reqId, const std::string& whiteBrandingId) {
    printf("User Info. ReqId: %d, WhiteBrandingId: %s\n", reqId, whiteBrandingId.c_str());
}
//! [userInfo]
