/*

MIT License

Copyright(c) 2023-2024 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "pch.h"

#include "Database/trade.h"
#include "Database/database.h"
#include "CustomLabel/CustomLabel.h"
#include "CustomCalendar/CustomCalendar.h"
#include "CustomTextBox/CustomTextBox.h"
#include "CustomMessageBox/CustomMessageBox.h"
#include "CustomVScrollBar/CustomVScrollBar.h"
#include "Utilities/UserMessages.h"
#include "Config/Config.h"
#include "tws-api/IntelDecimal/IntelDecimal.h"
#include "MainWindow/MainWindow.h"
#include "MainWindow/tws-client.h"

#include "ImportTrades.h"


HWND HWND_IMPORTDIALOG = NULL;

CImportDialog ImportDialog;

extern int dialog_return_code;

bool show_import_dialog = true;

std::vector<ImportStruct> ibkr;    // persistent 



// ========================================================================================
// Information received from TwsClient::position callback
// ========================================================================================
void ImportTrades_position(const Contract& contract, Decimal position, double avg_cost) {
	// This callback is initiated by the reqPositions() call when the user requests to
	// import existing IBR positions (i.e. the database is empty).

	// Remove any existing version of the contract before adding it again
	auto end = std::remove_if(ibkr.begin(),
		ibkr.end(),
		[contract](ImportStruct const& p) {
			return (p.contract.conId == contract.conId) ? true : false;
		});
	ibkr.erase(end, ibkr.end());

	// Add the position the vector
	ImportStruct p{};
	p.contract = contract;
	p.position = position;
	p.avg_cost = avg_cost;
	ibkr.push_back(p);
}


// ========================================================================================
// Sort all IBKR positions in order to start to group similar trades together.
// ========================================================================================
void ImportTrades_doPositionSorting(){

	// Sort by Symbol, secType, lastTradeDateOrContractMonth
	// Sort based on Category and then TickerSymbol
	std::sort(ibkr.begin(), ibkr.end(),
		[](const auto& trade1, const auto& trade2) {
			{
				if (trade1.contract.symbol < trade2.contract.symbol) return true;
				if (trade2.contract.symbol < trade1.contract.symbol) return false;

				// symbol=symbol for primary condition, go to secType
				if (trade1.contract.secType < trade2.contract.secType) return true;
				if (trade2.contract.secType < trade1.contract.secType) return false;

				// secType=secType for primary condition, go to lastTradeDateOrContractMonth
				if (trade1.contract.lastTradeDateOrContractMonth < trade2.contract.lastTradeDateOrContractMonth) return true;
				if (trade2.contract.lastTradeDateOrContractMonth < trade1.contract.lastTradeDateOrContractMonth) return false;

				return false;
			}
		});

	for (const auto& p : ibkr) {
		std::cout <<
		p.contract.conId << " " <<
		p.contract.symbol << " " <<
		p.contract.secType << " " <<
		p.contract.lastTradeDateOrContractMonth << " " <<
		p.contract.strike << " " <<
		p.contract.right << " " <<
		(int)intelDecimalToDouble(p.position) << " " <<
		p.avg_cost << " " <<
		std::endl;
	}
}


// ========================================================================================
// Ask to initiate importing trades from IBKR TWS.
// ========================================================================================
void ImportTrades_AskImportMessage() {
    if (!show_import_dialog) return;

    std::wstring text =
        L"No trades exist in the local database.\n\n" \
        "Would you like to connect to TWS and import existing positions from your online IBKR account?";

    int res = CustomMessageBox.Show(
        MainWindow.hWindow, text, L"Import Trades", MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_DEFBUTTON1);

    if (res != IDYES) return;

    client.connection_type = ConnectionType::tws_data_live;
    tws_Connect();
    
    tws_RequestPositions();

    // When positions are finished being received, MSG_POSITIONS_READY is sent to ActiveTrades
    // window. This is where the ImportDialog_Show() call is made.
}


// ========================================================================================
// Header control subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ImportDialog_Header_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {

    case WM_ERASEBKGND: {
        return true;
    }

    case WM_PAINT: {
        Header_OnPaint(hwnd);
        return 0;
    }

    case WM_DESTROY: {
        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, ImportDialog_Header_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Listbox subclass Window procedure
// ========================================================================================
LRESULT CALLBACK ImportDialog_ListBox_SubclassProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Create static accumulation variable to collect the data from
    // a series of middle mouse wheel scrolls.
    static int accum_delta = 0;

    switch (uMsg) {

    case WM_MOUSEWHEEL: {
        // Accumulate delta until scroll one line (up +120, down -120). 
        // 120 is the Microsoft default delta
        int zdelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        accum_delta += zdelta;
        if (accum_delta >= 120) {     // scroll up 3 lines
            top_index -= 3;
            top_index = max(0, top_index);
            SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
            accum_delta = 0;
        }
        else {
            if (accum_delta <= -120) {     // scroll down 3 lines
                top_index += +3;
                SendMessage(hwnd, LB_SETTOPINDEX, top_index, 0);
                accum_delta = 0;
            }
        }
        HWND hCustomVScrollBar = GetDlgItem(GetParent(hwnd), IDC_IMPORTDIALOG_CUSTOMVSCROLLBAR);
        CustomVScrollBar_Recalculate(hCustomVScrollBar);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int idx = Listbox_ItemFromPoint(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        // The return value contains the index of the nearest item in the LOWORD. The HIWORD is zero 
        // if the specified point is in the client area of the list box, or one if it is outside the 
        // client area.
        if (HIWORD(idx) == 1) break;

        SendMessage(GetParent(hwnd), MSG_CLOSEDTRADES_SETSHOWTRADEDETAIL, true, 0);
        SendMessage(GetParent(hwnd), MSG_CLOSEDTRADES_SHOWLISTBOXITEM, idx, 0);
        break;
    }

    case WM_ERASEBKGND: {
        // If the number of lines in the listbox maybe less than the number per page then 
        // calculate from last item to bottom of listbox, otherwise calculate based on
        // the mod of the lineheight to listbox height so we can color the partial line
        // that won't be displayed at the bottom of the list.
        RECT rc; GetClientRect(hwnd, &rc);

        RECT rcItem{};
        SendMessage(hwnd, LB_GETITEMRECT, 0, (LPARAM)&rcItem);
        int item_height = (rcItem.bottom - rcItem.top);
        int items_count = ListBox_GetCount(hwnd);
        int top_index = (int)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
        int visible_rows = 0;
        int items_per_page = 0;
        int bottom_index = 0;
        int width = (rc.right - rc.left);
        int height = (rc.bottom - rc.top);

        if (items_count > 0) {
            items_per_page = (height) / item_height;
            bottom_index = (top_index + items_per_page);
            if (bottom_index >= items_count)
                bottom_index = items_count - 1;
            visible_rows = (bottom_index - top_index) + 1;
            rc.top = visible_rows * item_height;
        }

        if (rc.top < rc.bottom) {
            height = (rc.bottom - rc.top);
            HDC hDC = (HDC)wParam;
            Graphics graphics(hDC);
            SolidBrush back_brush(COLOR_GRAYDARK);
            graphics.FillRectangle(&back_brush, rc.left, rc.top, width, height);
        }

        ValidateRect(hwnd, &rc);
        return true;
    }

    case WM_DESTROY: {
        // Destroy all manually allocated ListBox display data that is held
        // in the LineData structures..
        ListBoxData_DestroyItemData(hwnd);

        // REQUIRED: Remove control subclassing
        RemoveWindowSubclass(hwnd, ImportDialog_ListBox_SubclassProc, uIdSubclass);
        break;
    }

    }   // end of switch statment

    // For messages that we don't deal with
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


// ========================================================================================
// Process WM_MEASUREITEM message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* lpMeasureItem) {
    lpMeasureItem->itemHeight = AfxScaleY(IMPORTDIALOG_LISTBOX_ROWHEIGHT);
}


// ========================================================================================
// Process WM_CLOSE message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnClose(HWND hwnd) {
    if (dialog_return_code == DIALOG_RETURN_OK) {
    }

    MainWindow.BlurPanels(false);
    EnableWindow(MainWindow.hWindow, true);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnDestroy(HWND hwnd) {
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnSize(HWND hwnd, UINT state, int cx, int cy) {

    HWND hListBox = GetDlgItem(hwnd, IDC_IMPORTDIALOG_LISTBOX);
    HWND hHeader = GetDlgItem(hwnd, IDC_IMPORTDIALOG_HEADER);
    HWND hVScrollBar = GetDlgItem(hwnd, IDC_IMPORTDIALOG_CUSTOMVSCROLLBAR);

    // Do not call the calcVThumbRect() function during a scrollbar move. This WM_SIZE
    // gets triggered when the ListBox WM_DRAWITEM fires. If we do another calcVThumbRect()
    // calcualtion then the scrollbar will appear "jumpy" under the user's mouse cursor.
    bool bshow_scrollbar = false;
    CustomVScrollBar* pData = CustomVScrollBar_GetPointer(hVScrollBar);
    if (pData) {
        if (pData->drag_active) {
            bshow_scrollbar = true;
        }
        else {
            bshow_scrollbar = pData->calcVThumbRect();
        }
    }
    int custom_scrollbar_width = bshow_scrollbar ? AfxScaleX(CUSTOMVSCROLLBAR_WIDTH) : 0;

    //int margin = AfxScaleY(CLOSEDTRADES_MARGIN);
    int left = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    int top = 0;
    int width = cx;
    int height = 0;

    HDWP hdwp = BeginDeferWindowPos(10);

    left = AfxScaleX(APP_LEFTMARGIN_WIDTH);
    width = cx - left;
    height = AfxScaleY(IMPORTDIALOG_LISTBOX_ROWHEIGHT);
    hdwp = DeferWindowPos(hdwp, hHeader, 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
    top = top + height + AfxScaleY(1);

    width = cx - left - custom_scrollbar_width;
    height = cy - top;
    hdwp = DeferWindowPos(hdwp, hListBox, 0, left, top, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

    left = left + width;   // right edge of ListBox
    width = custom_scrollbar_width;
    hdwp = DeferWindowPos(hdwp, hVScrollBar, 0, left, top, width, height,
        SWP_NOZORDER | (bshow_scrollbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ImportDialog
// ========================================================================================
bool ImportDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
    HWND_IMPORTDIALOG = hwnd;

    std::wstring font_name = AfxGetDefaultFont();
    int font_size = 9;

    ImportTrades_doPositionSorting();

    HWND hCtl = ImportDialog.AddControl(Controls::Header, hwnd, IDC_IMPORTDIALOG_HEADER, L"",
        0, 0, 0, 0, -1, -1, NULL, (SUBCLASSPROC)ImportDialog_Header_SubclassProc,
        IDC_IMPORTDIALOG_HEADER, NULL);
    Header_InsertNewItem(hCtl, 0, AfxScaleX(nClosedMinColWidth[0]), L"", HDF_CENTER);
    Header_InsertNewItem(hCtl, 1, AfxScaleX(nClosedMinColWidth[1]), L"Date", HDF_LEFT);
    Header_InsertNewItem(hCtl, 2, AfxScaleX(nClosedMinColWidth[2]), L"Ticker", HDF_LEFT);
    Header_InsertNewItem(hCtl, 3, AfxScaleX(nClosedMinColWidth[3]), L"Description", HDF_LEFT);
    Header_InsertNewItem(hCtl, 4, AfxScaleX(nClosedMinColWidth[4]), L"Amount", HDF_RIGHT);
    // Must turn off Window Theming for the control in order to correctly apply colors
    SetWindowTheme(hCtl, L"", L"");

    // Create an Ownerdraw fixed row sized listbox that we will use to custom
    // paint our various closed trades.
    hCtl =
        ImportDialog.AddControl(Controls::ListBox, hwnd, IDC_IMPORTDIALOG_LISTBOX, L"",
            0, 0, 0, 0,
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP |
            LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY,
            WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR, NULL,
            (SUBCLASSPROC)ImportDialog_ListBox_SubclassProc,
            IDC_IMPORTDIALOG_LISTBOX, NULL);
    ListBox_AddString(hCtl, NULL);

    // Create our custom vertical scrollbar and attach the ListBox to it.
    CreateCustomVScrollBar(hwnd, IDC_IMPORTDIALOG_CUSTOMVSCROLLBAR, hCtl, Controls::ListBox);

    // SAVE button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_IMPORTDIALOG_SAVE, L"SAVE",
        COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 390, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    // CANCEL button
    hCtl = CustomLabel_ButtonLabel(hwnd, IDC_IMPORTDIALOG_CANCEL, L"Cancel",
        COLOR_BLACK, COLOR_RED, COLOR_RED, COLOR_GRAYMEDIUM, COLOR_WHITE,
        CustomLabelAlignment::middle_center, 580, 423, 80, 23);
    CustomLabel_SetFont(hCtl, font_name, font_size, true);
    CustomLabel_SetTextColorHot(hCtl, COLOR_WHITELIGHT);

    return true;
}


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: ImportDialog
// ========================================================================================
bool ImportDialog_OnEraseBkgnd(HWND hwnd, HDC hdc) {
    // Handle all of the painting in WM_PAINT
    return true;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    SaveDC(hdc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hbit = CreateCompatibleBitmap(hdc, ps.rcPaint.right, ps.rcPaint.bottom);
    SelectBitmap(memDC, hbit);

    Graphics graphics(memDC);
    int width = (ps.rcPaint.right - ps.rcPaint.left);
    int height = (ps.rcPaint.bottom - ps.rcPaint.top);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, width, height);

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
// Process WM_COMMAND message for window/dialog: ImportDialog
// ========================================================================================
void ImportDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (codeNotify) {

    case LBN_SELCHANGE: {
        int selected = ListBox_GetCurSel(hwndCtl);
        if (selected == -1) break;
        ListBoxData* ld = (ListBoxData*)ListBox_GetItemData(hwndCtl, selected);
        if (ld) {
            // Show the trade history for the selected trade
            //SetShowTradeDetail(true);
            //ShowListBoxItem(selected);
        }
        break;
    }

    }
}


// ========================================================================================
// Process WM_DRAWITEM message for window/dialog 
// (common function for custom drawing listbox data)
// ========================================================================================
void ImportDialog_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem) {
    if (lpDrawItem->itemID == -1) return;

    if (lpDrawItem->itemAction == ODA_DRAWENTIRE ||
        lpDrawItem->itemAction == ODA_SELECT) {

        int width = (lpDrawItem->rcItem.right - lpDrawItem->rcItem.left);
        int height = (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top);

        bool is_hot = false;

        SaveDC(lpDrawItem->hDC);

        HDC memDC = NULL;         // Double buffering
        HBITMAP hbit = NULL;      // Double buffering

        memDC = CreateCompatibleDC(lpDrawItem->hDC);
        hbit = CreateCompatibleBitmap(lpDrawItem->hDC, width, height);
        if (hbit) SelectObject(memDC, hbit);

        if ((lpDrawItem->itemAction | ODA_SELECT) &&
            (lpDrawItem->itemState & ODS_SELECTED)) {
            is_hot = true;
        }

        Graphics graphics(memDC);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // Set some defaults in case there is no valid ListBox line number
        std::wstring text;

        DWORD back_color = (is_hot) ? COLOR_SELECTION : COLOR_GRAYDARK;
        DWORD text_color = COLOR_WHITELIGHT;

        std::wstring font_name = AfxGetDefaultFont();
        FontFamily   fontFamily(font_name.c_str());
        REAL font_size = 10;
        int font_style = FontStyleRegular;

        StringAlignment HAlignment = StringAlignmentNear;
        StringAlignment VAlignment = StringAlignmentCenter;

        // Paint the full width background using brush 
        SolidBrush back_brush(back_color);
        graphics.FillRectangle(&back_brush, 0, 0, width, height);

        // Get the current ListBox line data should a valid line exist
        // Paint the individual columns with the specific data.
        ListBoxData* ld = (ListBoxData*)(lpDrawItem->itemData);
        int left = 0;
        int column_width = 0;
        int column_start = 0;
        int column_end = MAX_COLUMNS;

        // If this is a Category separator line then we need to draw it differently
        // then a regular that would be drawn (full line width)
        if (ld) {
            if (ld->line_type == LineType::category_header) {
                column_start = 0;
                column_end = 1;
            }
        }

        // Draw each of the columns
        for (int i = column_start; i < column_end; ++i) {
            if (ld == nullptr) break;
            if (ld->col[i].column_width == 0) break;

            // Prepare and draw the text
            text = ld->col[i].text;

            HAlignment = ld->col[i].HAlignment;
            VAlignment = ld->col[i].VAlignment;
            back_color = (is_hot) ? back_color : ld->col[i].back_theme;
            text_color = ld->col[i].text_theme;
            font_size = ld->col[i].font_size;
            font_style = ld->col[i].font_style;

            column_width = (ld->line_type == LineType::category_header) ? width : AfxScaleX((float)ld->col[i].column_width);

            back_brush.SetColor(back_color);
            graphics.FillRectangle(&back_brush, left, 0, column_width, height);

            Font         font(&fontFamily, font_size, font_style, Unit::UnitPoint);
            SolidBrush   text_brush(text_color);
            StringFormat stringF(StringFormatFlagsNoWrap);
            stringF.SetAlignment(HAlignment);
            stringF.SetLineAlignment(VAlignment);

            // If right alignment then add a very small amount of right side
            // padding so that text is not pushed up right against the right side.
            int rightPadding = 0;
            if (HAlignment == StringAlignmentFar) rightPadding = AfxScaleX(2);

            RectF rcText((REAL)left, (REAL)0, (REAL)column_width - rightPadding, (REAL)height);
            graphics.DrawString(text.c_str(), -1, &font, rcText, &stringF, &text_brush);

            left += column_width;
        }

        BitBlt(lpDrawItem->hDC, lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        RestoreDC(lpDrawItem->hDC, -1);
        if (hbit) DeleteObject(hbit);
        if (memDC) DeleteDC(memDC);
    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CImportDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        HANDLE_MSG(m_hwnd, WM_CREATE, ImportDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_SIZE, ImportDialog_OnSize);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ImportDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_DESTROY, ImportDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, ImportDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, ImportDialog_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, ImportDialog_OnPaint);
        HANDLE_MSG(m_hwnd, WM_MEASUREITEM, ImportDialog_OnMeasureItem);
        HANDLE_MSG(m_hwnd, WM_DRAWITEM, ImportDialog_OnDrawItem);

    case WM_SHOWWINDOW: {
        // Workaround for the Windows 11 (The cloaking solution seems to work only
        // on Windows 10 whereas this WM_SHOWWINDOW workaround seems to only work
        // on Windows 11).
        // https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit

        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        if (!GetLayeredWindowAttributes(m_hwnd, NULL, NULL, NULL)) {
            HDC hdc = GetDC(m_hwnd);
            SetLayeredWindowAttributes(m_hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(m_hwnd, WM_ERASEBKGND, (WPARAM)hdc, lParam);
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(m_hwnd, 1, AW_ACTIVATE | AW_BLEND);
            ReleaseDC(m_hwnd, hdc);
            return 0;
        }
        SetWindowLongPtr(m_hwnd,
            GWL_EXSTYLE,
            GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }

    case WM_KEYDOWN: {
        // We are handling the TAB naviagation ourselves.
        if (wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            HWND hNextCtrl = NULL;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, true);
            }
            else {
                hNextCtrl = GetNextDlgTabItem(m_hwnd, hFocus, false);
            }
            SetFocus(hNextCtrl);
            return true;
        }
        break;
    }

    case MSG_DATEPICKER_DATECHANGED: {
        //TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDMAIN));
        //TradeGrid_CalculateDTE(GetDlgItem(m_hwnd, IDC_TRADEDIALOG_TABLEGRIDROLL));
        return 0;
    }

    case MSG_CUSTOMLABEL_CLICK: {
        HWND hCtl = (HWND)lParam;
        int ctrl_id = (int)wParam;

        if (!hCtl) return 0;

        //if (ctrl_id == IDC_TRADEDIALOG_COMBODRCR) {
        //}

        if (ctrl_id == IDC_IMPORTDIALOG_SAVE) {
            dialog_return_code = DIALOG_RETURN_OK;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        if (ctrl_id == IDC_IMPORTDIALOG_CANCEL) {
            dialog_return_code = DIALOG_RETURN_CANCEL;
            SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        }

        return 0;
    }

    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}


// ========================================================================================
// Create and show the Import Trades modal dialog.
// ========================================================================================
int ImportDialog_Show() {

    int width = 715;
    int height = 500;

    HWND hwnd = ImportDialog.Create(MainWindow.hWindow, L"Import Trades", 0, 0, width, height,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    HBRUSH hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrBackground);

    HANDLE hIconSmall = LoadImage(ImportDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

    // Blur the underlying MainWindow panels in order to reduce "visual noise" while
    // our Trade Management popup is active. The MainWindow panels are shown again
    // during our call to TradeDialog_OnClose() prior to enabling the MainWindow
    // and this popup closing.
    MainWindow.BlurPanels(true);

    AfxCenterWindowMonitorAware(hwnd, MainWindow.hWindow);

    EnableWindow(MainWindow.hWindow, false);

    // Fix Windows 10 white flashing
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    cloak = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));

    dialog_return_code = DIALOG_RETURN_CANCEL;

    show_import_dialog = false;

    // Call modal message pump and wait for it to end.
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE) {
            //if (IsWindowVisible(HWND_CUSTOMCALENDAR)) {
            //    DestroyWindow(HWND_CUSTOMCALENDAR);
            //}
            //else if (IsWindowVisible(HWND_STRATEGYPOPUP)) {
            //    DestroyWindow(HWND_STRATEGYPOPUP);
            //}
            //else {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            //}
        }

        // We handle VK_TAB processing ourselves rather than using IsDialogMessage
        // Translates virtual-key messages into character messages.
        TranslateMessage(&msg);
        // Dispatches a message to a window procedure.
        DispatchMessage(&msg);
    }

    return dialog_return_code;
}

