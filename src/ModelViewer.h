#pragma once

#include "Renderer.h"

#include <filesystem>
#include <future>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>

#include "imgui.h"

namespace imp {
enum class ExportFormat {
    MQO,
    DAT
};
struct Node {
    std::string name;
    std::filesystem::path path;
    bool isExpanded { true };
    bool isScanned { false };
    std::vector<Node> children;
    bool isDirectory { false };
};

struct ApplicationSettings {
    bool wireframeMode { true };
    int tileGridSize { 1 };
    float fov { 45.0f };
    int windowWidth { 1280 };
    int windowHeight { 720 };
    int windowPosX { -1 };
    int windowPosY { -1 };
    std::vector<std::filesystem::path> files;
    bool faceTooltip { true };
    struct FaceTooltipOptions {
        bool showVertices { true };
        bool showColor { true };
        bool showLabel { true };
        bool showType { true };
        bool showPriority { true };
        bool showMaterial { true };
        bool showMapping { true };
    } faceTooltipOptions;
    bool vertexTooltip { true };
    struct VertexTooltipOptions {
        bool showPosition { true };
        bool showLabel { true };
    } vertexTooltipOptions;
    glm::vec4 hoveredHighlightColor { 1.0f, 0.8f, 0.2f, 1.0f };
    glm::vec4 selectedHighlightColor { 0.2f, 0.8f, 1.0f, 1.0f };
    glm::vec4 wireframeColor { 0.0f, 0.0f, 0.0f, 1.0f };
    int uiTheme { 0 };
    float uiScale { 1.0f };
    bool vertexMode { false };
};

class ModelViewer {
public:
    ModelViewer();
    ~ModelViewer();

    void Start();

private:
    void Logic(float deltaTime);
    void Draw(float deltaTime);
    void DrawPick();
    void ImGui_FrameStart();
    void ImGui_FrameEnd();

    void InitGLFW();
    void InitImGui() const;

    void CreateDockspace();
    void RenderMenuBar();
    void RenderFileExplorer();
    void RenderModelViewer();
    void RenderFaceTooltip();
    void RenderVertexTooltip();
    void RenderOptionsPanel();
    void RenderModelStatsPanel();
    void RenderFaceDetails(char const* label, int faceIndex, Face const& face, std::vector<Vertex> const& vertices);
    void RenderVertexDetails(char const* label, int vertexIndex, Vertex const& vertex);
    void HandleShortcuts();

    void ConfigureDocking();

    void OpenDirectory();
    void OpenFile();
    bool IsValidModelFile(std::filesystem::path const& path) const;
    void ScanDirectory(Node& node);
    void DisplayNodeContents(Node& node);
    void ClearFileExplorer();
    void RemoveNode(Node& node);
    void ShowError(std::string const& title, std::string const& message);

    void LoadModel(std::filesystem::path const& path);
    void ExportModel(ExportFormat format);

    void LoadSettings();
    void SaveSettings();

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    GLFWwindow* m_window;
    bool m_appRunning { true };
    float m_lastFrameTime { 0.0f };
    float m_deltaTime { 0.0f };

    bool m_mousePressed = false;
    bool m_mouseOverViewport { false };
    bool m_lastPressWasDrag { false };
    glm::vec2 m_prevMousePos = { 0.0f, 0.0f };

    int m_viewportX { 0 };
    int m_viewportY { 0 };

    ImGuiID m_centralID;
    char m_searchBuffer[256] { "" };
    std::string m_searchQuery;
    bool m_focusSearchNextFrame { false };
    std::string m_errorTitle;
    std::string m_errorMessage;
    std::vector<Node> m_rootNodes;

    Renderer m_renderer;
    std::filesystem::path m_currentLoadedModelPath;
    std::filesystem::path m_settingsPath;
    std::future<void> m_scanFuture;

    ApplicationSettings m_settings;
    bool m_settingsModified { false };
};
}
