/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "pch.h"
#include "EWrapper.h"
#include "Contract.h"
#include "TwsSocketClientErrors.h"
#include "EDecoder.h"
#include "EClientMsgSink.h"

#include <string.h>
#include <cstdlib>
#include <sstream>
#include <assert.h>
#include <string>
#include <bitset>
#include <cmath>

#define UNSET_DOUBLE DBL_MAX
#define UNSET_INTEGER INT_MAX
#define UNSET_LONG LLONG_MAX
#define COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID INFINITY

EDecoder::EDecoder(int serverVersion, EWrapper *callback, EClientMsgSink *clientMsgSink) {
	m_pEWrapper = callback;
	m_serverVersion = serverVersion;
	m_pClientMsgSink = clientMsgSink;
}


const char* EDecoder::processTickByTickDataMsg(const char* ptr, const char* endPtr) {
	int reqId;
	int tickType = 0;
	time_t time;

	DECODE_FIELD(reqId);
	DECODE_FIELD(tickType);
	DECODE_FIELD(time);

	if (tickType == 1 || tickType == 2) { // Last/AllLast
		double price;
		Decimal size;
		int attrMask;
		TickAttribLast tickAttribLast = {};
		std::string exchange;
		std::string specialConditions;

		DECODE_FIELD(price);
		DECODE_FIELD(size);
		DECODE_FIELD(attrMask);

		std::bitset<32> mask(attrMask);
		tickAttribLast.pastLimit = mask[0];
		tickAttribLast.unreported = mask[1];

		DECODE_FIELD(exchange);
		DECODE_FIELD(specialConditions);

		m_pEWrapper->tickByTickAllLast(reqId, tickType, time, price, size, tickAttribLast, exchange, specialConditions);

	}
	//else if (tickType == 3) { // BidAsk
	//	double bidPrice;
	//	double askPrice;
	//	Decimal bidSize;
	//	Decimal askSize;
	//	int attrMask;
	//	DECODE_FIELD(bidPrice);
	//	DECODE_FIELD(askPrice);
	//	DECODE_FIELD(bidSize);
	//	DECODE_FIELD(askSize);
	//	DECODE_FIELD(attrMask);

	//	TickAttribBidAsk tickAttribBidAsk = {};
	//	std::bitset<32> mask(attrMask);
	//	tickAttribBidAsk.bidPastLow = mask[0];
	//	tickAttribBidAsk.askPastHigh = mask[1];

	//	m_pEWrapper->tickByTickBidAsk(reqId, time, bidPrice, askPrice, bidSize, askSize, tickAttribBidAsk);
	//}
	//else if (tickType == 4) { // MidPoint
	//	double midPoint;
	//	DECODE_FIELD(midPoint);

	//	m_pEWrapper->tickByTickMidPoint(reqId, time, midPoint);
	//}

	return ptr;
}


const char* EDecoder::processTickPriceMsg(const char* ptr, const char* endPtr) {
	int version;
	int tickerId;
	int tickTypeInt;
	double price;

	Decimal size;
	int attrMask;

	DECODE_FIELD( version);
	DECODE_FIELD( tickerId);
	DECODE_FIELD( tickTypeInt);
	DECODE_FIELD( price);
	DECODE_FIELD( size); // ver 2 field
	DECODE_FIELD( attrMask); // ver 3 field

	TickAttrib attrib = {};

	attrib.canAutoExecute = attrMask == 1;

	if (m_serverVersion >= MIN_SERVER_VER_PAST_LIMIT)
	{
		std::bitset<32> mask(attrMask);

		attrib.canAutoExecute = mask[0];
		attrib.pastLimit = mask[1];

		if (m_serverVersion >= MIN_SERVER_VER_PRE_OPEN_BID_ASK)
		{
			attrib.preOpen = mask[2];
		}
	}

	m_pEWrapper->tickPrice( tickerId, (TickType)tickTypeInt, price, attrib);

	// process ver 2 fields
	{
		TickType sizeTickType = NOT_SET;
		switch( (TickType)tickTypeInt) {
		case BID:
			sizeTickType = BID_SIZE;
			break;
		case ASK:
			sizeTickType = ASK_SIZE;
			break;
		case LAST:
			sizeTickType = LAST_SIZE;
			break;
		case DELAYED_BID:
			sizeTickType = DELAYED_BID_SIZE;
			break;
		case DELAYED_ASK:
			sizeTickType = DELAYED_ASK_SIZE;
			break;
		case DELAYED_LAST:
			sizeTickType = DELAYED_LAST_SIZE;
			break;
		default:
			break;
		}
		if( sizeTickType != NOT_SET)
			m_pEWrapper->tickSize( tickerId, sizeTickType, size);
	}

	return ptr;
}

