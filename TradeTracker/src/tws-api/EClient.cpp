/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "pch.h"

#include "EPosixClientSocketPlatform.h"

#include "EClient.h"

#include "EWrapper.h"
#include "TwsSocketClientErrors.h"
#include "Contract.h"
#include "EDecoder.h"
#include "EMessage.h"
#include "ETransport.h"
#include "EClientException.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include <stdio.h>
#include <string.h>
#include <assert.h>


using namespace ibapi::client_constants;

///////////////////////////////////////////////////////////
// define explict specialization of int encoder before first use
template void EClient::EncodeField<int>(std::ostream&, int);

// encoders
template<>
void EClient::EncodeField<bool>(std::ostream& os, bool boolValue)
{
    EncodeField<int>(os, boolValue ? 1 : 0);
}

template<>
void EClient::EncodeField<double>(std::ostream& os, double doubleValue)
{
    char str[128];

    if (doubleValue == INFINITY) {
        snprintf(str, sizeof(str), "%s", INFINITY_STR.c_str());
    } 
    else {
        snprintf(str, sizeof(str), "%.10g", doubleValue);
    }

    EncodeField<const char*>(os, str);
}

template<>
void EClient::EncodeField<Decimal>(std::ostream& os, Decimal decimalValue)
{
    char str[128];
    snprintf(str, sizeof(str), "%s", decimalToString(decimalValue).c_str());

    EncodeField<const char*>(os, str);
}

template<class T>
void EClient::EncodeField(std::ostream& os, T value)
{
    os << value << '\0';
}
template<> 
void EClient::EncodeField<std::string>(std::ostream& os, std::string value)
{
    if (!value.empty() && !isAsciiPrintable(value)) {
        throw EClientException(INVALID_SYMBOL, value);
    }

    EncodeField<std::string&>(os, value);
}

bool EClient::isAsciiPrintable(const std::string& s)
{
    return std::all_of(s.begin(), s.end(), [](char c) {
        return static_cast<unsigned char>(c) >= 32 && static_cast<unsigned char>(c) < 127;
    });
}

void EClient::EncodeContract(std::ostream& os, const Contract &contract)
{
    EncodeField(os, contract.conId);
    EncodeField(os, contract.symbol);
    EncodeField(os, contract.secType);
    EncodeField(os, contract.lastTradeDateOrContractMonth);
    EncodeField(os, contract.strike);
    EncodeField(os, contract.right);
    EncodeField(os, contract.multiplier);
    EncodeField(os, contract.exchange);
    EncodeField(os, contract.primaryExchange);
    EncodeField(os, contract.currency);
    EncodeField(os, contract.localSymbol);
    EncodeField(os, contract.tradingClass);
    EncodeField(os, contract.includeExpired);
}

void EClient::EncodeTagValueList(std::ostream& os, const TagValueListSPtr &tagValueList) 
{
    std::string tagValueListStr("");
    const size_t tagValueListCount = tagValueList.get() ? tagValueList->size() : 0;

    if (tagValueListCount > 0) {
        for (size_t i = 0; i < tagValueListCount; ++i) {
            const TagValue* tagValue = ((*tagValueList)[i]).get();

            tagValueListStr += tagValue->tag;
            tagValueListStr += "=";
            tagValueListStr += tagValue->value;
            tagValueListStr += ";";
        }
    }

    EncodeField(os, tagValueListStr);
}

///////////////////////////////////////////////////////////
// "max" encoders
void EClient::EncodeFieldMax(std::ostream& os, int intValue)
{
    if( intValue == INT_MAX) {
        EncodeField(os, "");
        return;
    }
    EncodeField(os, intValue);
}

void EClient::EncodeFieldMax(std::ostream& os, double doubleValue)
{
    if( doubleValue == DBL_MAX) {
        EncodeField(os, "");
        return;
    }
    EncodeField(os, doubleValue);
}


///////////////////////////////////////////////////////////
// member funcs
EClient::EClient( EWrapper *ptr, ETransport *pTransport)
    : m_pEWrapper(ptr)
    , m_transport(pTransport)
    , m_clientId(-1)
    , m_connState(CS_DISCONNECTED)
    , m_extraAuth(false)
    , m_serverVersion(0)
    , m_useV100Plus(true)
    , m_port(0)
{
}

