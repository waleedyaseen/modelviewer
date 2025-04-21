#pragma once

#include "imgui.h"
#include <glm/mat4x4.hpp>
#include <string>

namespace imp {
class ModelViewer;
enum class AlertType : uint8_t {
    Info,
    Warning,
    Error
};

enum class ThemeType : uint8_t {
    Dark,
    Light,
};

struct UITheme {
    ThemeType type = ThemeType::Dark;
    ImGuiStyle style;
};

class UI {
public:
    static void Initialize();

    static void ApplyTheme(ThemeType themeType);

    static void BeginFrame();
    static void EndFrame();

    static void BeginDockspace(char const* id);
    static void EndDockspace();

    static bool BeginPanel(char const* name, bool* open = nullptr, ImGuiWindowFlags flags = 0);
    static void EndPanel();

    static bool Button(char const* label, ImVec2 const& size = ImVec2(0, 0));
    static bool OverlayButton(char const* label, bool active, char const* tooltip, ImVec2 const& size = ImVec2(0, 0));
    static bool Checkbox(char const* label, bool* value, char const* tooltip = nullptr);
    static bool RadioButton(char const* label, bool active);
    static bool SliderFloat(char const* label, float* value, float min, float max, char const* format = "%.3f");
    static bool SliderInt(char const* label, int* value, int min, int max, char const* format = "%d");
    static bool Combo(char const* label, int* current_item, char const* const items[], int items_count);

    static void ColorButton(char const* desc_id, ImVec4 const& color, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0));
    static bool ColorEdit3(char const* label, float col[3], ImGuiColorEditFlags flags = 0);
    static bool ColorEdit4(char const* label, float col[4], ImGuiColorEditFlags flags = 0);

    static void Gizmo(ImVec2 const& position, float size, glm::mat4 const& viewMatrix);

    static void ShowAlert(std::string const& title, std::string const& message, AlertType type = AlertType::Info);
    static void ShowErrorAlert(std::string const& title, std::string const& message);

private:
    static void ApplyDarkTheme();
    static void ApplyLightTheme();
    static void RenderAlertModal();

    static void PushLabelStyle();
    static void PopLabelStyle();

    static float Scale(float value)
    {
        return value * s_scale;
    }

    static ImVec2 Scale(ImVec2 value)
    {
        return ImVec2(value.x * s_scale, value.y * s_scale);
    }

    static float s_scale;
    static UITheme s_currentTheme;
    static bool s_initialized;
    static bool s_showingAlert;
    static std::string s_alertTitle;
    static std::string s_alertMessage;
    static AlertType s_alertType;
};
}
