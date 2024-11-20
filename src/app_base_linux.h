#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "implot.h"
#include <cstdlib>
#include <stdio.h>

#include "utilities.h"
#include "icons_material_design.h"

#define STB_IMAGE_IMPLEMENTATION  // This needs to be defined in **one** source file
#include "stb_image.h"

// #include <iostream>


static void ErrorCallback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


template <typename Derived>
class AppBase {

public:
    float dpi_scale = 1;
    int display_width = 0;
    int display_height = 0;
    ImFont* gui_font = nullptr;
    ImFont* gui_font_mono = nullptr;

    AppBase() {
        glfwSetErrorCallback(ErrorCallback);

        if (!glfwInit())
            std::exit(1);

        // Get the primary monitor and video mode of the primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        display_width = mode->width;
        display_height = mode->height;

        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // Create window with graphics context
        std::string appname = std::string("TradeTracker v") + state.version_current_display;
        window = glfwCreateWindow(1280, 720, appname.c_str(), nullptr, nullptr);
        if (window == NULL)
            std::exit(1);

        std::string icon_name = AfxGetExePath() + std::string("/mainicon.png");
        GLFWimage images[1];
        images[0].pixels = stbi_load(icon_name.c_str(), &images[0].width, &images[0].height, 0, 4);
        glfwSetWindowIcon(window, 1, images);
        stbi_image_free(images[0].pixels);

        glfwSetWindowSize(window, 1920, 1080);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

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
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Scale UI and fonts for high DPI
        float xscale, yscale;
        glfwGetWindowContentScale(window, &xscale, &yscale);
        dpi_scale = (xscale + yscale) * 0.5f;

        ImGui::GetStyle().ScaleAllSizes(dpi_scale);

        // Add custom fonts (18px seems to look best on Linux)
        float baseFontSize = 18.0f; 
        float iconFontSize = 18.0f;

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
    }


    virtual ~AppBase() {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Run() {
        // Initialize the underlying app
        StartUp();

        while (!glfwWindowShouldClose(window)) {
            // Poll events like key presses, mouse movements etc.
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Main loop of the underlying app
            Update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
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

    GLFWwindow* window = nullptr;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    AppState state{};

private:
};