EClient::~EClient()
{
}

EClient::ConnState EClient::connState() const
{
    return m_connState;
}

bool EClient::isConnected() const
{
    return m_connState == CS_CONNECTED;
}

bool EClient::isConnecting() const
{
    return m_connState == CS_CONNECTING;
}

void EClient::eConnectBase()
{
}

void EClient::eDisconnectBase()
{
    m_TwsTime.clear();
    m_serverVersion = 0;
    m_connState = CS_DISCONNECTED;
    m_extraAuth = false;
    m_clientId = -1;
}

int EClient::serverVersion()
{
    return m_serverVersion;
}

std::string EClient::TwsConnectionTime()
{
    return m_TwsTime;
}

const std::string& EClient::optionalCapabilities() const
{
    return m_optionalCapabilities;
}

void EClient::setOptionalCapabilities(const std::string& optCapts)
{
    m_optionalCapabilities = optCapts;
}

void EClient::setConnectOptions(const std::string& connectOptions)
{
    if( isSocketOK()) {
        m_pEWrapper->error( NO_VALID_ID, ALREADY_CONNECTED.code(), ALREADY_CONNECTED.msg(), "");
        return;
    }

    m_connectOptions = connectOptions;
}

void EClient::disableUseV100Plus()
{
    if( isSocketOK()) {
        m_pEWrapper->error( NO_VALID_ID, ALREADY_CONNECTED.code(), ALREADY_CONNECTED.msg(), "");
        return;
    }

    m_useV100Plus = false;
    m_connectOptions = "";
}

bool EClient::usingV100Plus() {
    return m_useV100Plus;
}

int EClient::bufferedSend(const std::string& msg) {
    EMessage emsg(std::vector<char>(msg.begin(), msg.end()));

    return m_transport->send(&emsg);
}