const char* EDecoder::processTickSizeMsg(const char* ptr, const char* endPtr) {
	int version;
	int tickerId;
	int tickTypeInt;
	Decimal size;

	DECODE_FIELD( version);
	DECODE_FIELD( tickerId);
	DECODE_FIELD( tickTypeInt);
	DECODE_FIELD( size);

	m_pEWrapper->tickSize( tickerId, (TickType)tickTypeInt, size);

	return ptr;
}

const char* EDecoder::processTickGenericMsg(const char* ptr, const char* endPtr) {
	int version;
	int tickerId;
	int tickTypeInt;
	double value;

	DECODE_FIELD( version);
	DECODE_FIELD( tickerId);
	DECODE_FIELD( tickTypeInt);
	DECODE_FIELD( value);

	m_pEWrapper->tickGeneric( tickerId, (TickType)tickTypeInt, value);

	return ptr;
}

const char* EDecoder::processTickStringMsg(const char* ptr, const char* endPtr) {
	int version;
	int tickerId;
	int tickTypeInt;
	std::string value;

	DECODE_FIELD( version);
	DECODE_FIELD( tickerId);
	DECODE_FIELD( tickTypeInt);
	DECODE_FIELD( value);

	m_pEWrapper->tickString( tickerId, (TickType)tickTypeInt, value);

	return ptr;
}

const char* EDecoder::processErrMsgMsg(const char* ptr, const char* endPtr) {
	int version;
	int id; // ver 2 field
	int errorCode; // ver 2 field
	std::string errorMsg;
	std::string advancedOrderRejectJson;

	DECODE_FIELD( version);
	DECODE_FIELD( id);
	DECODE_FIELD( errorCode);
	DECODE_FIELD( errorMsg);

	if (m_serverVersion >= MIN_SERVER_VER_ADVANCED_ORDER_REJECT)
	{
		DECODE_FIELD( advancedOrderRejectJson);
	}

	m_pEWrapper->error( id, errorCode, errorMsg, advancedOrderRejectJson);

	return ptr;
}

const char* EDecoder::processAcctValueMsg(const char* ptr, const char* endPtr) {
	int version;
	std::string key;
	std::string val;
	std::string cur;
	std::string accountName;

	DECODE_FIELD( version);
	DECODE_FIELD( key);
	DECODE_FIELD( val);
	DECODE_FIELD( cur);
	DECODE_FIELD( accountName); // ver 2 field

	m_pEWrapper->updateAccountValue( key, val, cur, accountName);
	return ptr;
}

const char* EDecoder::processPortfolioValueMsg(const char* ptr, const char* endPtr) {
	// decode version
	int version;
	DECODE_FIELD( version);

	// read contract fields
	Contract contract;
	DECODE_FIELD( contract.conId); // ver 6 field
	DECODE_FIELD( contract.symbol);
	DECODE_FIELD( contract.secType);
	DECODE_FIELD( contract.lastTradeDateOrContractMonth);
	DECODE_FIELD( contract.strike);
	DECODE_FIELD( contract.right);

	if( version >= 7) {
		DECODE_FIELD( contract.multiplier);
		DECODE_FIELD( contract.primaryExchange);
	}

	DECODE_FIELD( contract.currency);
	DECODE_FIELD( contract.localSymbol); // ver 2 field
	if (version >= 8) {
		DECODE_FIELD( contract.tradingClass);
	}

	Decimal  position;
	double  marketPrice;
	double  marketValue;
	double  averageCost;
	double  unrealizedPNL;
	double  realizedPNL;

	DECODE_FIELD( position);
	DECODE_FIELD( marketPrice);
	DECODE_FIELD( marketValue);
	DECODE_FIELD( averageCost); // ver 3 field
	DECODE_FIELD( unrealizedPNL); // ver 3 field
	DECODE_FIELD( realizedPNL); // ver 3 field

	std::string accountName;
	DECODE_FIELD( accountName); // ver 4 field

	if( version == 6 && m_serverVersion == 39) {
		DECODE_FIELD( contract.primaryExchange);
	}

	m_pEWrapper->updatePortfolio( contract,
		position, marketPrice, marketValue, averageCost,
		unrealizedPNL, realizedPNL, accountName);

	return ptr;
}

