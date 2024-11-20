#include "app_base_win32.h"

#include "appstate.h"
#include "main_window.h"
#include "trade_history.h"
#include "journal_notes.h"
#include "tws-client.h"
#include <winuser.h>
#include <dwmapi.h>


class App : public AppBase<App> {
public:
    App(){};
    virtual ~App() = default;

    // Anything that needs to be called once OUTSIDE of the main application loop
    void StartUp() {
        state.config.LoadConfig(state);
        state.db.LoadDatabase(state);
        state.dpi_scale = dpi_scale;
        state.display_width = display_width;
        state.display_height = display_height;
        state.gui_font = gui_font;
        state.gui_font_mono = gui_font_mono;

        // Attempt to apply the standard Windows dark theme to the non-client areas of the main form.
        if (state.config.color_theme == ColorThemeType::Dark) {
            BOOL value = TRUE;
            ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
        }

        HICON hNewIcon = LoadIcon(GetModuleHandle(NULL), L"mainicon");
        if (hNewIcon) {
            // Set the large icon (32x32, title bar)
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hNewIcon);

            // Set the small icon (16x16, taskbar)
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hNewIcon);
        }

        int width = state.dpi(state.config.GetStartupWidth(state));
        int height = state.dpi(state.config.GetStartupHeight(state));
        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

        state.right_panel_width = state.config.GetStartupRightPanelWidth(state);

        // Show the window
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);
    }

    // Anything that needs to be called cyclically INSIDE of the main application loop
    void Update() {
        if (state.config.color_theme == ColorThemeType::Dark) {
            clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        } else if (state.config.color_theme == ColorThemeType::Light) {
            clear_color = ImVec4(255.0f, 255.0f, 255.0f, 1.00f);
        }

        static bool show_mainwindow = true;
        ShowMainWindow(&show_mainwindow, state);
    }

    // Anything that needs to be called once OUTSIDE of the main application loop
    void ShutDown() {
        // Save any modified Trade History Notes, JournalNotes. (if applicable)
        SaveTradeHistoryNotes(state);
        SaveJournalNotes(state);
        state.config.SaveConfig(state);

        ShowWindow(hwnd, false);
        
        // If we are connected to TWS then initiate a disconnect that will also
        // stop the running background threads.
        if (tws_IsConnected(state)) tws_Disconnect(state);
    }

private:
};