void EClient::reqMktData(TickerId ticker_id, const Contract& contract,
                         const std::string& genericTicks, bool snapshot, bool regulatorySnaphsot, const TagValueListSPtr& mktDataOptions)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( ticker_id, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    // not needed anymore validation
    //if( m_serverVersion < MIN_SERVER_VER_SNAPSHOT_MKT_DATA && snapshot) {
    //	m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
    //		"  It does not support snapshot market data requests.");
    //	return;
    //}

    if( m_serverVersion < MIN_SERVER_VER_DELTA_NEUTRAL) {
        if( contract.deltaNeutralContract) {
            m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
                "  It does not support delta-neutral orders.", "");
            return;
        }
    }

    if (m_serverVersion < MIN_SERVER_VER_REQ_MKT_DATA_CONID) {
        if( contract.conId > 0) {
            m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
                "  It does not support conId parameter.", "");
            return;
        }
    }

    if (m_serverVersion < MIN_SERVER_VER_TRADING_CLASS) {
        if( !contract.tradingClass.empty() ) {
            m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
                "  It does not support tradingClass parameter in reqMktData.", "");
            return;
        }
    }

    std::stringstream msg;
    prepareBuffer( msg);

    try {
        const int VERSION = 11;

        // send req mkt data msg
        ENCODE_FIELD( REQ_MKT_DATA);
        ENCODE_FIELD( VERSION);
        ENCODE_FIELD( ticker_id);

        // send contract fields
        if( m_serverVersion >= MIN_SERVER_VER_REQ_MKT_DATA_CONID) {
            ENCODE_FIELD( contract.conId);
        }
        ENCODE_FIELD( contract.symbol);
        ENCODE_FIELD( contract.secType);
        ENCODE_FIELD( contract.lastTradeDateOrContractMonth);
        ENCODE_FIELD( contract.strike);
        ENCODE_FIELD( contract.right);
        ENCODE_FIELD( contract.multiplier); // srv v15 and above

        ENCODE_FIELD( contract.exchange);
        ENCODE_FIELD( contract.primaryExchange); // srv v14 and above
        ENCODE_FIELD( contract.currency);

        ENCODE_FIELD( contract.localSymbol); // srv v2 and above

        if( m_serverVersion >= MIN_SERVER_VER_TRADING_CLASS) {
            ENCODE_FIELD( contract.tradingClass);
        }

        // Send combo legs for BAG requests (srv v8 and above)
        if( contract.secType == "BAG")
        {
            const Contract::ComboLegList* const comboLegs = contract.comboLegs.get();
            const size_t comboLegsCount = comboLegs ? comboLegs->size() : 0;
            ENCODE_FIELD( comboLegsCount);
            if( comboLegsCount > 0) {
                for( size_t i = 0; i < comboLegsCount; ++i) {
                    const ComboLeg* comboLeg = ((*comboLegs)[i]).get();
                    assert( comboLeg);
                    ENCODE_FIELD( comboLeg->conId);
                    ENCODE_FIELD( comboLeg->ratio);
                    ENCODE_FIELD( comboLeg->action);
                    ENCODE_FIELD( comboLeg->exchange);
                }
            }
        }

        if( m_serverVersion >= MIN_SERVER_VER_DELTA_NEUTRAL) {
            if( contract.deltaNeutralContract) {
                const DeltaNeutralContract& deltaNeutralContract = *contract.deltaNeutralContract;
                ENCODE_FIELD( true);
                ENCODE_FIELD( deltaNeutralContract.conId);
                ENCODE_FIELD( deltaNeutralContract.delta);
                ENCODE_FIELD( deltaNeutralContract.price);
            }
            else {
                ENCODE_FIELD( false);
            }
        }

        ENCODE_FIELD( genericTicks); // srv v31 and above
        ENCODE_FIELD( snapshot); // srv v35 and above

        if (m_serverVersion >= MIN_SERVER_VER_REQ_SMART_COMPONENTS) {
            ENCODE_FIELD(regulatorySnaphsot);
        }

        // send mktDataOptions parameter
        if( m_serverVersion >= MIN_SERVER_VER_LINKING) {
            ENCODE_TAGVALUELIST(mktDataOptions);
        }
    }
    catch (EClientException& ex) {
        m_pEWrapper->error(ticker_id, ex.error().code(), ex.error().msg() + ex.text(), "");
        return;
    }

    closeAndSend( msg.str());
}

void EClient::cancelMktData(TickerId ticker_id)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( ticker_id, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 2;

    // send cancel mkt data msg
    ENCODE_FIELD( CANCEL_MKT_DATA);
    ENCODE_FIELD( VERSION);
    ENCODE_FIELD( ticker_id);

    closeAndSend( msg.str());
}

void EClient::reqMktDepth( TickerId ticker_id, const Contract& contract, int numRows, bool isSmartDepth, const TagValueListSPtr& mktDepthOptions)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( ticker_id, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    // Not needed anymore validation
    // This feature is only available for versions of TWS >=6
    //if( m_serverVersion < 6) {
    //	m_pEWrapper->error( NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg());
    //	return;
    //}

    if (m_serverVersion < MIN_SERVER_VER_TRADING_CLASS) {
        if( !contract.tradingClass.empty() || (contract.conId > 0)) {
            m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
                "  It does not support conId and tradingClass parameters in reqMktDepth.", "");
            return;
        }
    }

    if (m_serverVersion < MIN_SERVER_VER_SMART_DEPTH && isSmartDepth) {
        m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support SMART depth request.", "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_MKT_DEPTH_PRIM_EXCHANGE && !contract.primaryExchange.empty()) {
        m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support primaryExchange parameter in reqMktDepth.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    try {
        const int VERSION = 5;

        // send req mkt data msg
        ENCODE_FIELD( REQ_MKT_DEPTH);
        ENCODE_FIELD( VERSION);
        ENCODE_FIELD( ticker_id);

        // send contract fields
        if( m_serverVersion >= MIN_SERVER_VER_TRADING_CLASS) {
            ENCODE_FIELD( contract.conId);
        }
        ENCODE_FIELD( contract.symbol);
        ENCODE_FIELD( contract.secType);
        ENCODE_FIELD( contract.lastTradeDateOrContractMonth);
        ENCODE_FIELD( contract.strike);
        ENCODE_FIELD( contract.right);
        ENCODE_FIELD( contract.multiplier); // srv v15 and above
        ENCODE_FIELD( contract.exchange);
        if( m_serverVersion >= MIN_SERVER_VER_MKT_DEPTH_PRIM_EXCHANGE) {
            ENCODE_FIELD( contract.primaryExchange);
        }
        ENCODE_FIELD( contract.currency);
        ENCODE_FIELD( contract.localSymbol);
        if( m_serverVersion >= MIN_SERVER_VER_TRADING_CLASS) {
            ENCODE_FIELD( contract.tradingClass);
        }

        ENCODE_FIELD( numRows); // srv v19 and above

        if( m_serverVersion >= MIN_SERVER_VER_SMART_DEPTH) {
            ENCODE_FIELD( isSmartDepth);
        }

        // send mktDepthOptions parameter
        if( m_serverVersion >= MIN_SERVER_VER_LINKING) {
            ENCODE_TAGVALUELIST(mktDepthOptions);
        }
    }
    catch (EClientException& ex) {
        m_pEWrapper->error(ticker_id, ex.error().code(), ex.error().msg() + ex.text(), "");
        return;
    }

    closeAndSend( msg.str());
}