const char* EDecoder::processNextValidIdMsg(const char* ptr, const char* endPtr) {
	int version;
	int orderId;

	DECODE_FIELD( version);
	DECODE_FIELD( orderId);

	m_pEWrapper->nextValidId(orderId);

	return ptr;
}

const char* EDecoder::processCurrentTimeMsg(const char* ptr, const char* endPtr) {
	int version;
	int time;

	DECODE_FIELD(version);
	DECODE_FIELD(time);

	m_pEWrapper->currentTime( time);

	return ptr;
}

const char* EDecoder::processPositionDataMsg(const char* ptr, const char* endPtr) {
	int version;
	std::string account;
	Decimal position;
	double avgCost = 0;

	DECODE_FIELD( version);
	DECODE_FIELD( account);

	// decode contract fields
	Contract contract;
	DECODE_FIELD( contract.conId);
	DECODE_FIELD( contract.symbol);
	DECODE_FIELD( contract.secType);
	DECODE_FIELD( contract.lastTradeDateOrContractMonth);
	DECODE_FIELD( contract.strike);
	DECODE_FIELD( contract.right);
	DECODE_FIELD( contract.multiplier);
	DECODE_FIELD( contract.exchange);
	DECODE_FIELD( contract.currency);
	DECODE_FIELD( contract.localSymbol);
	if (version >= 2) {
		DECODE_FIELD( contract.tradingClass);
	}
	DECODE_FIELD( position);
	if (version >= 3) {
		DECODE_FIELD( avgCost);
	}

	m_pEWrapper->position( account, contract, position, avgCost);

	return ptr;
}

const char* EDecoder::processPositionEndMsg(const char* ptr, const char* endPtr) {
	int version;

	DECODE_FIELD( version);

	m_pEWrapper->positionEnd();

	return ptr;
}

const char* EDecoder::processAccountSummaryMsg(const char* ptr, const char* endPtr) {
	int version;
	int reqId;
	std::string account;
	std::string tag;
	std::string value;
	std::string curency;

	DECODE_FIELD( version);
	DECODE_FIELD( reqId);
	DECODE_FIELD( account);
	DECODE_FIELD( tag);
	DECODE_FIELD( value);
	DECODE_FIELD( curency);

	m_pEWrapper->accountSummary( reqId, account, tag, value, curency);

	return ptr;
}

const char* EDecoder::processAccountSummaryEndMsg(const char* ptr, const char* endPtr) {
	int version;
	int reqId;

	DECODE_FIELD( version);
	DECODE_FIELD( reqId);

	m_pEWrapper->accountSummaryEnd( reqId);

	return ptr;
}

const char* EDecoder::processWshEventData(const char* ptr, const char* endPtr) {
	int reqId;
	std::string dataJson;

	DECODE_FIELD(reqId);
	DECODE_FIELD(dataJson);

	m_pEWrapper->wshEventData(reqId, dataJson);

	return ptr;
}

const char* EDecoder::processWshMetaData(const char* ptr, const char* endPtr) {
	int reqId;
	std::string dataJson;

	DECODE_FIELD(reqId);
	DECODE_FIELD(dataJson);

	m_pEWrapper->wshMetaData(reqId, dataJson);

	return ptr;
}

