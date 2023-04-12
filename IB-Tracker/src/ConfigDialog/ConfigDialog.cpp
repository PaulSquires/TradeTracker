
#include "pch.h"

#include "ConfigDialog.h"
#include "..\Themes\Themes.h"
#include "..\MainWindow\MainWindow.h"
#include "..\SuperLabel\SuperLabel.h"
#include "..\Database\database.h"


HWND HWND_CONFIGDIALOG = NULL;

extern HWND HWND_MAINWINDOW;

CConfigDialog ConfigDialog;



// ========================================================================================
// Process WM_CLOSE message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnClose(HWND hwnd)
{
    EnableWindow(HWND_MAINWINDOW, TRUE);
    DestroyWindow(hwnd);
}


// ========================================================================================
// Process WM_DESTROY message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}


// ========================================================================================
// Process WM_CREATE message for window/dialog: ConfigDialog
// ========================================================================================
BOOL ConfigDialog_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{

    HWND_CONFIGDIALOG = hwnd;

    int nLeft = 40;
    int nTop = 30;


    HWND hCtl = ConfigDialog.AddControl(Controls::Label, hwnd, -1, L"Themes", nLeft, nTop, 200, 18);

    nTop += 20;
    hCtl = ConfigDialog.AddControl(Controls::Line, hwnd, -1, L"", nLeft, nTop, 200, 1);

    nTop += 5;
    hCtl = ConfigDialog.AddControl(Controls::OptionButton, hwnd, IDC_CONFIGDIALOG_DARKTHEME,
            L"Dark Theme", nLeft, nTop, 200, 18, WS_GROUP);

    nTop += 20;
    hCtl = ConfigDialog.AddControl(Controls::OptionButton, hwnd, IDC_CONFIGDIALOG_DARKPLUSTHEME,
            L"Dark Plus Theme", nLeft, nTop, 200, 18);

    nTop += 20;
    hCtl = ConfigDialog.AddControl(Controls::OptionButton, hwnd, IDC_CONFIGDIALOG_BLUETHEME,
            L"Blue Theme", nLeft, nTop, 200, 18);

    nTop += 20;
    hCtl = ConfigDialog.AddControl(Controls::OptionButton, hwnd, IDC_CONFIGDIALOG_LIGHTTHEME,
            L"Light Theme", nLeft, nTop, 200, 18);

    CheckRadioButton(hwnd, IDC_CONFIGDIALOG_DARKTHEME, IDC_CONFIGDIALOG_LIGHTTHEME, GetThemeControlId());



    // --------------------------------
    nTop += 50;
    hCtl = ConfigDialog.AddControl(Controls::Label, hwnd, -1, L"Trader Name", nLeft, nTop, 200, 18);

    nTop += 20;
    hCtl = ConfigDialog.AddControl(Controls::TextBox, hwnd, IDC_CONFIGDIALOG_TRADERNAME, 
            L"", nLeft, nTop, 200, 20);
    std::wstring wszText = GetTraderName();
    AfxSetWindowText(hCtl, wszText.c_str());



    // --------------------------------
    nTop += 50;
    hCtl = ConfigDialog.AddControl(Controls::CheckBox, hwnd, IDC_CONFIGDIALOG_STARTUPCONNECT,
            L"Connect to Trader Workstation (TWS) when the application starts.", 
            nLeft, nTop, 500, 18);
    Button_SetCheck(hCtl, GetStartupConnect() ? BST_CHECKED : BST_UNCHECKED);



    // --------------------------------
    nLeft = 400;
    nTop += 60;
    hCtl = ConfigDialog.AddControl(Controls::Button, hwnd, IDC_CONFIGDIALOG_OK, 
            L"OK", nLeft, nTop, 74, 28);

    nLeft += 80;
    hCtl = ConfigDialog.AddControl(Controls::Button, hwnd, IDC_CONFIGDIALOG_CANCEL, 
            L"Cancel", nLeft, nTop, 74, 28);


    return TRUE;
}


