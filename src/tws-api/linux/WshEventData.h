/* Copyright (C) 2022 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once
#ifndef TWS_API_CLIENT_WSHEVENTDATA_H
#define TWS_API_CLIENT_WSHEVENTDATA_H

#include <string>

struct WshEventData
{
    int conId;
    std::string filter;
    bool fillWatchlist;
    bool fillPortfolio;
    bool fillCompetitors;
    std::string startDate;
    std::string endDate;
    int totalLimit;

	WshEventData(int conId, bool fillWatchlist, bool fillPortfolio, bool fillCompetitors, std::string startDate, std::string endDate, int totalLimit)
	{
        this->conId = conId;
        this->filter = "";
        this->fillWatchlist = fillWatchlist;
        this->fillPortfolio = fillPortfolio;
        this->fillCompetitors = fillCompetitors;
        this->startDate = startDate;
        this->endDate = endDate;
        this->totalLimit = totalLimit;
    }

    WshEventData(std::string filter, bool fillWatchlist, bool fillPortfolio, bool fillCompetitors, std::string startDate, std::string endDate, int totalLimit)
    {
        this->conId = INT_MAX;
        this->filter = filter;
        this->fillWatchlist = fillWatchlist;
        this->fillPortfolio = fillPortfolio;
        this->fillCompetitors = fillCompetitors;
        this->startDate = startDate;
        this->endDate = endDate;
        this->totalLimit = totalLimit;
    }
};

#endif // wsheventdata_def