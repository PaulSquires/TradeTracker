#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "implot.h"
#include <cstdlib>
#include <stdio.h>

#include "utilities.h"

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

        state.initialize_imgui_state();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Scale UI and fonts for high DPI
        float xscale, yscale;
        glfwGetWindowContentScale(window, &xscale, &yscale);
        dpi_scale = (xscale + yscale) * 0.5f;
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
            glfwSwapInterval(1);   // enables vsync

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


