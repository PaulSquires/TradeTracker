/* Copyright (C) 2021 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once

#ifndef TWS_API_CLIENT_HISTORICALSESSION_H
#define TWS_API_CLIENT_HISTORICALSESSION_H

#include <string>

struct HistoricalSession
{
    std::string startDateTime;
    std::string endDateTime;
    std::string refDate;
};

#endif