int EDecoder::processConnectAck(const char*& beginPtr, const char* endPtr)
{
	// process a connect Ack message from the buffer;
	// return number of bytes consumed
	assert( beginPtr && beginPtr < endPtr);

	try {

		const char* ptr = beginPtr;

		// check server version
		DECODE_FIELD( m_serverVersion);

		// handle redirects
		if( m_serverVersion < 0) {
			m_serverVersion = 0;

			std::string hostport, host;
			int port = -1;

			DECODE_FIELD( hostport);

			std::string::size_type sep = hostport.find( ':');
			if( sep != std::string::npos) {
				host = hostport.substr(0, sep);
				port = atoi( hostport.c_str() + ++sep);
			}
			else {
				host = hostport;
			}

			if (m_pClientMsgSink)
				m_pClientMsgSink->redirect(host.c_str(), port);
		} else {
			std::string twsTime;

			if( m_serverVersion >= 20) {

				DECODE_FIELD(twsTime);
			}

			if (m_pClientMsgSink)
				m_pClientMsgSink->serverVersion(m_serverVersion, twsTime.c_str());

			m_pEWrapper->connectAck();
		}

		int processed = ptr - beginPtr;
		beginPtr = ptr;
		return processed;
	}
	catch(const std::exception& e) {
		m_pEWrapper->error( NO_VALID_ID, SOCKET_EXCEPTION.code(), SOCKET_EXCEPTION.msg() + e.what(), "");
	}

	return 0;
}


int EDecoder::parseAndProcessMsg(const char*& beginPtr, const char* endPtr) {
	// process a single message from the buffer;
	// return number of bytes consumed

	assert( beginPtr && beginPtr < endPtr);

	if (m_serverVersion == 0)
		return processConnectAck(beginPtr, endPtr);

	try {

		const char* ptr = beginPtr;

		int msgId;
		DECODE_FIELD( msgId);

		switch( msgId) {
		case TICK_PRICE:
			ptr = processTickPriceMsg(ptr, endPtr);
			break;

		case TICK_SIZE:
			ptr = processTickSizeMsg(ptr, endPtr);
			break;

		case TICK_GENERIC:
			ptr = processTickGenericMsg(ptr, endPtr);
			break;

		case TICK_STRING:
			ptr = processTickStringMsg(ptr, endPtr);
			break;

		case TICK_BY_TICK:
			ptr = processTickByTickDataMsg(ptr, endPtr);
			break;

		case ERR_MSG:
			ptr = processErrMsgMsg(ptr, endPtr);
			break;

		case ACCT_VALUE:
			ptr = processAcctValueMsg(ptr, endPtr);
			break;

		case PORTFOLIO_VALUE:
			ptr = processPortfolioValueMsg(ptr, endPtr);
			break;

		case NEXT_VALID_ID:
			ptr = processNextValidIdMsg(ptr, endPtr);
			break;

		case CURRENT_TIME:
			ptr = processCurrentTimeMsg(ptr, endPtr);
			break;

		case POSITION_DATA:
			ptr = processPositionDataMsg(ptr, endPtr);
			break;

		case POSITION_END:
			ptr = processPositionEndMsg(ptr, endPtr);
			break;

		case ACCOUNT_SUMMARY:
			ptr = processAccountSummaryMsg(ptr, endPtr);
			break;

		case ACCOUNT_SUMMARY_END:
			ptr = processAccountSummaryEndMsg(ptr, endPtr);
			break;

		case WSH_META_DATA:
			ptr = processWshMetaData(ptr, endPtr);
			break;

		case WSH_EVENT_DATA:
			ptr = processWshEventData(ptr, endPtr);
			break;

		default:
			{
			// I have removed most messages other than the ones above so don't throw
			// error on any message not listed above.
			//	m_pEWrapper->error( msgId, UNKNOWN_ID.code(), UNKNOWN_ID.msg(), "");
			//	m_pEWrapper->connectionClosed();
				break;
			}
		}

		if (!ptr)
			return 0;

		int processed = ptr - beginPtr;
		beginPtr = ptr;
		return processed;
	}
	catch(const std::exception& e) {
		m_pEWrapper->error( NO_VALID_ID, SOCKET_EXCEPTION.code(), SOCKET_EXCEPTION.msg() + e.what(), "");
	}
	return 0;
}