void EClient::cancelMktDepth( TickerId ticker_id, bool isSmartDepth)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( ticker_id, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_SMART_DEPTH && isSmartDepth) {
        m_pEWrapper->error( ticker_id, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support SMART depth cancel.", "");
        return;
    }

    // Not needed anymore validation
    // This feature is only available for versions of TWS >=6
    //if( m_serverVersion < 6) {
    //	m_pEWrapper->error( NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg());
    //	return;
    //}

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 1;

    // send cancel mkt data msg
    ENCODE_FIELD( CANCEL_MKT_DEPTH);
    ENCODE_FIELD( VERSION);
    ENCODE_FIELD( ticker_id);

    if( m_serverVersion >= MIN_SERVER_VER_SMART_DEPTH) {
        ENCODE_FIELD( isSmartDepth);
    }

    closeAndSend( msg.str());
}

void EClient::reqCurrentTime()
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 1;

    // send current time req
    ENCODE_FIELD( REQ_CURRENT_TIME);
    ENCODE_FIELD( VERSION);

    closeAndSend( msg.str());
}

void EClient::reqAccountUpdates(bool subscribe, const std::string& acctCode)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    try {
        const int VERSION = 2;

        // send req acct msg
        ENCODE_FIELD( REQ_ACCT_DATA);
        ENCODE_FIELD( VERSION);
        ENCODE_FIELD( subscribe);  // true = subscribe, false = unsubscribe.

        // Send the account code. This will only be used for FA clients
        ENCODE_FIELD( acctCode); // srv v9 and above
    }
    catch (EClientException& ex) {
        m_pEWrapper->error(NO_VALID_ID, ex.error().code(), ex.error().msg() + ex.text(), "");
        return;
    }

    closeAndSend( msg.str());
}

void EClient::reqPositions()
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_POSITIONS) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support positions request.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 1;

    ENCODE_FIELD( REQ_POSITIONS);
    ENCODE_FIELD( VERSION);

    closeAndSend( msg.str());
}

void EClient::cancelPositions()
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_POSITIONS) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support positions cancellation.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 1;

    ENCODE_FIELD( CANCEL_POSITIONS);
    ENCODE_FIELD( VERSION);

    closeAndSend( msg.str());
}

void EClient::reqAccountSummary( int reqId, const std::string& groupName, const std::string& tags)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_ACCOUNT_SUMMARY) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support account summary request.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    try {
        const int VERSION = 1;

        ENCODE_FIELD( REQ_ACCOUNT_SUMMARY);
        ENCODE_FIELD( VERSION);
        ENCODE_FIELD( reqId);
        ENCODE_FIELD( groupName);
        ENCODE_FIELD( tags);
    }
    catch (EClientException& ex) {
        m_pEWrapper->error(reqId, ex.error().code(), ex.error().msg() + ex.text(), "");
        return;
    }

    closeAndSend( msg.str());
}

