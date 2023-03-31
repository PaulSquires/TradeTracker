#pragma once

#include "..\Utilities\CWindow.h"

const int IDC_NAVPANEL = 100;

const int IDC_NAVPANEL_LOGO          = 101;
const int IDC_NAVPANEL_GEARICON      = 102;
const int IDC_NAVPANEL_USERNAME      = 103;
const int IDC_NAVPANEL_APPNAME       = 104;
const int IDC_NAVPANEL_ACTIVETRADES  = 105;
const int IDC_NAVPANEL_CLOSEDTRADES  = 106;
const int IDC_NAVPANEL_NEWTRADE      = 107;
const int IDC_NAVPANEL_SHORTSTRANGLE = 108;
const int IDC_NAVPANEL_SHORTPUT      = 109;
const int IDC_NAVPANEL_SHORTCALL     = 110;
const int IDC_NAVPANEL_TICKERTOTALS  = 111;
const int IDC_NAVPANEL_DAILYTOTALS   = 112;
const int IDC_NAVPANEL_RECONCILE     = 113;
const int IDC_NAVPANEL_CONFIGURE     = 114;
const int IDC_NAVPANEL_MESSAGES      = 115;

const int NAVPANEL_WIDTH = 180;

class CNavPanel
{
private:

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static void OnDestroy(HWND hwnd);
    static BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
    static void OnPaint(HWND hwnd);
    static void OnSize(HWND hwnd, UINT state, int cx, int cy);

    static void Show(HWND hWndParent);

    CWindow* m_pWindow = nullptr;

public:
    CNavPanel(HWND hWndParent);
    ~CNavPanel();

};

