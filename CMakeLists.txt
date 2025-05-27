# Windows uses only one build tree:
# (Note: Ensure that CMake and MS Studio Build Tools are installed)
# (need to run these commands in x64 Developer Command Prompt)
# cmake -G "Visual Studio 17 2022" -A x64 -Bbuild 
# cmake --build build -j4 --config Release
# cmake --build build -j4   # use if above command fails

# On Mac or Linux you'll need to maintain two build trees:
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug -GNinja
# cmake --build build -j4
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S. -Bbuild-rel -DCMAKE_BUILD_TYPE=Release -GNinja
# cmake --build build-rel -j4

# Need to install Homebrew, xcode, ninja, sdl2, sdl2_image, curl, and cmake on Mac
# Apple distributions are done via /resources/TradeTracker.app folder.
# Create the .dmg distribution image
# hdiutil create -volname "TradeTracker" -srcfolder "./TradeTracker.app" -ov -format UDZO "TradeTracker.dmg"

# UBUNTU needs the fuse library in order to run AppImage applications.
# sudo apt-get install libfuse-dev


# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME TradeTracker)
project(${PROJECT_NAME} LANGUAGES CXX C)

# Specify the C++ standard and Compiler
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/imgui)

if(WIN32)
    set(CMAKE_CXX_COMPILER cl)
    AUX_SOURCE_DIRECTORY(src/tws-api/windows TWSAPISOURCES) 
else()
    set(CMAKE_CXX_COMPILER clang++)
    AUX_SOURCE_DIRECTORY(src/tws-api/linux TWSAPISOURCES) 
endif()

# Source files
set(SOURCES
    src/main.cpp;
    src/main_window.cpp;
    src/active_trades.cpp;
    src/active_trades_actions.cpp;
    src/closed_trades.cpp;
    src/trade_history.cpp;
    src/transaction_panel.cpp;
    src/transaction_edit.cpp;
    src/journal_notes.cpp;
    src/filter_panel.cpp;
    src/tab_panel.cpp;
    src/settings_dialog.cpp;
    src/categories.cpp;
    src/colors.cpp;
    src/utilities.cpp;
    src/config.cpp;
    src/database.cpp;
    src/trade.cpp;
    src/list_panel_data.cpp;
    src/list_panel.cpp;
    src/reconcile.cpp;
    src/assignment.cpp;
    src/messagebox.cpp;
    src/questionbox.cpp;
    src/text_input_popup.cpp;
    src/tws-client.cpp;
    src/import_dialog.cpp;
    src/trade_dialog.cpp;
    src/trade_dialog_save.cpp;
    src/trade_dialog_initialize.cpp;
    src/yearend_dialog.cpp;
    src/imgui_stdlib.cpp;
    src/implot.cpp;
    src/implot_items.cpp;
    ${TWSAPISOURCES};
    ${IMGUI_DIR}/imgui.cpp;
    ${IMGUI_DIR}/imgui_demo.cpp;
    ${IMGUI_DIR}/imgui_draw.cpp;
    ${IMGUI_DIR}/imgui_tables.cpp;
    ${IMGUI_DIR}/imgui_widgets.cpp;
)

if (UNIX AND NOT APPLE)
    set(SOURCES 
        ${SOURCES}
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    #    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    #    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
elseif (APPLE)
    set(SOURCES 
        ${SOURCES}
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    )
elseif (WIN32)
    set(SOURCES 
        ${SOURCES}
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Resources/TradeTracker.rc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Resources/TradeTracker.exe.manifest
        )
endif()


# Include directories
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Platform-specific settings
if (UNIX AND NOT APPLE)
    message("Configuring for Linux")
    find_package(PkgConfig REQUIRED)

#    pkg_check_modules(GLFW REQUIRED glfw3)
#    set(LIBS ${GLFW_STATIC_LIBRARIES} GL curl libbid)
#    include_directories(${GLFW_INCLUDE_DIRS})
#    link_directories(${GLFW_LIBRARY_DIRS})

#    get_filename_component(LIB_B_PATH ./src/tws-api/IntelDecimal/lib/linux/libbid.a ABSOLUTE)
#    add_library(libbid STATIC IMPORTED)
#    set_target_properties(libbid PROPERTIES IMPORTED_LOCATION ${LIB_B_PATH})

    add_definitions(-DGL_SILENCE_DEPRECATION)
    find_package(SDL2 REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
#    set(LIBS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo" SDL2::SDL2-static SDL2::SDL2main curl libbid)
    set(LIBS SDL2 curl libbid)

    include_directories(/usr/local/include /opt/local/include /opt/homebrew/include /usr/local/opt/llvm/include)
    link_directories(/usr/local/lib /opt/local/lib /opt/homebrew/lib /usr/local/opt/llvm/lib )
    set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++")
#    get_filename_component(LIB_B_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/tws-api/IntelDecimal/lib/osx/libbid.a ABSOLUTE)
    get_filename_component(LIB_B_PATH ./src/tws-api/IntelDecimal/lib/linux/libbid.a ABSOLUTE)
    add_library(libbid STATIC IMPORTED)
    set_target_properties(libbid PROPERTIES IMPORTED_LOCATION ${LIB_B_PATH})

elseif (APPLE)
    # Using SDL on Apple because could not get GLFW/OpenGL3 to work.
    message("Configuring for Mac OS X")
    add_definitions(-DGL_SILENCE_DEPRECATION)
    find_package(SDL2 REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    set(LIBS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo" SDL2::SDL2-static SDL2::SDL2main curl libbid)

    include_directories(/usr/local/include /opt/local/include /opt/homebrew/include /usr/local/opt/llvm/include)
    link_directories(/usr/local/lib /opt/local/lib /opt/homebrew/lib /usr/local/opt/llvm/lib )
    set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++")
    get_filename_component(LIB_B_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/tws-api/IntelDecimal/lib/osx/libbid.a ABSOLUTE)
    add_library(libbid STATIC IMPORTED)
    set_target_properties(libbid PROPERTIES IMPORTED_LOCATION ${LIB_B_PATH})


elseif (WIN32)
    message("Configuring for Windows")

    # Windows compiler warnings
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
    add_definitions(-std:c++20)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    
    add_compile_options(/MT)    # static link operating system files

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    set(LIBS d3d11 d3dcompiler libbid)

    link_directories(${WINDOW_DXSDK_LIB})
    include_directories(${WINDOW_DXSDK_INCLUDE} ${WINDOW_API_INCLUDE} ./src/curl-win64/include ./src/curl-win64/include/curl)

    get_filename_component(LIB_B_PATH ./src/tws-api/IntelDecimal/lib/win/libbid.lib ABSOLUTE)
    add_library(libbid STATIC IMPORTED)
    set_target_properties(libbid PROPERTIES IMPORTED_LOCATION ${LIB_B_PATH})

endif()

message("Compiling")


# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

    
# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})