// ========================================================================================
// Process WM_CTLCOLORBTN message for window/dialog: ConfigDialog
// ========================================================================================
HBRUSH ConfigDialog_OnCtlColorBtn(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_CTLCOLOREDIT message for window/dialog: ConfigDialog
// ========================================================================================
HBRUSH ConfigDialog_OnCtlColorEdit(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_CTLCOLORSTATIC message for window/dialog: ConfigDialog
// ========================================================================================
HBRUSH ConfigDialog_OnCtlColorStatic(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return GetSysColorBrush(COLOR_WINDOW);
}


// ========================================================================================
// Process WM_COMMAND message for window/dialog: ConfigDialog
// ========================================================================================
void ConfigDialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case (IDC_CONFIGDIALOG_OK):
        if (codeNotify == BN_CLICKED) {
            
            // Save the config data to the database
 
            bool applyTheme = false;

            if (IsDlgButtonChecked(hwnd, IDC_CONFIGDIALOG_DARKTHEME) == BST_CHECKED) {
                SetTheme(Themes::Dark);
                applyTheme = true;
            }
            else if (IsDlgButtonChecked(hwnd, IDC_CONFIGDIALOG_DARKPLUSTHEME) == BST_CHECKED) {
                SetTheme(Themes::DarkPlus);
                applyTheme = true;
            }
            else if (IsDlgButtonChecked(hwnd, IDC_CONFIGDIALOG_BLUETHEME) == BST_CHECKED) {
                SetTheme(Themes::Blue);
                applyTheme = true;
            }
            else if (IsDlgButtonChecked(hwnd, IDC_CONFIGDIALOG_LIGHTTHEME) == BST_CHECKED) {
                SetTheme(Themes::Light);
                applyTheme = true;
            }

            if (applyTheme) ApplyActiveTheme();


            std::wstring wszText = AfxGetWindowText(GetDlgItem(hwnd, IDC_CONFIGDIALOG_TRADERNAME));
            SetTraderName(wszText);


            bool startupConnect = 
                Button_GetCheck(GetDlgItem(hwnd, IDC_CONFIGDIALOG_STARTUPCONNECT)) == BST_CHECKED ? true : false;
            SetStartupConnect(startupConnect);

            SaveDatabase();
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    case (IDC_CONFIGDIALOG_CANCEL):
        if (codeNotify == BN_CLICKED) {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    }
}


// ========================================================================================
// Windows callback function.
// ========================================================================================
LRESULT CConfigDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(m_hwnd, WM_CREATE, ConfigDialog_OnCreate);
        HANDLE_MSG(m_hwnd, WM_COMMAND, ConfigDialog_OnCommand);
        HANDLE_MSG(m_hwnd, WM_DESTROY, ConfigDialog_OnDestroy);
        HANDLE_MSG(m_hwnd, WM_CLOSE, ConfigDialog_OnClose);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORBTN, ConfigDialog_OnCtlColorBtn);
        HANDLE_MSG(m_hwnd, WM_CTLCOLOREDIT, ConfigDialog_OnCtlColorEdit);
        HANDLE_MSG(m_hwnd, WM_CTLCOLORSTATIC, ConfigDialog_OnCtlColorStatic);

    default: return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}


void ConfigDialog_Show()
{
    HWND hwnd = ConfigDialog.Create(HWND_MAINWINDOW, L"Configuration", 0, 0, 600, 390,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        WS_EX_CONTROLPARENT | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR);

    HANDLE hIconSmall = LoadImage(ConfigDialog.hInst(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_SHARED);
    SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);
    
    AfxCenterWindow(hwnd, HWND_MAINWINDOW);

    EnableWindow(HWND_MAINWINDOW, FALSE);

    ShowWindow(hwnd, SW_SHOWNORMAL);

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