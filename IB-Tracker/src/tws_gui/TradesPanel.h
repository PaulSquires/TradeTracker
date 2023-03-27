#pragma once

#include "pch.h"
#include "CWindow.h"


const int IDC_TRADESPANEL = 101;

const int IDC_LISTBOX = 102;

// Construct the Trades ListBox data structure (Vector) that will be directly accessed
// for each row during the WM_DRAWITEM notification. The message will specify which ListBox
// line it is attempting to draw. We simply get that line information from the following
// display structure vector. Likewise, when new price data is received from TWS, it will also
// directly reference a specific ListBox line because we would have set the requested tickerId
// to be the same as the line number of the ticker in the ListBox. Need to just invalidate
// that ListBox line in order to display the updated price information.

enum class LineType {
    blank,
    optionHeader,
    optionLeg,
    shares
};

struct OpenTradesStruct{
    LineType        lineType;
    TickerId        tickerId;          // Column 0  TWS tickerId (same as ListBox line number)
    Trade* trade;

    // LineType::optionHeader
    std::wstring    symbol;            // Column 0
    std::wstring    ITM;               // Column 1  'ITM' or blank
    DWORD           clrITM;            // ARGB red or green
    // Column 2  blank 
    // Column 3  blank 
    std::wstring    priceChange;       // Column 4
    DWORD           clrPriceChange;    // ARGB red or green
    std::wstring    priceLast;         // Column 5  most recent received price data
    std::wstring    percentChange;     // Column 6
    DWORD           clrPercentChange;  // ARGB red or green


    // LineType::optionLeg
                                       // Column 0  blank 
    std::wstring    dot;               // Column 1
    DWORD           clrDot;            // ARGB magenta or yellow (2 days or less DTE)
    std::wstring    position;          // Column 2
    std::wstring    expiryDate;        // Column 3  (short date)
    std::wstring    DTE;               // Column 4  days to expiration
    std::wstring    strikePrice;       // Column 5
    std::wstring    PutCall;           // Column 6


    // LineType::shares
                                       // Column 0  blank 
    // dot (from above)                // Column 1  dot
    // clrDot (from above)             // ARGB magenta (always)
    std::wstring    shares;            // Column 2  'SHARES' text 
                                       // Column 3  blank (darkGrayBack) 
                                       // Column 4  blank (darkGrayBack)
    std::wstring    ACB;               // Column 5  avg Cost Base of shares
    std::wstring    numShares;         // Column 6  total aggregate share quantity

};


CWindow* TradesPanel_Show(HWND hWndParent);
void ShowActiveTradesTable();

