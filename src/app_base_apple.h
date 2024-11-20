#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "implot.h"
#include <cstdlib>
#include <stdio.h>
#include <SDL.h>
#include <string>

#include "utilities.h"
#include "icons_material_design.h"

// #include <iostream>


template <typename Derived>
class AppBase {

public:
    float dpi_scale = 1;
    int display_width = 0;
    int display_height = 0;
    ImFont* gui_font = nullptr;
    ImFont* gui_font_mono = nullptr;

    AppBase() {
        // Setup SDL
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
            printf("Error: %s\n", SDL_GetError());
            std::exit(1);
        }

        // From 2.0.18: Enable native IME.
        #ifdef SDL_HINT_IME_SHOW_UI
            SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
        #endif

        // Get the primary monitor and video mode of the primary monitor
        SDL_DisplayMode displayMode;
        SDL_Rect displayBounds;

        // Get the bounds of the primary display (display index 0)
        if (SDL_GetDisplayBounds(0, &displayBounds) != 0) {
            printf("Could not get display bounds: %s\n", SDL_GetError());
            std::exit(1);
        }
        display_width = displayBounds.w;
        display_height = displayBounds.h;

        // Create window with SDL_Renderer graphics context
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        std::string appname = std::string("TradeTracker v") + state.version_current_display;
        window = SDL_CreateWindow(appname.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
        if (!window) {
            printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
            std::exit(1);
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printf("Error creating SDL_Renderer!");
            std::exit(1);
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImVec4* colors                         = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text]                  = ImVec4(252.f / 255.f, 224.f / 255.f, 176.f / 255.f, 1.f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.25f, 0.25f, 0.54f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.3f, 0.3f, 0.3f, 0.54f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_Header]                = ImVec4(0.17f, 0.19f, 0.23f, 1.00f);    // Selected row
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.17f, 0.19f, 0.23f, 1.00f);    // Selected row
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.17f, 0.19f, 0.23f, 0.00f);    // Hovered row color (zero alpha so inactive)

        ImGuiStyle& style       = ImGui::GetStyle();
        style.WindowPadding     = ImVec2(8.00f, 8.00f);
        style.FramePadding      = ImVec2(5.00f, 2.00f);
        style.CellPadding       = ImVec2(12.00f, 4.00f);
        style.ItemSpacing       = ImVec2(6.00f, 6.00f);
        style.ItemInnerSpacing  = ImVec2(6.00f, 6.00f);
        style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
        style.IndentSpacing     = 25;
        style.ScrollbarSize     = 15;
        style.GrabMinSize       = 10;
        style.WindowBorderSize  = 0; //1;
        style.ChildBorderSize   = 0; //1;
        style.PopupBorderSize   = 1;
        style.FrameBorderSize   = 1;
        style.TabBorderSize     = 1;
        style.WindowRounding    = 0; //7;
        style.ChildRounding     = 0; //4;
        style.FrameRounding     = 3;
        style.PopupRounding     = 4;
        style.ScrollbarRounding = 9;
        style.GrabRounding      = 3;
        style.LogSliderDeadzone = 4;
        style.TabRounding       = 4;

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        // Add custom fonts (16px looks best on Mac)
        float baseFontSize = 16.0f; 
        float iconFontSize = 16.0f;

        // Load the embedded font
        //io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_compressed_data, segoeui_compressed_size, baseFontSize * dpi_scale);

        // Merging two memory compressed fonts does not seem to work so load the fonts from disk instead.
        // ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(materialicons_compressed_data, materialicons_compressed_size, baseFontSize * dpi_scale);
        // if (font == NULL) {
        //     std::cerr << "Failed to load embedded font" << std::endl;
        // }

        std::string fontfile = AfxGetExePath() + "/gui-font.ttf";
        gui_font = io.Fonts->AddFontFromFileTTF(fontfile.c_str(), baseFontSize * dpi_scale);

        // Define and merge into the base font the Unicode ranges for Material Design Icons
        static const ImWchar icon_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphMinAdvanceX = iconFontSize; 
        config.GlyphOffset.y = 3.0f * dpi_scale; 
        fontfile = AfxGetExePath() + "/icon-font.ttf";
        io.Fonts->AddFontFromFileTTF(fontfile.c_str(), iconFontSize * dpi_scale, &config, icon_ranges);

        fontfile = AfxGetExePath() + "/gui-font-mono.ttf";
        gui_font_mono = io.Fonts->AddFontFromFileTTF(fontfile.c_str(), baseFontSize * dpi_scale);

        ImGui::GetStyle().ScaleAllSizes(dpi_scale);
    }


    virtual ~AppBase() {
        // Cleanup
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Run() {
        // Initialize the underlying app
        StartUp();

        // Main loop
        const int FPS = 60;
        const int frameDelay = 1000 / FPS;

        Uint32 frameStart;
        int frameTime;

        bool done = false;
        while (!done) {
            frameStart = SDL_GetTicks();

            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                    done = true;
            }
            
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
                SDL_Delay(10);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // Main loop of the underlying app
            Update();

            // Rendering
            ImGui::Render();
            ImGuiIO& io = ImGui::GetIO();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);

            frameTime = SDL_GetTicks() - frameStart;

            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }

        // Shutdown the underlying app
        ShutDown();
    }

    void Update() {
        static_cast<Derived*>(this)->Update();
    }

    void StartUp() {
        static_cast<Derived*>(this)->StartUp();
    }

    void ShutDown() {
        static_cast<Derived*>(this)->ShutDown();
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    AppState state{};

private:
};


