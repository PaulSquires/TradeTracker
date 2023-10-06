/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "pch.h"
#include "DefaultEWrapper.h"

void DefaultEWrapper::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) { }
void DefaultEWrapper::position( const std::string& account, const Contract& contract, Decimal position, double avgCost) { }
void DefaultEWrapper::positionEnd() { }
void DefaultEWrapper::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) { }
void DefaultEWrapper::connectionClosed() { }
void DefaultEWrapper::connectAck() { }


void DefaultEWrapper::tickSize( TickerId tickerId, TickType field, Decimal size) { }
void DefaultEWrapper::tickGeneric(TickerId tickerId, TickType tickType, double value) { }
void DefaultEWrapper::tickString(TickerId tickerId, TickType tickType, const std::string& value) { }
void DefaultEWrapper::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) { }
void DefaultEWrapper::winError( const std::string& str, int lastError) { }
void DefaultEWrapper::updateAccountValue(const std::string& key, const std::string& val,
   const std::string& currency, const std::string& accountName) { }
void DefaultEWrapper::updatePortfolio(const Contract& contract, Decimal position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const std::string& accountName) { }
void DefaultEWrapper::nextValidId( OrderId orderId) { }
void DefaultEWrapper::currentTime(long time) { }
void DefaultEWrapper::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency) { }
void DefaultEWrapper::accountSummaryEnd( int reqId) { }