void EClient::cancelAccountSummary( int reqId)
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_ACCOUNT_SUMMARY) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support account summary cancellation.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer( msg);

    const int VERSION = 1;

    ENCODE_FIELD( CANCEL_ACCOUNT_SUMMARY);
    ENCODE_FIELD( VERSION);
    ENCODE_FIELD( reqId);

    closeAndSend( msg.str());
}

void EClient::startApi()
{
    // not connected?
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion >= 3) {
        if( m_serverVersion < MIN_SERVER_VER_LINKING) {
            std::stringstream msg;
            ENCODE_FIELD( m_clientId);
            bufferedSend( msg.str());
        }
        else
        {
            std::stringstream msg;
            prepareBuffer( msg);

            try {
                const int VERSION = 2;

                ENCODE_FIELD( START_API);
                ENCODE_FIELD( VERSION);
                ENCODE_FIELD( m_clientId);

                if (m_serverVersion >= MIN_SERVER_VER_OPTIONAL_CAPABILITIES)
                    ENCODE_FIELD(m_optionalCapabilities);
            }
            catch (EClientException& ex) {
                m_pEWrapper->error(NO_VALID_ID, ex.error().code(), ex.error().msg() + ex.text(), "");
                return;
            }

            closeAndSend( msg.str());
        }
    }
}

void EClient::reqTickByTickData(int reqId, const Contract &contract, const std::string& tickType, int numberOfTicks, bool ignoreSize) {
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_TICK_BY_TICK) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support tick-by-tick data request.", "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_TICK_BY_TICK_IGNORE_SIZE) {
        if (numberOfTicks != 0 || ignoreSize) {
            m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
                "  It does not support ignoreSize and numberOfTicks parameters in tick-by-tick data requests.", "");
            return;
        }
    }

    std::stringstream msg;
    prepareBuffer(msg);

    try {
        ENCODE_FIELD(REQ_TICK_BY_TICK_DATA);
        ENCODE_FIELD(reqId);
        ENCODE_FIELD( contract.conId);
        ENCODE_FIELD( contract.symbol);
        ENCODE_FIELD( contract.secType);
        ENCODE_FIELD( contract.lastTradeDateOrContractMonth);
        ENCODE_FIELD( contract.strike);
        ENCODE_FIELD( contract.right);
        ENCODE_FIELD( contract.multiplier);
        ENCODE_FIELD( contract.exchange);
        ENCODE_FIELD( contract.primaryExchange);
        ENCODE_FIELD( contract.currency);
        ENCODE_FIELD( contract.localSymbol);
        ENCODE_FIELD( contract.tradingClass);
        ENCODE_FIELD( tickType);
        if( m_serverVersion >= MIN_SERVER_VER_TICK_BY_TICK_IGNORE_SIZE) {
            ENCODE_FIELD( numberOfTicks);
            ENCODE_FIELD( ignoreSize);
        }
    }
    catch (EClientException& ex) {
        m_pEWrapper->error(reqId, ex.error().code(), ex.error().msg() + ex.text(), "");
        return;
    }

    closeAndSend(msg.str());    
}

