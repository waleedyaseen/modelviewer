#include "UI.h"
#include "platform/imgui_impl_glfw.h"
#include "platform/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace imp {
float UI::s_scale = 1.0f;
bool UI::s_initialized = false;
bool UI::s_showingAlert = false;
std::string UI::s_alertTitle;
std::string UI::s_alertMessage;
AlertType UI::s_alertType = AlertType::Info;

void UI::Initialize()
{
    if (s_initialized)
        return;

    s_scale = 1.0f;

    ApplyTheme(ThemeType::Dark);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    float fontSize = Scale(15.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", fontSize);
    s_initialized = true;
}

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame()
{
    if (s_showingAlert) {
        RenderAlertModal();
    }
    ImGui::Render();
    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void UI::BeginDockspace(char const* id)
{
    ImGuiViewport const* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;
    ImGui::Begin(id, nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0) {
        ImGui::DockSpace(ImGui::GetID(id), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
}

void UI::EndDockspace()
{
    ImGui::End();
}

bool UI::BeginPanel(char const* name, bool* open, ImGuiWindowFlags flags)
{
    return ImGui::Begin(name, open, flags);
}

void UI::EndPanel()
{
    ImGui::End();
}

void UI::PushLabelStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8 * s_scale, 6 * s_scale));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
}

void UI::PopLabelStyle()
{
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

bool UI::Button(char const* label, ImVec2 const& size)
{
    ImVec2 scaledSize = Scale(size);
    return ImGui::Button(label, scaledSize);
}

bool UI::OverlayButton(char const* label, bool active, char const* tooltip, ImVec2 const& size)
{
    ImVec2 scaledSize = Scale(size);

    ImVec4 activeButtonColor = ImVec4(0.4f, 0.4f, 0.7f, 0.9f);
    ImVec4 activeHoverColor = ImVec4(0.5f, 0.5f, 0.8f, 1.0f);
    ImVec4 inactiveButtonColor = ImVec4(0.15f, 0.15f, 0.15f, 0.7f);
    ImVec4 inactiveHoverColor = ImVec4(0.25f, 0.25f, 0.25f, 0.9f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, activeButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeHoverColor);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, inactiveButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, inactiveHoverColor);
    }

    bool clicked = ImGui::Button(label, scaledSize);

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    return clicked;
}

bool UI::RadioButton(char const* label, bool active)
{
    float labelWidth = Scale(150.0f);
    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    return ImGui::RadioButton(id.c_str(), active);
}

bool UI::Checkbox(char const* label, bool* value, char const* tooltip)
{
    float labelWidth = Scale(150.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip(tooltip);
    }
    PopLabelStyle();

    std::string id = "##";
    id += label;

    bool result = ImGui::Checkbox(id.c_str(), value);
    return result;
}

