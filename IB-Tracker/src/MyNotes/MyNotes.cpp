/*

MIT License

Copyright(c) 2023 Paul Squires

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
#include "CustomLabel/CustomLabel.h"
#include "CustomTextBox/CustomTextBox.h"
#include "Utilities/ListBoxData.h"
#include "Utilities/AfxWin.h"
#include "MyNotes.h"


HWND HWND_MYNOTES = NULL;

CMyNotes MyNotes;

void MyNotes_OnSize(HWND hwnd, UINT state, int cx, int cy);


// ========================================================================================
// Process WM_ERASEBKGND message for window/dialog: MyNotes
// ========================================================================================
BOOL MyNotes_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    // Handle all of the painting in WM_PAINT
    return TRUE;
}


// ========================================================================================
// Process WM_PAINT message for window/dialog: MyNotes
// ========================================================================================
void MyNotes_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    // Create the background brush
    SolidBrush back_brush(COLOR_GRAYDARK);

    // Paint the background using brush.
    int nWidth = (ps.rcPaint.right - ps.rcPaint.left);
    int nHeight = (ps.rcPaint.bottom - ps.rcPaint.top);
    graphics.FillRectangle(&back_brush, ps.rcPaint.left, ps.rcPaint.top, nWidth, nHeight);

    EndPaint(hwnd, &ps);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: MyNotes
// ========================================================================================
void MyNotes_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    static bool is_notes_dirty = false;
    static std::wstring notes = L"";

    switch (id)
    {
    case (IDC_MYNOTES_TXTNOTES):
        if (codeNotify == EN_KILLFOCUS) {
            if (is_notes_dirty == true) {

                // Save the database because the Notes for this Trade has been updated.
                //SaveDatabase();

                is_notes_dirty = false;
                notes = L"";
            }
            break;
        }

        if (codeNotify == EN_CHANGE) {
            notes = AfxGetWindowText(hwndCtl);
            is_notes_dirty = true;
            break;
        }

        if (codeNotify == EN_SETFOCUS) {
            is_notes_dirty = false;
            break;
        }

        break;

    }
}


// ========================================================================================
// Process WM_SIZE message for window/dialog: MyNotes
// ========================================================================================
void MyNotes_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hNotesLabel = GetDlgItem(hwnd, IDC_MYNOTES_LBLNOTES);
    HWND hNotesTextBox = GetDlgItem(hwnd, IDC_MYNOTES_TXTNOTES);

    int margin = AfxScaleY(MYNOTES_MARGIN);
    
    HDWP hdwp = BeginDeferWindowPos(10);

    // Move and size the top label into place
    hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_MYNOTES_LBLNOTES), 0,
        0, 0, cx, margin, SWP_NOZORDER | SWP_SHOWWINDOW);

    int height_notes_textbox = AfxScaleY(100);
    int height_notes_label = AfxScaleY(24);

    int nLeft = 0;
    int nTop = cy - height_notes_textbox - height_notes_label;
    int nWidth = cx;
    hdwp = DeferWindowPos(hdwp, hNotesLabel, 0, nLeft, nTop, nWidth, height_notes_label, SWP_NOZORDER | SWP_SHOWWINDOW);

    nLeft = 0;
    nTop = cy - height_notes_textbox;
    nWidth = cx;
    hdwp = DeferWindowPos(hdwp, hNotesTextBox, 0, nLeft, nTop, nWidth, height_notes_textbox, SWP_NOZORDER | SWP_SHOWWINDOW);

    EndDeferWindowPos(hdwp);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: MyNotes
// ========================================================================================
BOOL MyNotes_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HWND_MYNOTES = hwnd;

    HWND hCtl = CustomLabel_SimpleLabel(hwnd, IDC_MYNOTES_LBLNOTES, L"My Notes",
        COLOR_WHITELIGHT, COLOR_BLACK);

    hCtl = CreateCustomTextBox(hwnd, IDC_MYNOTES_TXTNOTES, true, ES_LEFT,
        L"", 0, 0, 0, 0);
    CustomTextBox_SetMargins(hCtl, 3, 3);
    CustomTextBox_SetColors(hCtl, COLOR_WHITELIGHT, COLOR_GRAYMEDIUM);

    return TRUE;
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CMyNotes::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, MyNotes_OnCreate);
        HANDLE_MSG(m_hwnd, WM_ERASEBKGND, MyNotes_OnEraseBkgnd);
        HANDLE_MSG(m_hwnd, WM_PAINT, MyNotes_OnPaint);
        HANDLE_MSG(m_hwnd, WM_COMMAND, MyNotes_OnCommand);
        HANDLE_MSG(m_hwnd, WM_SIZE, MyNotes_OnSize);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

