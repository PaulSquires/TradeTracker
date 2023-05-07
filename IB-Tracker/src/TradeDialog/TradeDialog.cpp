
#include "pch.h"

#include "TradeDialog.h"
#include "TradeDialogControls.h"
#include "..\MainWindow\MainWindow.h"
#include "..\Utilities\ListBoxData.h"
#include "..\TradesPanel\TradesPanel.h"


HWND HWND_TRADEDIALOG = NULL;

extern HWND HWND_MAINWINDOW;

CTradeDialog TradeDialog;

COLORREF CtrlTextColor = GetThemeCOLORREF(ThemeElement::TradesPanelText);
COLORREF CtrlTextBack = GetThemeCOLORREF(ThemeElement::TradesPanelBack);
HBRUSH CtrlBackBrush = NULL;

int tradeAction = ACTION_NOACTION;


// ========================================================================================
// Process WM_CLOSE message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnClose(HWND hwnd)
{
    MainWindow_BlurPanels(false);
    EnableWindow(HWND_MAINWINDOW, TRUE);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnDestroy(HWND hwnd)
{
    DeleteObject(CtrlBackBrush);

    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: TradeDialog
// ========================================================================================
BOOL TradeDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_TRADEDIALOG = hwnd;

    CtrlBackBrush = CreateSolidBrush(CtrlTextBack);
    TradeDialogControls_CreateControls(hwnd);

    return TRUE;
}


// ========================================================================================
// Process WM_CTLCOLORBTN message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetTextColor(hdc, CtrlTextColor);
    SetBkColor(hdc, CtrlTextBack);
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_CTLCOLOREDIT message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetTextColor(hdc, CtrlTextColor);

    // If this is the Total TextBox then display text as Red/Green 
    // depending on the status of the Debit/Credit combobox.
    if (GetDlgCtrlID(hwndChild) == IDC_TRADEDIALOG_TXTTOTAL) {
        std::wstring wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_TRADEDIALOG_COMBODRCR));
        if (wszText == L"DEBIT") SetTextColor(hdc, GetThemeCOLORREF(ThemeElement::valueNegative));
        if (wszText == L"CREDIT") SetTextColor(hdc, GetThemeCOLORREF(ThemeElement::valuePositive));
    }

    SetBkColor(hdc, CtrlTextBack);
    SetBkMode(hdc, OPAQUE);
    return CtrlBackBrush;
}


// ========================================================================================
// Process WM_CTLCOLORSTATIC message for window/dialog: TradeDialog
// ========================================================================================
HBRUSH TradeDialog_OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetTextColor(hdc, CtrlTextColor);
    SetBkColor(hdc, CtrlTextBack);
    return CtrlBackBrush;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: TradeDialog
// ========================================================================================
BOOL TradesDialog_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: TradeDialog
// ========================================================================================
void TradesDialog_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);

    DWORD nBackColor = GetThemeColor(ThemeElement::TradesPanelBack);

    // Create the background brush
    SolidBrush backBrush(nBackColor);
    graphics.FillRectangle(&backBrush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    // Copy the entire memory bitmap to the main display
    BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, memDC, 0, 0, SRCCOPY);

    // Restore the original state of the DC
    RestoreDC(hdc, -1);

    // Cleanup
    DeleteObject(hbit);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_NOTIFY message for window/dialog: TradeDialog
// ========================================================================================
LRESULT TradeDialog_OnNotify(HWND hwnd, int id, LPNMHDR lpNMHDR)
{
    //switch (lpNMHDR->code)
    //{
    //case (DTN_DATETIMECHANGE):
    //    if (lpNMHDR->idFrom == IDC_TRADEDIALOG_TRANSDATE ||
    //        (lpNMHDR->idFrom >= IDC_TRADEDIALOG_TABLEEXPIRY && lpNMHDR->idFrom <= IDC_TRADEDIALOG_TABLEEXPIRY + 10)) {
    //        CalculateTradeDTE(hwnd);
    //        return 0;
    //    }
    //    break;
    //}

    return 0;
}



// ========================================================================================
// Process WM_COMMAND message for window/dialog: TradeDialog
// ========================================================================================
void TradeDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case (IDC_TRADEDIALOG_SAVE):
        if (codeNotify == BN_CLICKED) {
            // SaveDatabase();
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    //case (IDC_TRADEDIALOG_CANCEL):
    //    if (codeNotify == BN_CLICKED) {
    //        SendMessage(hwnd, WM_CLOSE, 0, 0);
    //    }
    //    break;

    case (IDC_TRADEDIALOG_COMBODRCR):
        if (codeNotify == CBN_SELCHANGE) {
            CalculateTradeTotal(hwnd);
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
        HANDLE_MSG(m_hwnd, WM_COMMAND, TradeDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_NOTIFY, TradeDialog_OnNotify);
        HANDLE_MSG(m_hwnd, WM_DESTROY, TradeDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, TradeDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORBTN, TradeDialog_OnCtlColorBtn);
        HANDLE_MSG(m_hwnd, WM_CTLCOLOREDIT, TradeDialog_OnCtlColorEdit);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORSTATIC, TradeDialog_OnCtlColorStatic);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, TradesDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, TradesDialog_OnPaint);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


// ========================================================================================
// Create and show the Trade modal dialog.
// ========================================================================================
void TradeDialog_Show(int inTradeAction)
{
    tradeAction = inTradeAction;

    int nWidth = 575;
    int nHeight = 405;

    if (inTradeAction == ACTION_ROLL_LEG) nHeight += 100;

    HWND hwnd = TradeDialog.Create(HWND_MAINWINDOW, L"Trade Management", 0, 0, nWidth, nHeight,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // If we are using a dark theme then attempt to apply the standard Windows dark theme
    // to the non-client areas of the main form.
    BOOL value = GetIsThemeDark();
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(TradeDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    AfxCenterWindow(hwnd, HWND_MAINWINDOW);

    EnableWindow(HWND_MAINWINDOW, FALSE);
    

    // Show the legsEdit legs (if any) based on the incoming action.
    LoadEditLegsInTradeTable(hwnd);

    
    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to TradeDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow_BlurPanels(true);


    ShowWindow(hwnd, SW_SHOWNORMAL);
    
    
    // set focus to the Transaction date picker
    if (inTradeAction == ACTION_ROLL_LEG) {
        SetFocus(GetDlgItem(hwnd, IDC_TRADEDIALOG_TRANSDATE));
    }
    else {
        SetFocus(GetDlgItem(hwnd, IDC_TRADEDIALOG_TXTTICKER));
    }
     
   
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

