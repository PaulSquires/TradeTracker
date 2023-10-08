/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

/* not using "#pragma once" on purpose! */

#if ! defined(EWRAPPER_VIRTUAL_IMPL)
# define EWRAPPER_VIRTUAL_IMPL
#endif


virtual void tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attrib) EWRAPPER_VIRTUAL_IMPL;
virtual void connectionClosed() EWRAPPER_VIRTUAL_IMPL;
virtual void connectAck() EWRAPPER_VIRTUAL_IMPL;
virtual void error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) EWRAPPER_VIRTUAL_IMPL;
virtual void position( const std::string& account, const Contract& contract, Decimal position, double avgCost) EWRAPPER_VIRTUAL_IMPL;
virtual void positionEnd() EWRAPPER_VIRTUAL_IMPL;


virtual void tickSize(TickerId tickerId, TickType field, Decimal size) EWRAPPER_VIRTUAL_IMPL;
virtual void tickGeneric(TickerId tickerId, TickType tickType, double value) EWRAPPER_VIRTUAL_IMPL;
virtual void tickString(TickerId tickerId, TickType tickType, const std::string& value) EWRAPPER_VIRTUAL_IMPL;
virtual void tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) EWRAPPER_VIRTUAL_IMPL;
virtual void winError( const std::string& str, int lastError) EWRAPPER_VIRTUAL_IMPL;
virtual void updateAccountValue(const std::string& key, const std::string& val,
const std::string& currency, const std::string& accountName) EWRAPPER_VIRTUAL_IMPL;
virtual void updateAccountTime(const std::string& timeStamp) EWRAPPER_VIRTUAL_IMPL;
virtual void updatePortfolio( const Contract& contract, Decimal position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const std::string& accountName) EWRAPPER_VIRTUAL_IMPL;
virtual void nextValidId( OrderId orderId) EWRAPPER_VIRTUAL_IMPL;
virtual void currentTime(long time) EWRAPPER_VIRTUAL_IMPL;
virtual void accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency) EWRAPPER_VIRTUAL_IMPL;
virtual void accountSummaryEnd( int reqId) EWRAPPER_VIRTUAL_IMPL;

#undef EWRAPPER_VIRTUAL_IMPL
