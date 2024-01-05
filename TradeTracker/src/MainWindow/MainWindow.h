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

#pragma once

#include "Utilities/CWindowBase.h"


class CMainWindow : public CWindowBase<CMainWindow> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND hWindow = NULL;
    HWND hLeftPanel = NULL;
    HWND hRightPanel = NULL;

    void DisplayPaperTradingWarning();
    void DisplayUpdateAvailableMessage();
    void BlurPanels(bool active);
    HWND GetLeftPanel();
    void SetLeftPanel(HWND hPanel);
    void SetRightPanel(HWND hPanel);

private:
    bool OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnPaint(HWND hwnd);
    bool OnEraseBkgnd(HWND hwnd, HDC hdc);
    void OnClose(HWND hwnd);
    void OnDestroy(HWND hwnd);
    void OnLButtonDown(HWND hwnd, bool fDoubleClick, int x, int y, UINT keyFlags);
    void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
    void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
    bool OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);

    bool IsMouseOverSplitter(HWND hwnd);
    void UpdateSplitterChildren(HWND hwnd, int xdelta);
};

class CMainWindowShadow : public CWindowBase<CMainWindowShadow> {
public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#define IDI_MAINICON 106
#define IDB_CONNECT 110
#define IDB_DISCONNECT 111
#define IDB_RECONCILE 114
#define IDB_SETTINGS 115


constexpr int IDC_MAINWINDOW_WARNING = 100;
constexpr int IDC_MAINWINDOW_UPDATEAVAILABLE = 101;

constexpr int APP_LEFTMARGIN_WIDTH = 24;

extern CMainWindow MainWindow;
