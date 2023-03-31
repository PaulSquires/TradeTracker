#pragma once

#include "..\Utilities\CWindow.h"
#include "..\Themes\Themes.h"
#include "..\Database\trade.h"

typedef long TickerId;

const int IDC_TRADESPANEL = 101;

const int IDC_LISTBOX = 102;
const int IDC_LABEL = 103;
const int IDC_VSCROLLBAR = 104;


// Construct the Trades ListBox data structure (Vector) that will be directly accessed
// for each row during the WM_DRAWITEM notification. The message will specify which ListBox
// line it is attempting to draw. We simply get that line information from the following
// display structure vector. Likewise, when new price data is received from TWS, it will also
// directly reference a specific ListBox line because we would have set the requested tickerId
// to be the same as the line number of the ticker in the ListBox. Need to just invalidate
// that ListBox line in order to display the updated price information.

class ColumnData {
public:
    std::wstring        wszText;
    StringAlignment     alignment = StringAlignmentNear;  // StringAlignmentNear, StringAlignmentCenter, StringAlignmentFar
    ThemeElement        backTheme = ThemeElement::TradesPanelBack;
    ThemeElement        textTheme = ThemeElement::TradesPanelText;;
    REAL                fontSize = 8;                    // 8, 10
    int                 fontStyle = FontStyleRegular;    // FontStyleRegular, FontStyleBold
};

class LineData {
public:
    bool            isTickerLine = false;
    Trade*          trade = nullptr;
    ColumnData      col[8];
    TickerId        tickerId = 0;
};

const int TICKER_NUMBER_OFFEST = 100;

// These columns in the table are updated in real time when connected
// to TWS. The LineData pointer is updated via a call to SetColumnData
// and the correct ListBox line is invalidated/redrawn in order to force
// display of the new price data. Refer to TwsClient::tickPrice in the
// tws-client.cpp file to see this in action.
const int COLUMN_TICKER_ITM          = 2;    // ITM (In the Money)
const int COLUMN_TICKER_CHANGE       = 5;    // price change
const int COLUMN_TICKER_CURRENTPRICE = 6;    // current price
const int COLUMN_TICKER_PERCENTAGE   = 7;    // price percentage change

//const int NUM_COLUMNS = 8;


class CTradesPanel
{
private:
    static bool calcVThumbRect();

    static LRESULT CALLBACK VScrollBar_SubclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    static LRESULT CALLBACK ListBox_SubclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static int CTradesPanel::OnDrawItem(HWND hWnd, DRAWITEMSTRUCT* lpdis);
    static void OnDestroy(HWND hwnd);
    static BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
    static void OnPaint(HWND hwnd);
    static void OnSize(HWND hwnd, UINT state, int cx, int cy);

    static void Show(HWND hWndParent);

    int nMinColWidth[8] =
    {
        25,     /* dropdown arrow*/
        45,     /* ticker symbol */
        50,     /* ITM */
        50,     /* position quantity */
        50,     /* expiry date */
        40,     /* DTE */
        45,     /* strike price */
        40      /* put/call */
    };
    int nColWidth[8] = {0,0,0,0,0,0,0,0};

    CWindow* m_pWindow = nullptr;

public:
    CTradesPanel(HWND hWndParent);
    ~CTradesPanel();
    
    void CalculateColumnWidths(int nIndex = -1);

};


extern std::vector<LineData*> vec;

void ShowActiveTrades();
void SetColumnData(LineData* ld, int index, std::wstring wszText, StringAlignment alignment,
    ThemeElement backTheme, ThemeElement textTheme, REAL fontSize, int fontStyle);
