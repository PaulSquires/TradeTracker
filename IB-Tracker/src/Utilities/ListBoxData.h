#pragma once

#include "..\Themes\Themes.h"
#include "..\Database\trade.h"

// ListBox data structure that will be directly accessed for each row 
// during the WM_DRAWITEM notification. 

typedef long TickerId;

class ColumnData {
public:
    std::wstring        wszText;
    StringAlignment     alignment = StringAlignmentNear;  
    ThemeElement        backTheme = ThemeElement::TradesPanelBack;
    ThemeElement        textTheme = ThemeElement::TradesPanelText;;
    REAL                fontSize = 8;                    // 8, 10
    int                 fontStyle = FontStyleRegular;    // FontStyleRegular, FontStyleBold
};

class ListBoxData {
public:
    bool            isTickerLine = false;
    TickerId        tickerId = -1;
    Trade*          trade = nullptr;
    ColumnData      col[10];

    void SetData(
        int index, Trade* tradeptr, TickerId tickId,
        std::wstring wszText, StringAlignment alignment, ThemeElement backTheme,
        ThemeElement textTheme, REAL fontSize, int fontStyle)
    {
        if (tickId != -1) isTickerLine = true;
        tickerId = tickId;
        trade = tradeptr;
        col[index].wszText = wszText;
        col[index].alignment = alignment;
        col[index].backTheme = backTheme;
        col[index].textTheme = textTheme;
        col[index].fontSize = fontSize;
        col[index].fontStyle = fontStyle;
    }

    // Update Text & color only. This is called from tws-client when TWS
    // sends new price data that needs to be updated.
    void SetTextData(int index, std::wstring wszText, ThemeElement textTheme)
    {
        col[index].wszText = wszText;
        col[index].textTheme = textTheme;
    }

};

void ListBoxData_ResizeColumnWidths(HWND hListBox, int nIndex);
void ListBoxData_DestroyItemData(HWND hListBox);
void ListBoxData_OpenPosition(HWND hListBox, Trade* trade, TickerId tickerId);
void ListBoxData_HistoryHeader(HWND hListBox, Trade* trade, Transaction* trans);
void ListBoxData_HistoryOptionsLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg);
void ListBoxData_HistorySharesLeg(HWND hListBox, Trade* trade, Transaction* trans, Leg* leg);
void ListBoxData_HistoryBlankLine(HWND hListBox);
