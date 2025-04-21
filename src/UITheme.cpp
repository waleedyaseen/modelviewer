#include "UI.h"

namespace imp {
UITheme UI::s_currentTheme;

void UI::ApplyTheme(ThemeType themeType)
{
    s_currentTheme.type = themeType;
    ImGuiStyle& style = ImGui::GetStyle();
    s_currentTheme.style = style;
    switch (themeType) {
    case ThemeType::Dark:
        ApplyDarkTheme();
        break;
    case ThemeType::Light:
        ApplyLightTheme();
        break;
    }
}

void UI::ApplyDarkTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.09f, 0.90f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.90f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.46f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.56f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.22f, 0.58f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.58f, 0.98f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.22f, 0.58f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.58f, 0.98f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 0.58f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.58f, 0.98f, 0.30f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.58f, 0.98f, 0.40f);
    colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.22f, 0.58f, 0.98f, 0.40f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.22f, 0.58f, 0.98f, 0.60f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.22f, 0.58f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.22f, 0.58f, 0.98f, 0.50f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.22f, 0.58f, 0.98f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.58f, 0.98f, 0.30f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.58f, 0.98f, 0.40f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.15f, 0.90f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.22f, 0.58f, 0.98f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.18f, 0.18f, 0.22f, 0.50f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.22f, 0.58f, 0.98f, 0.25f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.22f, 0.58f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.CellPadding = ImVec2(6, 2);
    style.ItemSpacing = ImVec2(6, 4);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 22;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 12;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;

    style.WindowRounding = 6;
    style.ChildRounding = 4;
    style.FrameRounding = 4;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 12;
    style.GrabRounding = 4;
    style.TabRounding = 4;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.ScaleAllSizes(s_scale);
}

void UI::ApplyLightTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.80f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.97f, 0.97f, 0.97f, 0.90f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.85f, 0.85f, 0.85f, 0.90f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.45f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.45f, 0.85f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.45f, 0.85f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.45f, 0.85f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.45f, 0.85f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.45f, 0.85f, 0.30f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.45f, 0.85f, 0.40f);
    colors[ImGuiCol_Separator] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.45f, 0.85f, 0.40f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.45f, 0.85f, 0.60f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.45f, 0.85f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.45f, 0.85f, 0.50f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.45f, 0.85f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.45f, 0.85f, 0.30f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.45f, 0.85f, 0.40f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.45f, 0.85f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.33f, 0.25f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.80f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.90f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.75f, 0.75f, 0.75f, 0.50f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.45f, 0.85f, 0.25f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.90f, 0.80f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.45f, 0.85f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.80f, 0.80f, 0.80f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.CellPadding = ImVec2(6, 2);
    style.ItemSpacing = ImVec2(6, 4);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 22;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 12;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;

    style.WindowRounding = 6;
    style.ChildRounding = 4;
    style.FrameRounding = 4;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 12;
    style.GrabRounding = 4;
    style.TabRounding = 4;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.ScaleAllSizes(s_scale);
}
}