bool UI::SliderFloat(char const* label, float* value, float min, float max, char const* format)
{
    float labelWidth = Scale(150.0f);
    float itemWidth = Scale(180.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    ImGui::SetNextItemWidth(itemWidth);
    bool result = ImGui::SliderFloat(id.c_str(), value, min, max, format);

    return result;
}

bool UI::SliderInt(char const* label, int* value, int min, int max, char const* format)
{
    float labelWidth = Scale(150.0f);
    float itemWidth = Scale(180.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    ImGui::SetNextItemWidth(itemWidth);
    bool result = ImGui::SliderInt(id.c_str(), value, min, max, format);

    return result;
}

bool UI::Combo(char const* label, int* current_item, char const* const items[], int items_count)
{
    float labelWidth = Scale(150.0f);
    float itemWidth = Scale(180.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    ImGui::SetNextItemWidth(itemWidth);
    bool result = ImGui::Combo(id.c_str(), current_item, items, items_count);

    return result;
}

void UI::ColorButton(char const* desc_id, ImVec4 const& color, ImGuiColorEditFlags flags, ImVec2 size)
{
    ImGui::ColorButton(desc_id, color, flags, size);
}

void UI::ShowAlert(std::string const& title, std::string const& message, AlertType type)
{
    s_alertTitle = title;
    s_alertMessage = message;
    s_alertType = type;
    s_showingAlert = true;
}

void UI::ShowErrorAlert(std::string const& title, std::string const& message)
{
    ShowAlert(title, message, AlertType::Error);
}

void UI::RenderAlertModal()
{
    if (!s_showingAlert) {
        return;
    }
    char const* POPUP_ID = "##AlertModalPopup";
    ImGui::OpenPopup(POPUP_ID);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    float modalWidth = Scale(400.0f);
    float modalHeight = Scale(150.0f);

    ImGui::SetNextWindowPos(ImVec2(center.x - (modalWidth / 2), center.y - (modalHeight / 2)), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(modalWidth, 0), ImGuiCond_Always);

    bool open = true;
    if (ImGui::BeginPopupModal(POPUP_ID, &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        auto titleSize = ImGui::CalcTextSize(s_alertTitle.c_str());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);
        ImGui::TextUnformatted(s_alertTitle.c_str());
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 12));

        ImVec4 iconColor;
        switch (s_alertType) {
        case AlertType::Info:
            iconColor = ImVec4(0.2f, 0.7f, 1.0f, 1.0f);
            break;
        case AlertType::Warning:
            iconColor = ImVec4(1.0f, 0.7f, 0.2f, 1.0f);
            break;
        case AlertType::Error:
            iconColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            break;
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        float iconSize = Scale(32.0f);
        float padding = Scale(10.0f);
        ImGui::Indent(iconSize + padding);

        ImVec2 iconPos = ImGui::GetCursorScreenPos();
        iconPos.x -= (iconSize + padding);
        iconPos.y += 4.0f;

        ImU32 iconColorU32 = ImGui::ColorConvertFloat4ToU32(iconColor);
        ImU32 iconBgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(iconColor.x, iconColor.y, iconColor.z, 0.2f));

        float iconCenterX = iconPos.x + (iconSize * 0.5f);
        float iconCenterY = iconPos.y + (iconSize * 0.5f);

        drawList->AddCircleFilled(ImVec2(iconCenterX, iconCenterY), iconSize * 0.5f, iconBgColor, 16);

        switch (s_alertType) {
        case AlertType::Info: {
            drawList->AddCircleFilled(ImVec2(iconCenterX, iconCenterY), iconSize * 0.4f, iconColorU32, 16);
            drawList->AddRectFilled(
                ImVec2(iconCenterX - (iconSize * 0.05f), iconCenterY - (iconSize * 0.2f)),
                ImVec2(iconCenterX + (iconSize * 0.05f), iconCenterY + (iconSize * 0.2f)),
                0xffffffff);
            drawList->AddCircleFilled(
                ImVec2(iconCenterX, iconCenterY - (iconSize * 0.25f)),
                iconSize * 0.05f,
                0xffffffff);
            break;
        }

        case AlertType::Warning: {
            ImVec2 triangle[3] = {
                ImVec2(iconCenterX, iconCenterY - (iconSize * 0.35f)),
                ImVec2(iconCenterX - (iconSize * 0.35f), iconCenterY + (iconSize * 0.25f)),
                ImVec2(iconCenterX + (iconSize * 0.35f), iconCenterY + (iconSize * 0.25f))
            };
            drawList->AddTriangleFilled(triangle[0], triangle[1], triangle[2], iconColorU32);

            drawList->AddRectFilled(
                ImVec2(iconCenterX - (iconSize * 0.05f), iconCenterY - (iconSize * 0.2f)),
                ImVec2(iconCenterX + (iconSize * 0.05f), iconCenterY + (iconSize * 0.05f)),
                0xffffffff);
            drawList->AddCircleFilled(
                ImVec2(iconCenterX, iconCenterY + (iconSize * 0.15f)),
                iconSize * 0.05f,
                0xffffffff);
            break;
        }

        case AlertType::Error: {
            drawList->AddCircleFilled(ImVec2(iconCenterX, iconCenterY), iconSize * 0.4f, iconColorU32, 16);

            float lineThickness = iconSize * 0.06f;
            float offset = iconSize * 0.2f;

            drawList->AddLine(
                ImVec2(iconCenterX - offset, iconCenterY - offset),
                ImVec2(iconCenterX + offset, iconCenterY + offset),
                0xffffffff,
                lineThickness);

            drawList->AddLine(
                ImVec2(iconCenterX + offset, iconCenterY - offset),
                ImVec2(iconCenterX - offset, iconCenterY + offset),
                0xffffffff,
                lineThickness);
            break;
        }
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + modalWidth - 50.0f);
        ImGui::TextWrapped("%s", s_alertMessage.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Unindent(iconSize + padding);
        ImGui::Spacing();

        float buttonWidth = Scale(120.0f);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);

        switch (s_alertType) {
        case AlertType::Info: {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
            break;
        }
        case AlertType::Warning: {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.6f, 0.2f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.7f, 0.3f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
            break;
        }
        case AlertType::Error: {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.3f, 0.3f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            break;
        }
        }

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
            s_showingAlert = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        if (!s_showingAlert) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (!open) {
        s_showingAlert = false;
    }
}

void UI::Gizmo(ImVec2 const& position, float size, glm::mat4 const& viewMatrix)
{
    glm::vec4 xAxis(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 yAxis(0.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 zAxis(0.0f, 0.0f, 1.0f, 1.0f);

    glm::mat4 viewRotation = viewMatrix;
    viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    xAxis = viewRotation * xAxis;
    yAxis = viewRotation * yAxis;
    zAxis = viewRotation * zAxis;

    xAxis = glm::normalize(xAxis);
    yAxis = glm::normalize(yAxis);
    zAxis = glm::normalize(zAxis);

    ImVec2 gizmoCenter(position.x + (size * 0.5f), position.y + (size * 0.5f));

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float axisLength = size * 0.4f;

    drawList->AddLine(
        gizmoCenter,
        ImVec2(gizmoCenter.x + (xAxis.x * axisLength), gizmoCenter.y - (xAxis.y * axisLength)),
        IM_COL32(255, 80, 80, 255),
        2.0f);

    drawList->AddLine(
        gizmoCenter,
        ImVec2(gizmoCenter.x + (yAxis.x * axisLength), gizmoCenter.y - (yAxis.y * axisLength)),
        IM_COL32(80, 255, 80, 255),
        2.0f);

    drawList->AddLine(
        gizmoCenter,
        ImVec2(gizmoCenter.x + (zAxis.x * axisLength), gizmoCenter.y - (zAxis.y * axisLength)),
        IM_COL32(80, 80, 255, 255),
        2.0f);

    float textOffset = axisLength + 5.0f;
    drawList->AddText(
        ImVec2(gizmoCenter.x + (xAxis.x * textOffset), gizmoCenter.y - (xAxis.y * textOffset)),
        IM_COL32(255, 80, 80, 255),
        "X");

    drawList->AddText(
        ImVec2(gizmoCenter.x + (yAxis.x * textOffset), gizmoCenter.y - (yAxis.y * textOffset)),
        IM_COL32(80, 255, 80, 255),
        "Y");

    drawList->AddText(
        ImVec2(gizmoCenter.x + (zAxis.x * textOffset), gizmoCenter.y - (zAxis.y * textOffset)),
        IM_COL32(80, 80, 255, 255),
        "Z");
}

bool UI::ColorEdit3(char const* label, float col[3], ImGuiColorEditFlags flags)
{
    float labelWidth = Scale(150.0f);
    float itemWidth = Scale(180.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    ImGui::SetNextItemWidth(itemWidth);
    bool result = ImGui::ColorEdit3(id.c_str(), col, flags);

    return result;
}

bool UI::ColorEdit4(char const* label, float col[4], ImGuiColorEditFlags flags)
{
    float labelWidth = Scale(150.0f);
    float itemWidth = Scale(180.0f);

    PushLabelStyle();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine(labelWidth);
    PopLabelStyle();

    std::string id = "##";
    id += label;

    ImGui::SetNextItemWidth(itemWidth);
    bool result = ImGui::ColorEdit4(id.c_str(), col, flags);

    return result;
}
}
