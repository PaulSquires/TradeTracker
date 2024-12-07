#include "app_base_linux.h"

#include "appstate.h"
#include "main_window.h"
#include "trade_history.h"
#include "journal_notes.h"
#include "tws-client.h"
#include <GLFW/glfw3.h>

//#include <iostream>


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

        state.config.CreateAppFonts(state);

        int width = state.dpi(state.config.GetStartupWidth(state));
        int height = state.dpi(state.config.GetStartupHeight(state));
        glfwSetWindowSize(window, width, height);

        state.right_panel_width = state.config.GetStartupRightPanelWidth(state);
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

        // Save main window size
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        state.config.SetStartupWidth(state, width);
        state.config.SetStartupHeight(state, height);
        state.config.SetStartupRightPanelWidth(state, state.right_panel_width);
        state.config.SaveConfig(state);

        glfwHideWindow(window);
        
        // If we are connected to TWS then initiate a disconnect that will also
        // stop the running background threads.
        if (tws_IsConnected(state)) tws_Disconnect(state);
    }

private:
};


