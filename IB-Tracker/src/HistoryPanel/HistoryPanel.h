#pragma once

//#include "..\Utilities\CWindow.h"


const int IDC_HISTORYPANEL = 102;

const int HISTORYPANEL_WIDTH = 300;

const int NUM_COLUMNS = 8;


class CHistoryPanel
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

    static void OnDestroy(HWND hwnd);
    static BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
    static void OnPaint(HWND hwnd);
    static void OnSize(HWND hwnd, UINT state, int cx, int cy);

    static void Show(HWND hWndParent);


    int nColWidth[NUM_COLUMNS] =
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

    CWindow* m_pWindow = nullptr;

public:
    CHistoryPanel(HWND hWndParent);
    ~CHistoryPanel();

};