void EClient::cancelTickByTickData(int reqId) {
    if( !isConnected()) {
        m_pEWrapper->error( NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if( m_serverVersion < MIN_SERVER_VER_TICK_BY_TICK) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support tick-by-tick data cancel.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer(msg);

    ENCODE_FIELD(CANCEL_TICK_BY_TICK_DATA);
    ENCODE_FIELD(reqId);

    closeAndSend(msg.str());    
}

void EClient::reqWshMetaData(int reqId) {
    if (!isConnected()) {
        m_pEWrapper->error(NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_WSHE_CALENDAR) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support WSHE Calendar API.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer(msg);

    ENCODE_FIELD(REQ_WSH_META_DATA)
        ENCODE_FIELD(reqId)

        closeAndSend(msg.str());
}

void EClient::reqWshEventData(int reqId, const WshEventData& wshEventData) {
    if (!isConnected()) {
        m_pEWrapper->error(NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_WSHE_CALENDAR) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support WSHE Calendar API.", "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_WSH_EVENT_DATA_FILTERS) {
        if (!wshEventData.filter.empty() || wshEventData.fillWatchlist || wshEventData.fillPortfolio || wshEventData.fillCompetitors) {
            m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() + "  It does not support WSH event data filters.", "");
            return;
        }
    }

    if (m_serverVersion < MIN_SERVER_VER_WSH_EVENT_DATA_FILTERS_DATE) {
        if (!wshEventData.startDate.empty() || !wshEventData.endDate.empty() || wshEventData.totalLimit != INT_MAX) {
            m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() + "  It does not support WSH event data date filters.", "");
            return;
        }
    }

    std::stringstream msg;
    prepareBuffer(msg);

    ENCODE_FIELD(REQ_WSH_EVENT_DATA)
        ENCODE_FIELD(reqId)
        ENCODE_FIELD(wshEventData.conId)

        if (m_serverVersion >= MIN_SERVER_VER_WSH_EVENT_DATA_FILTERS) {
            ENCODE_FIELD(wshEventData.filter);
            ENCODE_FIELD(wshEventData.fillWatchlist);
            ENCODE_FIELD(wshEventData.fillPortfolio);
            ENCODE_FIELD(wshEventData.fillCompetitors);
        }

    if (m_serverVersion >= MIN_SERVER_VER_WSH_EVENT_DATA_FILTERS_DATE) {
        ENCODE_FIELD(wshEventData.startDate);
        ENCODE_FIELD(wshEventData.endDate);
        ENCODE_FIELD(wshEventData.totalLimit);
    }

    closeAndSend(msg.str());
}

void EClient::cancelWshMetaData(int reqId) {
    if (!isConnected()) {
        m_pEWrapper->error(NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_WSHE_CALENDAR) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support WSHE Calendar API.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer(msg);

    ENCODE_FIELD(CANCEL_WSH_META_DATA)
        ENCODE_FIELD(reqId)

        closeAndSend(msg.str());
}

void EClient::cancelWshEventData(int reqId) {
    if (!isConnected()) {
        m_pEWrapper->error(NO_VALID_ID, NOT_CONNECTED.code(), NOT_CONNECTED.msg(), "");
        return;
    }

    if (m_serverVersion < MIN_SERVER_VER_WSHE_CALENDAR) {
        m_pEWrapper->error(NO_VALID_ID, UPDATE_TWS.code(), UPDATE_TWS.msg() +
            "  It does not support WSHE Calendar API.", "");
        return;
    }

    std::stringstream msg;
    prepareBuffer(msg);

    ENCODE_FIELD(CANCEL_WSH_EVENT_DATA)
        ENCODE_FIELD(reqId)

        closeAndSend(msg.str());
}

bool EClient::extraAuth() {
    return m_extraAuth;
}

EWrapper* EClient::getWrapper() const
{
    return m_pEWrapper;
}

void EClient::setClientId(int clientId)
{
    m_clientId = clientId;
}

void EClient::setExtraAuth(bool extraAuth)
{
    m_extraAuth = extraAuth;
}

void EClient::setHost(const std::string& host)
{
    m_host = host;
}

void EClient::setPort(int port)
{
    m_port = port;
}


///////////////////////////////////////////////////////////
// callbacks from socket
int EClient::sendConnectRequest()
{
    m_connState = CS_CONNECTING;

    int rval;

    // send client version
    std::stringstream msg;
    if( m_useV100Plus) {
        msg.write( API_SIGN, sizeof(API_SIGN));
        prepareBufferImpl( msg);
        if( MIN_CLIENT_VER < MAX_CLIENT_VER) {
            msg << 'v' << MIN_CLIENT_VER << ".." << MAX_CLIENT_VER;
        }
        else {
            msg << 'v' << MIN_CLIENT_VER;
        }
        if( !m_connectOptions.empty()) {
            msg << ' ' << m_connectOptions;
        }

        rval = closeAndSend( msg.str(), sizeof(API_SIGN)) ? 1 : -1;
    }
    else {
        ENCODE_FIELD( CLIENT_VERSION);

        rval = bufferedSend( msg.str());
    }

    m_connState = rval > 0 ? CS_CONNECTED : CS_DISCONNECTED;

    return rval;
}