bool EDecoder::CheckOffset(const char* ptr, const char* endPtr)
{
	assert (ptr && ptr <= endPtr);
	return (ptr && ptr < endPtr);
}

const char* EDecoder::FindFieldEnd(const char* ptr, const char* endPtr)
{
	return (const char*)memchr(ptr, 0, endPtr - ptr);
}

bool EDecoder::DecodeField(bool& boolValue, const char*& ptr, const char* endPtr)
{
	int intValue;
	if( !DecodeField(intValue, ptr, endPtr))
		return false;
	boolValue = (intValue > 0);
	return true;
}

bool EDecoder::DecodeField(int& intValue, const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if( !fieldEnd)
		return false;
	intValue = atoi(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeFieldTime(time_t& time_tValue, const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if( !fieldEnd)
		return false;
	time_tValue = atoll(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(long long& longLongValue, const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if( !fieldEnd)
		return false;
	longLongValue = atoll(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(long& longValue, const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if( !fieldEnd)
		return false;
	longValue = atol(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(double& doubleValue, const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if( !fieldEnd)
		return false;
	doubleValue = fieldBeg == INFINITY_STR ? INFINITY : atof(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(std::string& stringValue,
						   const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(ptr, endPtr);
	if( !fieldEnd)
		return false;
	stringValue = fieldBeg; // better way?
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(char& charValue,
						   const char*& ptr, const char* endPtr)
{
	if( !CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(ptr, endPtr);
	if( !fieldEnd)
		return false;
	charValue = fieldBeg[0]; // better way?
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeField(Decimal& decimalValue, const char*& ptr, const char* endPtr)
{
	if (!CheckOffset(ptr, endPtr))
		return false;
	const char* fieldBeg = ptr;
	const char* fieldEnd = FindFieldEnd(fieldBeg, endPtr);
	if (!fieldEnd)
		return false;
	decimalValue = stringToDecimal(fieldBeg);
	ptr = ++fieldEnd;
	return true;
}

bool EDecoder::DecodeFieldMax(int& intValue, const char*& ptr, const char* endPtr)
{
	std::string stringValue;
	if( !DecodeField(stringValue, ptr, endPtr))
		return false;
	intValue = stringValue.empty() ? UNSET_INTEGER : atoi(stringValue.c_str());
	return true;
}

bool EDecoder::DecodeFieldMax(long& longValue, const char*& ptr, const char* endPtr)
{
	int intValue;
	if( !DecodeFieldMax(intValue, ptr, endPtr))
		return false;
	longValue = intValue;
	return true;
}

bool EDecoder::DecodeFieldMax(double& doubleValue, const char*& ptr, const char* endPtr)
{
	std::string stringValue;
	if( !DecodeField(stringValue, ptr, endPtr))
		return false;
	doubleValue = stringValue.empty() ? UNSET_DOUBLE : atof(stringValue.c_str());
	return true;
}

const char* EDecoder::decodeLastTradeDate(const char* ptr, const char* endPtr, ContractDetails& contract, bool isBond) {
	std::string lastTradeDateOrContractMonth;
	DECODE_FIELD( lastTradeDateOrContractMonth);
	if (!lastTradeDateOrContractMonth.empty()){
		char split_with = ' ';
		if (lastTradeDateOrContractMonth.find("-") != std::string::npos) {
			split_with = '-';
		}
		std::vector<std::string> splitted;
		std::istringstream buf(lastTradeDateOrContractMonth);
		std::string s;
		while (getline(buf, s, split_with)) {
			splitted.push_back(s);
		}
		if (splitted.size() > 0) {
			if (isBond) {
				contract.maturity = splitted[0];
			} else {
				contract.contract.lastTradeDateOrContractMonth = splitted[0];
			}
		}
		if (splitted.size() > 1) {
			contract.lastTradeTime = splitted[1];
		}
		if (isBond && splitted.size() > 2) {
			contract.timeZoneId = splitted[2];
		}
	}
	return ptr;
}
