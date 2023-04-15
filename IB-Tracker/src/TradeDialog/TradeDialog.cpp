
#include "pch.h"

#include "TradeDialog.h"
#include "TradeDialogControls.h"
#include "..\MainWindow\MainWindow.h"
#include "..\Utilities\ListBoxData.h"


HWND HWND_TRADEDIALOG = NULL;

extern HWND HWND_MAINWINDOW;

CTradeDialog TradeDialog;




// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
    lpMeasureItem->itemHeight = AfxScaleY(TRADEDIALOG_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnClose(HWND hwnd)
{
    EnableWindow(HWND_MAINWINDOW, TRUE);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnDestroy(HWND hwnd)
{
    // Clear the Trade Templates Linedata held in the ListBox
    ListBoxData_DestroyItemData(GetDlgItem(hwnd, IDC_TRADEDIALOG_LISTBOX));

    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    TradeDialogControls_SizeControls(hwnd, cx, cy);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
BOOL TradeDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRADEDIALOG = hwnd;

    TradeDialogControls_CreateControls(hwnd);

    return TRUE;
}


// ========================================================================================
// Process WM_CTLCOLORBTN message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_CTLCOLOREDIT message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_CTLCOLORSTATIC message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case (IDC_TRADEDIALOG_OK):
        if (codeNotify == BN_CLICKED) {
            // SaveDatabase();
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    case (IDC_TRADEDIALOG_CANCEL):
        if (codeNotify == BN_CLICKED) {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CTradeDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, TradeDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_SIZE, TradeDialog_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradeDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_DESTROY, TradeDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, TradeDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORBTN, TradeDialog_OnCtlColorBtn);
        HANDLE_MSG(m_hwnd, WM_CTLCOLOREDIT, TradeDialog_OnCtlColorEdit);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORSTATIC, TradeDialog_OnCtlColorStatic);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, TradeDialog_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ListBoxData_OnDrawItem);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Trade modal dialog.
// ========================================================================================
void TradeDialog_Show()
{
    HWND hwnd = TradeDialog.Create(HWND_MAINWINDOW, L"Trade", 0, 0, 800, 400,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    HANDLE hIconSmall = LoadImage(TradeDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    AfxCenterWindow(hwnd, HWND_MAINWINDOW);

    EnableWindow(HWND_MAINWINDOW, FALSE);

    ShowWindow(hwnd, SW_SHOWNORMAL);

    // Need to force a resize of the HistoryPanel in order to properly show (or not show) 
    // and position the Summary listbox and daily detail listbox.
    RECT rc; GetClientRect(hwnd, &rc);
    TradeDialog_OnSize(hwnd, 0, rc.right, rc.bottom);

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Determines whether a message is intended for the specified
        // dialog box and, if it is, processes the message.
        if (!IsDialogMessage(hwnd, &msg)) {
            // Translates virtual-key messages into character messages.
            TranslateMessage(&msg);
            // Dispatches a message to a window procedure.
            DispatchMessage(&msg);
        }
    }

}

