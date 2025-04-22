/*

MIT License

Copyright(c) 2023-2025 Paul Squires

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "imgui.h"
#include "imgui_stdlib.h"

#include "icons_material_design.h"
#include "appstate.h"
#include <string>


ImU32 clrBackDarkBlack(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(230,230,230,255) : IM_COL32(0,0,0,255));
}

ImU32 clrBackDarkGray(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(255,255,255,255) : IM_COL32(22,26,27,255));
}

ImU32 clrBackMediumGray(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(250,250,250,255) : IM_COL32(51,51,51,255));
}

ImU32 clrBackLightGray(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(251,251,249,255) : IM_COL32(68,68,68,255));
}

ImU32 clrTextDarkWhite(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(128,128,128,255) : IM_COL32(128,128,128,255));
}

ImU32 clrTextMediumWhite(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(98,90,75,255) : IM_COL32(157,165,180,255));
}

ImU32 clrTextLightWhite(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(38,38,38,255) : IM_COL32(212,212,212,255));
}

ImU32 clrTextBrightWhite(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,255));
}

ImU32 clrGreen(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(72,151,13,255) : IM_COL32(72,151,13,255));
}

ImU32 clrBlue(AppState& state) {  // CornflowerBlue
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(100,149,237,255) : IM_COL32(100,149,237,255));
}

ImU32 clrOrange(AppState& state) {  // burnt orange
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(193,98,24,255) : IM_COL32(193,98,24,255));
}

ImU32 clrRed(AppState& state) {  // Firebrick
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(178,34,34,255) : IM_COL32(178,34,34,255));
}

ImU32 clrMagenta(AppState& state) {  // magenta
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(255,0,255,255) : IM_COL32(255,0,255,255));
}

ImU32 clrYellow(AppState& state) {  // yellow
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(255,255,0,255) : IM_COL32(255,255,0,255));
}

ImU32 clrSelection(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(235,235,235,255) : IM_COL32(44,49,58,255));
}

ImU32 clrTextSelection(AppState& state) {  
    return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(51,153,255,255) : IM_COL32(51,153,255,255));
}

ImU32 clrCheckBox(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(235,235,235,255) : IM_COL32(44,49,58,255));
}

ImU32 clrCheckBoxHover(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(255,255,255,255) : IM_COL32(64,69,78,255));
}

ImU32 clrPopupBg(AppState& state) {
	return (state.config.color_theme == ColorThemeType::Light ? IM_COL32(235,235,235,175) : IM_COL32(44,49,58,175));
}


// Function to convert ImVec4 color to ImU32
ImU32 ColorConvertImVec4ToU32(const ImVec4& color) {
    // Convert each float component to an 8-bit integer (0-255) and pack into a uint32_t
    uint8_t r = static_cast<uint8_t>(color.x * 255.0f);
    uint8_t g = static_cast<uint8_t>(color.y * 255.0f);
    uint8_t b = static_cast<uint8_t>(color.z * 255.0f);
    uint8_t a = static_cast<uint8_t>(color.w * 255.0f);

    // Pack the RGBA components into a uint32_t
    ImU32 result = (a << 24) | (b << 16) | (g << 8) | r;
    return result; 
}


// Function to convert ImU32 color to ImVec4
ImVec4 ColorConvertU32ToFloat4(ImU32 color) {
    float s = 1.0f / 255.0f;
    return ImVec4(
        s * (float)((color >> IM_COL32_R_SHIFT) & 0xFF),
        s * (float)((color >> IM_COL32_G_SHIFT) & 0xFF),
        s * (float)((color >> IM_COL32_B_SHIFT) & 0xFF),
        s * (float)((color >> IM_COL32_A_SHIFT) & 0xFF)
    );
}


// Function to create a colored button
bool ColoredButton(AppState& state, const char* label, const float x_position, 
            const ImVec2& size, const ImU32& text_color, const ImU32& back_color) {
    ImVec4 ColorVec4 = ColorConvertU32ToFloat4(back_color);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_Button, ColorVec4);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ColorVec4.x * 2.0f, ColorVec4.y * 2.0f, ColorVec4.z * 2.0f, ColorVec4.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ColorVec4.x * 0.9f, ColorVec4.y * 0.9f, ColorVec4.z * 0.9f, ColorVec4.w));
    ImGui::SameLine(state.dpi(x_position));
    ImGui::PushID(ImGui::GetCursorScreenPos().x + ImGui::GetCursorScreenPos().y);
    bool pressed = ImGui::Button(label, size);
    ImGui::PopID();
    ImGui::PopStyleColor(4); // Restore original colors
    return pressed;
}


// Function to create a center label (use a button because it centers text by default)
bool TextLabelCentered(AppState& state, const char* label, const float x_position, 
            const ImVec2& size, const ImU32& text_color, const ImU32& back_color) {
    // Use a Button because that automatically centers the text
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_Button, back_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, back_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, back_color);
    ImGui::SameLine(state.dpi(x_position));
    ImGui::PushID(ImGui::GetCursorScreenPos().x + ImGui::GetCursorScreenPos().y);
    bool pressed = ImGui::Button(label, size);
    ImGui::PopID();
    ImGui::PopStyleColor(4); // Restore original colors
    return pressed;
}


// Function to create a small colored button
bool ColoredSmallButton(AppState& state, const char* label, const float x_position, const ImU32& text_color, const ImU32& back_color) {
    ImVec4 ColorVec4 = ColorConvertU32ToFloat4(back_color);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_Button, ColorVec4);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ColorVec4.x * 1.1f, ColorVec4.y * 1.1f, ColorVec4.z * 1.1f, ColorVec4.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ColorVec4.x * 0.9f, ColorVec4.y * 0.9f, ColorVec4.z * 0.9f, ColorVec4.w));
    ImGui::SameLine(state.dpi(x_position));
    ImGui::PushID(ImGui::GetCursorScreenPos().x + ImGui::GetCursorScreenPos().y);
    bool pressed = ImGui::SmallButton(label);
    ImGui::PopID();
    ImGui::PopStyleColor(4); // Restore original colors
    return pressed;
}

void TextLabel(AppState& state, const char* label, const float x_position, const ImU32& text_color, const ImU32& back_color) {
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, back_color);  
    ImGui::SameLine(state.dpi(x_position));
    ImGui::PushID(ImGui::GetCursorScreenPos().x + ImGui::GetCursorScreenPos().y);
    ImGui::Text("%s", label);
    ImGui::PopID();
    ImGui::PopStyleColor(3); // Restore original colors
}

bool CheckBox(AppState& state, const char* label, bool* v, const float x_position, const ImU32& text_color, const ImU32& back_color) {
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, back_color);  
    ImGui::SameLine(state.dpi(x_position));
    ImGui::PushID(ImGui::GetCursorScreenPos().x + ImGui::GetCursorScreenPos().y);
    bool result = ImGui::Checkbox(label, v);
    ImGui::PopID();
    ImGui::PopStyleColor(3); // Restore original colors
    return result;
}


bool TextInput(AppState& state, const char* id, std::string* str, ImGuiInputTextFlags flags, const float x_position,
             const float width, const ImU32& text_color, const ImU32& back_color, ImGuiInputTextCallback callback) {
    flags = (flags | ImGuiInputTextFlags_AutoSelectAll);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, back_color);  
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, clrTextSelection(state));  
    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));
    // Creating a dynamic ID did not work when passing to InputText and then trying
    // to set its initial keyboard focus. I changed this function to accept an incoming
    // id rather than generate it.
    bool result = ImGui::InputText(id, str, flags, callback);
    ImGui::PopStyleColor(4); // Restore original colors
    return result;
}

bool DoubleInput(AppState& state, const char* id, double* v, const float x_position, const float width, 
            const ImU32& text_color, const ImU32& back_color) {
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, back_color);  
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, clrTextSelection(state));  
    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));
    // Creating a dynamic ID did not work when passing to InputText and then trying
    // to set its initial keyboard focus. I changed this function to accept an incoming
    // id rather than generate it.
    bool result = ImGui::InputDouble(id, v);
    ImGui::PopStyleColor(4); // Restore original colors
    return result;
}


bool IntegerInput(AppState& state, const char* id, int* v, const float x_position, const float width, 
            const ImU32& text_color, const ImU32& back_color) {
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, back_color);  
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, clrTextSelection(state));  
    ImGui::SameLine(state.dpi(x_position));
    ImGui::SetNextItemWidth(state.dpi(width));
    // Creating a dynamic ID did not work when passing to InputText and then trying
    // to set its initial keyboard focus. I changed this function to accept an incoming
    // id rather than generate it.
    bool result = ImGui::InputInt(id, v);
    ImGui::PopStyleColor(4); // Restore original colors
    return result;
}

void DialogTitleBar(AppState& state, const char* label, bool& show_dialog_popup, bool& is_popup_open) {
    ImGui::PushStyleColor(ImGuiCol_Text, clrTextMediumWhite(state));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, clrBackMediumGray(state));  
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, clrBackMediumGray(state));  

    ImGui::SameLine(state.dpi(14.0f));
    ImGui::Text("%s", label);

    // Custom close button in the title bar
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x); 
    if (ImGui::Button(ICON_MD_CLOSE)) {
        ImGui::CloseCurrentPopup();
        show_dialog_popup = false;
        is_popup_open = false;
    }
    ImGui::NewLine();

    ImGui::PopStyleColor(3); 
}

void Tooltip(AppState& state, const char* label, const ImU32& text_color, const ImU32& back_color) {
    if (ImGui::IsItemHovered()) {
        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, back_color);  
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(state.dpi(8.0f), state.dpi(6.0f)));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, state.dpi(6.0f));
        ImGui::BeginTooltip();
        ImGui::Text("%s", label);
        ImGui::EndTooltip();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
}
