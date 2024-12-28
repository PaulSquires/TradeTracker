#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "implot.h"
#include <cstdlib>
#include <stdio.h>
#include <SDL.h>
#include <string>

#include "utilities.h"

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

        state.initialize_imgui_state();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);
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


