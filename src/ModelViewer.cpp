#include <glad/glad.h>

#include "Dialogs.h"
#include "Model.h"
#include "ModelLoader.h"
#include "ModelViewer.h"

#include "Packet.h"
#include "RunetekColor.h"
#include "imgui_internal.h"
#include "platform/imgui_impl_glfw.h"
#include "platform/imgui_impl_opengl3.h"
#include <algorithm>
#include <cfloat>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>

#include "ModelExporter.h"
#include "UI.h"

namespace imp {
ModelViewer::ModelViewer()
    : m_settingsPath(std::filesystem::current_path() / "modelviewer_settings.json")
{
    LoadSettings();

    InitGLFW();
    InitImGui();

    m_renderer.SetFarPlane(3584.0f);
    m_renderer.SetNearPlane(0.1f);

    m_renderer.SetFOV(m_settings.fov);
    m_renderer.SetWireframeMode(m_settings.wireframeMode);
    m_renderer.SetTileGridSize(m_settings.tileGridSize);

    ModelRenderer& modelRenderer = m_renderer.GetModelRenderer();
    modelRenderer.SetHighlightColor(m_settings.hoveredHighlightColor);
    modelRenderer.SetSelectedColor(m_settings.selectedHighlightColor);
    modelRenderer.SetWireframeColor(m_settings.wireframeColor);
    modelRenderer.SetVertexMode(m_settings.vertexMode);
}

ModelViewer::~ModelViewer()
{
    SaveSettings();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void ModelViewer::Start()
{
    m_renderer.Initialize();

    float currentFrameTime;
    float deltaTime;

    while (glfwWindowShouldClose(m_window) == GLFW_FALSE && m_appRunning) {
        currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - m_lastFrameTime;
        m_deltaTime = deltaTime;
        m_lastFrameTime = currentFrameTime;

        glfwPollEvents();
        Logic(deltaTime);
        Draw(deltaTime);
        glfwSwapBuffers(m_window);
    }
}

void ModelViewer::Logic(float deltaTime)
{
    if (m_settingsModified) {
        SaveSettings();
    }
}

void ModelViewer::Draw(float deltaTime)
{
    m_renderer.Render(deltaTime);
    DrawPick();

    ImGui_FrameStart();
    RenderMenuBar();
    RenderFileExplorer();
    RenderModelViewer();
    RenderOptionsPanel();
    RenderModelStatsPanel();
    HandleShortcuts();
    ImGui_FrameEnd();
}

void ModelViewer::DrawPick()
{
    ModelRenderer& modelRenderer = m_renderer.GetModelRenderer();
    if (!m_mouseOverViewport) {
        modelRenderer.SetHoveredVertex(-1);
        modelRenderer.SetHoveredFace(-1);
        return;
    }
    int selection = m_renderer.Pick(m_viewportX, m_viewportY);
    if (m_settings.vertexMode) {
        modelRenderer.SetHoveredFace(-1);
        modelRenderer.SetHoveredVertex(selection);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !m_lastPressWasDrag) {
            modelRenderer.SetSelectedVertex(selection);
        }
    } else {
        modelRenderer.SetHoveredVertex(-1);
        modelRenderer.SetHoveredFace(selection);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !m_lastPressWasDrag) {
            modelRenderer.SetSelectedFace(selection);
        }
    }
}
void ModelViewer::ImGui_FrameStart()
{
    UI::BeginFrame();

    UI::BeginDockspace("ModelViewerDockspace");
}

void ModelViewer::ImGui_FrameEnd()
{
    UI::EndDockspace();

    UI::EndFrame();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModelViewer::InitGLFW()
{
    if (glfwInit() == 0) {
        ShowFatalDialog("Error", "Failed to initialize GLFW");
        std::exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_settings.windowWidth, m_settings.windowHeight, "Model Viewer (" GIT_COMMIT_HASH ")", nullptr, nullptr);
    if (!m_window) {
        ShowFatalDialog("Error", "Failed to create GLFW window");
        std::exit(1);
    }

    if (m_settings.windowPosX == -1 && m_settings.windowPosY == -1) {
        int monitorWidth, monitorHeight;
        glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), nullptr, nullptr, &monitorWidth, &monitorHeight);
        m_settings.windowPosX = (monitorWidth - m_settings.windowWidth) / 2;
        m_settings.windowPosY = (monitorHeight - m_settings.windowHeight) / 2;
    } else {
        glfwSetWindowPos(m_window, m_settings.windowPosX, m_settings.windowPosY);
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        ShowFatalDialog("Error", "Failed to initialize OpenGL context");
        std::exit(1);
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);

    glEnable(GL_DEPTH_TEST);
}

void ModelViewer::InitImGui() const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    UI::Initialize();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ModelViewer::CreateDockspace()
{
    ImGuiViewport const* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("ModelViewer", nullptr, windowFlags);

    ImGui::PopStyleVar(3); // WindowRounding, WindowBorderSize, WindowPadding

    if (ImGuiIO const& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        m_centralID = ImGui::GetID("Central");
        ImGui::DockSpace(m_centralID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        static bool dockingConfigured = true;
        if (dockingConfigured) {
            dockingConfigured = false;
            ConfigureDocking();
        }
    }

    ImGui::End();
}

void ModelViewer::ConfigureDocking()
{
    ImGui::DockBuilderRemoveNode(m_centralID);
    ImGui::DockBuilderAddNode(m_centralID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(m_centralID, ImGui::GetMainViewport()->Size);

    ImGuiID const fileExplorerID = ImGui::DockBuilderSplitNode(m_centralID, ImGuiDir_Left, 0.20f, nullptr, &m_centralID);
    ImGuiID const optionsID = ImGui::DockBuilderSplitNode(m_centralID, ImGuiDir_Right, 0.25f, nullptr, &m_centralID);

    ImGui::DockBuilderDockWindow("File Explorer", fileExplorerID);
    ImGui::DockBuilderDockWindow("Options", optionsID);
    ImGui::DockBuilderDockWindow("Model Viewer", m_centralID);

    ImGui::DockBuilderFinish(m_centralID);
}

void ModelViewer::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File", "Ctrl+O")) {
                OpenFile();
            }
            if (ImGui::MenuItem("Open Directory", "Ctrl+D")) {
                OpenDirectory();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Export to MQO", nullptr, false, m_renderer.HasModelLoaded())) {
                ExportModel(ExportFormat::MQO);
            }
            if (ImGui::MenuItem("Export to DAT", nullptr, false, m_renderer.HasModelLoaded())) {
                ExportModel(ExportFormat::DAT);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                m_appRunning = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ModelViewer::RenderFileExplorer()
{
    m_fileExplorer.Render();
}

void ModelViewer::RenderModelViewer()
{
    if (UI::BeginPanel("Model Viewer", nullptr, ImGuiWindowFlags_NoScrollbar)) {
        m_mouseOverViewportActive = ImGui::IsWindowFocused();
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        m_renderer.SetViewportSize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));

        GLuint textureID = m_renderer.GetRenderTextureID();
        ImGui::Image((ImTextureID)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 mousePos = ImGui::GetMousePos();

        m_viewportX = static_cast<int>(mousePos.x - imagePos.x);
        m_viewportY = static_cast<int>(mousePos.y - imagePos.y);
        m_mouseOverViewport = mousePos.x >= imagePos.x && mousePos.x < imagePos.x + viewportSize.x && mousePos.y >= imagePos.y && mousePos.y < imagePos.y + viewportSize.y;
        m_mouseOverViewportActive &= m_mouseOverViewport;

        float const buttonSize = 24.0f;
        float const padding = 4.0f;

        ImVec2 leftButtonPos {
            imagePos.x + padding,
            imagePos.y + padding
        };
        ImGui::SetCursorScreenPos(leftButtonPos);
        if (UI::OverlayButton("F", !m_settings.vertexMode, "Face Mode (F)", ImVec2(buttonSize, buttonSize))) {
            m_settings.vertexMode = false;
            m_renderer.GetModelRenderer().SetVertexMode(m_settings.vertexMode);
            m_settingsModified = true;
        }

        leftButtonPos.y += buttonSize + padding;
        ImGui::SetCursorScreenPos(leftButtonPos);
        if (UI::OverlayButton("V", m_settings.vertexMode, "Vertex Mode (V)", ImVec2(buttonSize, buttonSize))) {
            m_settings.vertexMode = true;
            m_renderer.GetModelRenderer().SetVertexMode(m_settings.vertexMode);
            m_settingsModified = true;
        }
        ImVec2 rightButtonPos {
            imagePos.x + viewportSize.x - buttonSize - padding,
            imagePos.y + padding
        };

        ColorMode currentMode = m_renderer.GetModelRenderer().GetColorMode();

        ImGui::SetCursorScreenPos(rightButtonPos);
        if (UI::OverlayButton("D", currentMode == ColorMode::Diffuse, "Diffuse Color Mode (1)", ImVec2(buttonSize, buttonSize))) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Diffuse);
        }

        rightButtonPos.y += buttonSize + padding;
        ImGui::SetCursorScreenPos(rightButtonPos);
        if (UI::OverlayButton("P", currentMode == ColorMode::Priority, "Priority Color Mode (2)", ImVec2(buttonSize, buttonSize))) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Priority);
        }

        rightButtonPos.y += buttonSize + padding;
        ImGui::SetCursorScreenPos(rightButtonPos);
        if (UI::OverlayButton("L", currentMode == ColorMode::Label, "Label Color Mode (3)", ImVec2(buttonSize, buttonSize))) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Label);
        }

        constexpr float gizmoSize = 60.0f;
        ImVec2 gizmoPos {
            imagePos.x + padding,
            imagePos.y + viewportSize.y - gizmoSize - padding
        };

        UI::Gizmo(gizmoPos, gizmoSize, m_renderer.GetViewMatrix());

        RenderFaceTooltip();
        RenderVertexTooltip();
    }
    UI::EndPanel();
}

void ModelViewer::RenderFaceTooltip()
{
    ModelRenderer& modelRenderer = m_renderer.GetModelRenderer();
    if (modelRenderer.GetHoveredFace() == -1 || !m_settings.faceTooltip) {
        return;
    }
    std::shared_ptr<ModelData> modelData = m_renderer.GetModelData();
    if (auto const* const face = modelData->GetFace(modelRenderer.GetHoveredFace()); face) {
        ImGui::SetNextWindowBgAlpha(0.8f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));

        ImGui::BeginTooltip();

        ImGui::Text("Face %d", modelRenderer.GetHoveredFace());

        ImGui::Separator();

        if (ImGui::BeginTable("FaceInfoColumns", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 70);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            if (m_settings.faceTooltipOptions.showVertices) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Vertices:");
                ImGui::TableNextColumn();
                ImGui::Text("%d, %d, %d", face->v1, face->v2, face->v3);
            }

            if (m_settings.faceTooltipOptions.showColor) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Color:");
                ImGui::TableNextColumn();

                uint32_t rgb = math::RunetekColor::HSLToRGB(face->color);
                float r = static_cast<float>(rgb >> 16 & 0xff) / 255.0f;
                float g = static_cast<float>(rgb >> 8 & 0xff) / 255.0f;
                float b = static_cast<float>(rgb & 0xff) / 255.0f;

                UI::ColorButton("##faceColor", ImVec4(r, g, b, 1.0f));
                ImGui::SameLine();
                ImGui::Text("HSL(%d) / RGB(0x%06X)", face->color, rgb & 0xffffff);
            }

            if (face->label && m_settings.faceTooltipOptions.showLabel) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Label:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *face->label);
            }

            if (face->type && m_settings.faceTooltipOptions.showType) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Type:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *face->type);
            }

            if (face->priority && m_settings.faceTooltipOptions.showPriority) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Priority:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *face->priority);
            }

            if (face->material && m_settings.faceTooltipOptions.showMaterial) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Material:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *face->material);
            }

            if (face->mapping && m_settings.faceTooltipOptions.showMapping) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Mapping:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *face->mapping);
            }

            ImGui::EndTable();
        }

        ImGui::EndTooltip();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
}

void ModelViewer::RenderVertexTooltip()
{
    ModelRenderer& modelRenderer = m_renderer.GetModelRenderer();
    if (modelRenderer.GetHoveredVertex() == -1 || !m_settings.vertexTooltip) {
        return;
    }
    std::shared_ptr<ModelData> modelData = m_renderer.GetModelData();
    if (Vertex const* vertex = modelData->GetVertex(modelRenderer.GetHoveredVertex()); vertex) {
        ImGui::SetNextWindowBgAlpha(0.8f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));

        ImGui::BeginTooltip();

        ImGui::Text("Vertex %d", modelRenderer.GetHoveredVertex());

        ImGui::Separator();

        if (ImGui::BeginTable("VertexInfoColumns", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 70);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            if (m_settings.vertexTooltipOptions.showPosition) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Position:");
                ImGui::TableNextColumn();
                ImGui::Text("%d, %d, %d", vertex->x, vertex->y, vertex->z);
            }

            if (vertex->label && m_settings.vertexTooltipOptions.showLabel) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Label:");
                ImGui::TableNextColumn();
                ImGui::Text("%d", *vertex->label);
            }
            ImGui::EndTable();
        }

        ImGui::EndTooltip();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
}

void ModelViewer::RenderOptionsPanel()
{
    if (UI::BeginPanel("Options", nullptr, ImGuiWindowFlags_None)) {
        if (ImGui::CollapsingHeader("Display Settings", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (UI::Checkbox("Show Wireframe", &m_settings.wireframeMode, "Toggle wireframe mode (W)")) {
                m_renderer.SetWireframeMode(m_settings.wireframeMode);
                m_settingsModified = true;
            }

            if (UI::SliderInt("Tile Grid Size", &m_settings.tileGridSize, 0, 10)) {
                m_renderer.SetTileGridSize(m_settings.tileGridSize);
                m_settingsModified = true;
            }

            if (UI::SliderFloat("Field of View", &m_settings.fov, 10.0f, 120.0f)) {
                m_renderer.SetFOV(m_settings.fov);
                m_settingsModified = true;
            }

            ImGui::Spacing();

            if (UI::Button("Reset Camera (R)", ImVec2(120, 0))) {
                m_renderer.ResetCamera();
            }
        }

        if (ImGui::CollapsingHeader("Colors", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (UI::ColorEdit4("Hover Highlight", &m_settings.hoveredHighlightColor[0], ImGuiColorEditFlags_AlphaBar)) {
                m_renderer.GetModelRenderer().SetHighlightColor(m_settings.hoveredHighlightColor);
                m_settingsModified = true;
            }
            if (UI::ColorEdit4("Select Highlight", &m_settings.selectedHighlightColor[0], ImGuiColorEditFlags_AlphaBar)) {
                m_renderer.GetModelRenderer().SetSelectedColor(m_settings.selectedHighlightColor);
                m_settingsModified = true;
            }
            if (UI::ColorEdit4("Wireframe Color", &m_settings.wireframeColor[0], ImGuiColorEditFlags_AlphaBar)) {
                m_renderer.GetModelRenderer().SetWireframeColor(m_settings.wireframeColor);
                m_settingsModified = true;
            }
        }

        if (ImGui::CollapsingHeader("Face Tooltip")) {
            if (UI::Checkbox("Enabled", &m_settings.faceTooltip)) {
                m_settingsModified = true;
            }

            if (m_settings.faceTooltip) {
                ImGui::Indent();

                if (UI::Checkbox("Show Vertices", &m_settings.faceTooltipOptions.showVertices)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Color", &m_settings.faceTooltipOptions.showColor)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Label", &m_settings.faceTooltipOptions.showLabel)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Type", &m_settings.faceTooltipOptions.showType)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Priority", &m_settings.faceTooltipOptions.showPriority)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Material", &m_settings.faceTooltipOptions.showMaterial)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Mapping", &m_settings.faceTooltipOptions.showMapping)) {
                    m_settingsModified = true;
                }

                ImGui::Unindent();
            }
        }

        if (ImGui::CollapsingHeader("Vertex Tooltip")) {
            if (UI::Checkbox("Enabled", &m_settings.vertexTooltip)) {
                m_settingsModified = true;
            }

            if (m_settings.vertexTooltip) {
                ImGui::Indent();

                if (UI::Checkbox("Show Position", &m_settings.vertexTooltipOptions.showPosition)) {
                    m_settingsModified = true;
                }

                if (UI::Checkbox("Show Label", &m_settings.vertexTooltipOptions.showLabel)) {
                    m_settingsModified = true;
                }

                ImGui::Unindent();
            }
        }

        if (ImGui::CollapsingHeader("UI Theme")) {
            if (UI::RadioButton("Dark Theme", m_settings.uiTheme == 0)) {
                m_settings.uiTheme = 0;
                UI::ApplyTheme(ThemeType::Dark);
                m_settingsModified = true;
            }

            if (UI::RadioButton("Light Theme", m_settings.uiTheme == 1)) {
                m_settings.uiTheme = 1;
                UI::ApplyTheme(ThemeType::Light);
                m_settingsModified = true;
            }
        }
    }
    UI::EndPanel();
}

void ModelViewer::HandleShortcuts()
{
    auto& io = ImGui::GetIO();

    if (io.WantTextInput) {
        return;
    }
    if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
        if (ImGui::IsKeyPressed(ImGuiKey_O, false)) {
            OpenFile();
        } else if (ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            OpenDirectory();
        } else if (ImGui::IsKeyPressed(ImGuiKey_F, false)) {
            m_fileExplorer.m_focusSearchNextFrame = true;
            return;
        }
    }
    if (m_mouseOverViewport) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            auto& renderer = m_renderer.GetModelRenderer();
            renderer.SetSelectedVertex(-1);
            renderer.SetSelectedFace(-1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F, false)) {
            m_settings.vertexMode = false;
            m_renderer.GetModelRenderer().SetVertexMode(m_settings.vertexMode);
            m_settingsModified = true;
        } else if (ImGui::IsKeyPressed(ImGuiKey_V, false)) {
            m_settings.vertexMode = true;
            m_renderer.GetModelRenderer().SetVertexMode(m_settings.vertexMode);
            m_settingsModified = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_1, false)) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Diffuse);
        } else if (ImGui::IsKeyPressed(ImGuiKey_2, false)) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Priority);
        } else if (ImGui::IsKeyPressed(ImGuiKey_3, false)) {
            m_renderer.GetModelRenderer().SetColorMode(ColorMode::Label);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            m_settings.wireframeMode = !m_settings.wireframeMode;
            m_renderer.SetWireframeMode(m_settings.wireframeMode);
            m_settingsModified = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            m_renderer.ResetCamera();
        }
    }
}

void ModelViewer::OpenDirectory()
{
    if (auto directoryPath = OpenDirectoryDialog()) {
        FileNode node {
            .name = directoryPath->filename().string(),
            .path = *directoryPath,
            .isDirectory = true,
            .isExpanded = true,
            .isScanned = false
        };
        auto const it = std::ranges::find_if(m_fileExplorer.m_rootNodes, [&](FileNode const& n) {
            return n.path == *directoryPath;
        });
        if (it == m_fileExplorer.m_rootNodes.end()) {
            m_fileExplorer.m_rootNodes.push_back(std::move(node));
            m_settings.files.push_back(*directoryPath);
            m_settingsModified = true;
            m_fileExplorer.m_dirty = true;
        }
    }
}

void ModelViewer::OpenFile()
{
    std::vector<std::pair<std::string, std::string>> filters = {
        { "Model Files", "*.mqo;*.dat" },
        { "MQO Files", "*.mqo" },
        { "DAT Files", "*.dat" }
    };

    if (auto const filePath = OpenFileDialog(filters); filePath && IsValidModelFile(*filePath)) {
        FileNode fileNode;
        fileNode.name = filePath->filename().string();
        fileNode.path = *filePath;
        fileNode.isDirectory = false;

        auto it = std::ranges::find_if(m_fileExplorer.m_rootNodes, [&](FileNode const& node) {
            return !node.isDirectory && node.path == *filePath;
        });
        if (it == m_fileExplorer.m_rootNodes.end()) {
            m_fileExplorer.m_rootNodes.push_back(std::move(fileNode));
            m_settings.files.push_back(*filePath);
            m_settingsModified = true;
            m_fileExplorer.m_dirty = true;
        }
        LoadModel(*filePath);
    } else if (filePath) {
        ShowError("Invalid File", "The selected file is not a supported model format.");
    }
}

bool ModelViewer::IsValidModelFile(std::filesystem::path const& path) const
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".mqo" || extension == ".dat";
}

void ModelViewer::ShowError(std::string const& title, std::string const& message)
{
    UI::ShowErrorAlert(title, message);
}

void ModelViewer::LoadModel(std::filesystem::path const& path)
{
    std::shared_ptr<ModelData> model = std::make_shared<ModelData>();
    if (!ModelLoader::LoadFromFile(*model, path)) {
        return;
    }

    m_renderer.GetModelRenderer().SetModelData(model);

    m_currentLoadedModelPath = path;

    int16_t maxX = std::numeric_limits<int16_t>::min();
    int16_t maxY = std::numeric_limits<int16_t>::min();
    int16_t maxZ = std::numeric_limits<int16_t>::min();
    int16_t minX = std::numeric_limits<int16_t>::max();
    int16_t minY = std::numeric_limits<int16_t>::max();
    int16_t minZ = std::numeric_limits<int16_t>::max();
    for (auto const& vertex : model->vertices) {
        maxX = std::max(maxX, vertex.x);
        maxY = std::max(maxY, static_cast<int16_t>(-vertex.y));
        maxZ = std::max(maxZ, vertex.z);
        minX = std::min(minX, vertex.x);
        minY = std::min(minY, static_cast<int16_t>(-vertex.y));
        minZ = std::min(minZ, vertex.z);
    }

    glm::vec3 min(minX, minY, minZ);
    glm::vec3 max(maxX, maxY, maxZ);
    glm::vec3 center = (min + max) * 0.5f;
    m_renderer.SetCameraTarget(center);
}

void ModelViewer::LoadSettings()
{
    try {
        std::ifstream file(m_settingsPath);
        if (!file.is_open()) {
            return;
        }

        nlohmann::json j;
        file >> j;

        m_settings.wireframeMode = j.value("wireframeMode", false);
        m_settings.tileGridSize = j.value("tileGridSize", 1);

        m_settings.fov = j.value("cameraFov", 45.0f);

        m_settings.windowWidth = j.value("windowWidth", 1280);
        m_settings.windowHeight = j.value("windowHeight", 720);
        m_settings.windowPosX = j.value("windowPosX", 100);
        m_settings.windowPosY = j.value("windowPosY", 100);

        m_settings.vertexMode = j.value("vertexMode", false);

        m_settings.faceTooltip = j.value("faceTooltip", true);
        if (j.contains("faceTooltipOptions")) {
            auto& tooltipOpts = j["faceTooltipOptions"];
            m_settings.faceTooltipOptions.showVertices = tooltipOpts.value("showVertices", true);
            m_settings.faceTooltipOptions.showColor = tooltipOpts.value("showColor", true);
            m_settings.faceTooltipOptions.showLabel = tooltipOpts.value("showLabel", true);
            m_settings.faceTooltipOptions.showType = tooltipOpts.value("showType", true);
            m_settings.faceTooltipOptions.showPriority = tooltipOpts.value("showPriority", true);
            m_settings.faceTooltipOptions.showMaterial = tooltipOpts.value("showMaterial", true);
            m_settings.faceTooltipOptions.showMapping = tooltipOpts.value("showMapping", true);
        }
        m_settings.vertexTooltip = j.value("vertexTooltip", true);
        if (j.contains("vertexTooltipOptions")) {
            auto& tooltipOpts = j["vertexTooltipOptions"];
            m_settings.vertexTooltipOptions.showPosition = tooltipOpts.value("showPosition", true);
            m_settings.vertexTooltipOptions.showLabel = tooltipOpts.value("showLabel", true);
        }
        if (j.contains("hoveredHighlightColor")) {
            if (auto& color = j["hoveredHighlightColor"]; color.is_array() && color.size() == 4) {
                for (int i = 0; i < 4; i++) {
                    m_settings.hoveredHighlightColor[i] = color[i];
                }
            }
        }
        if (j.contains("selectedHighlightColor")) {
            if (auto& color = j["selectedHighlightColor"]; color.is_array() && color.size() == 4) {
                for (int i = 0; i < 4; i++) {
                    m_settings.selectedHighlightColor[i] = color[i];
                }
            }
        }
        if (j.contains("wireframeColor")) {
            if (auto& color = j["wireframeColor"]; color.is_array() && color.size() == 4) {
                for (int i = 0; i < 4; i++) {
                    m_settings.wireframeColor[i] = color[i];
                }
            }
        }

        m_settings.uiTheme = j.value("uiTheme", 0);

        if (j["files"].is_array()) {
            for (auto& path : j["files"]) {
                std::filesystem::path filePath = path.get<std::string>();
                if (!std::filesystem::exists(filePath)) {
                    continue;
                }
                FileNode node {
                    .name = filePath.filename().string(),
                    .path = filePath,
                    .isDirectory = std::filesystem::is_directory(filePath),
                    .isExpanded = node.isDirectory,
                    .isScanned = false,
                };
                m_fileExplorer.m_rootNodes.push_back(node);
                m_fileExplorer.m_dirty = true;
                m_settings.files.push_back(filePath);
            }
        }
    } catch (std::exception const& e) {
        ShowError("Settings Error", std::string("Failed to load settings: ") + e.what());
    }
}

void ModelViewer::SaveSettings()
{
    try {
        nlohmann::json j;

        j["wireframeMode"] = m_settings.wireframeMode;
        j["tileGridSize"] = m_settings.tileGridSize;

        j["cameraFov"] = m_settings.fov;

        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        j["windowWidth"] = width;
        j["windowHeight"] = height;

        int posX, posY;
        glfwGetWindowPos(m_window, &posX, &posY);
        j["windowPosX"] = posX;
        j["windowPosY"] = posY;
        j["vertexMode"] = m_settings.vertexMode;
        j["faceTooltip"] = m_settings.faceTooltip;

        nlohmann::json faceTooltipOpts;
        faceTooltipOpts["showVertices"] = m_settings.faceTooltipOptions.showVertices;
        faceTooltipOpts["showColor"] = m_settings.faceTooltipOptions.showColor;
        faceTooltipOpts["showLabel"] = m_settings.faceTooltipOptions.showLabel;
        faceTooltipOpts["showType"] = m_settings.faceTooltipOptions.showType;
        faceTooltipOpts["showPriority"] = m_settings.faceTooltipOptions.showPriority;
        faceTooltipOpts["showMaterial"] = m_settings.faceTooltipOptions.showMaterial;
        faceTooltipOpts["showMapping"] = m_settings.faceTooltipOptions.showMapping;
        j["faceTooltipOptions"] = faceTooltipOpts;

        j["vertexTooltip"] = m_settings.vertexTooltip;
        nlohmann::json vertexTooltipOpts;
        vertexTooltipOpts["showPosition"] = m_settings.vertexTooltipOptions.showPosition;
        vertexTooltipOpts["showLabel"] = m_settings.vertexTooltipOptions.showLabel;
        j["vertexTooltipOptions"] = vertexTooltipOpts;

        j["hoveredHighlightColor"] = {
            m_settings.hoveredHighlightColor[0],
            m_settings.hoveredHighlightColor[1],
            m_settings.hoveredHighlightColor[2],
            m_settings.hoveredHighlightColor[3]
        };

        j["selectedHighlightColor"] = {
            m_settings.selectedHighlightColor[0],
            m_settings.selectedHighlightColor[1],
            m_settings.selectedHighlightColor[2],
            m_settings.selectedHighlightColor[3]
        };

        j["wireframeColor"] = {
            m_settings.wireframeColor[0],
            m_settings.wireframeColor[1],
            m_settings.wireframeColor[2],
            m_settings.wireframeColor[3]
        };

        j["uiTheme"] = m_settings.uiTheme;

        nlohmann::json fileArray = nlohmann::json::array();

        for (auto const& node : m_fileExplorer.m_rootNodes) {
            fileArray.push_back(node.path.string());
        }

        j["files"] = fileArray;

        std::ofstream file(m_settingsPath);
        file << j.dump(2);

        m_settingsModified = false;
    } catch (std::exception const& e) {
        ShowError("Settings Error", std::string("Failed to save settings: ") + e.what());
    }
}

void ModelViewer::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* modelViewer = static_cast<ModelViewer*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        modelViewer->m_leftMousePressed = (action == GLFW_PRESS);
        if (modelViewer->m_leftMousePressed) {
            modelViewer->m_lastPressWasDrag = false;
            double xpos;
            double ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            modelViewer->m_prevMousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        modelViewer->m_middleMousePressed = (action == GLFW_PRESS);
    }
}

void ModelViewer::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto* viewer = static_cast<ModelViewer*>(glfwGetWindowUserPointer(window));
    float deltaX = static_cast<float>(xpos) - viewer->m_prevMousePos.x;
    float deltaY = viewer->m_prevMousePos.y - static_cast<float>(ypos);
    if (viewer->m_leftMousePressed && viewer->m_mouseOverViewportActive) {
        if (deltaX != 0 || deltaY != 0) {
            viewer->m_lastPressWasDrag = true;
        }
        viewer->m_renderer.OrbitCamera(deltaX, deltaY);
    } else if (viewer->m_middleMousePressed) {
        viewer->m_renderer.PanCamera(deltaX, deltaY);
    }
    viewer->m_prevMousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };
}

void ModelViewer::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto* viewer = static_cast<ModelViewer*>(glfwGetWindowUserPointer(window));
    if (viewer->m_mouseOverViewportActive) {
        viewer->m_renderer.ZoomCamera(-static_cast<float>(yoffset) * 300.0f * viewer->m_deltaTime);
    }
}

void ModelViewer::RenderModelStatsPanel()
{
    if (UI::BeginPanel("Model", nullptr, ImGuiWindowFlags_None)) {
        std::shared_ptr<ModelData> modelData = m_renderer.GetModelData();
        if (!modelData || modelData->vertices.empty() || modelData->faces.empty()) {
            ImGui::TextWrapped("No model loaded or model data is empty.");
            UI::EndPanel();
            return;
        }

        if (ImGui::CollapsingHeader("Model Statistics", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("ModelStatsTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Vertex Count:");
                ImGui::TableNextColumn();
                ImGui::Text("%zu", modelData->vertices.size());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Face Count:");
                ImGui::TableNextColumn();
                ImGui::Text("%zu", modelData->faces.size());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Texture Count:");
                ImGui::TableNextColumn();
                ImGui::Text("%zu", modelData->textures.size());

                if (modelData->priority) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Model Priority:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", *modelData->priority);
                }

                glm::vec3 min(FLT_MAX), max(-FLT_MAX);
                for (auto const& vertex : modelData->vertices) {
                    min.x = std::min(min.x, static_cast<float>(vertex.x));
                    min.y = std::min(min.y, static_cast<float>(vertex.y));
                    min.z = std::min(min.z, static_cast<float>(vertex.z));

                    max.x = std::max(max.x, static_cast<float>(vertex.x));
                    max.y = std::max(max.y, static_cast<float>(vertex.y));
                    max.z = std::max(max.z, static_cast<float>(vertex.z));
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Bounds Min:");
                ImGui::TableNextColumn();
                ImGui::Text("(%.1f, %.1f, %.1f)", min.x, min.y, min.z);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Bounds Max:");
                ImGui::TableNextColumn();
                ImGui::Text("(%.1f, %.1f, %.1f)", max.x, max.y, max.z);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Dimensions:");
                ImGui::TableNextColumn();
                ImGui::Text("(%.1f, %.1f, %.1f)", max.x - min.x, max.y - min.y, max.z - min.z);

                ImGui::EndTable();
            }
        }

        int selectedFaceIndex = m_renderer.GetModelRenderer().GetSelectedFace();
        if (ImGui::CollapsingHeader("Selected Face", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (selectedFaceIndex == -1) {
                ImGui::TextWrapped("No face selected. Click on a face in the model viewer to select it.");
            } else {
                Face const* selectedFace = modelData->GetFace(selectedFaceIndex);
                if (selectedFace) {
                    RenderFaceDetails("Selected", selectedFaceIndex, *selectedFace, modelData->vertices);
                } else {
                    ImGui::TextWrapped("Selected face index is invalid.");
                }
            }
        }

        int hoveredFaceIndex = m_renderer.GetModelRenderer().GetHoveredFace();
        if (ImGui::CollapsingHeader("Hovered Face", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (hoveredFaceIndex == -1) {
                ImGui::TextWrapped("No face hovered. Move cursor over a face in the model viewer.");
            } else {
                Face const* hoveredFace = modelData->GetFace(hoveredFaceIndex);
                if (hoveredFace) {
                    RenderFaceDetails("Hovered", hoveredFaceIndex, *hoveredFace, modelData->vertices);
                } else {
                    ImGui::TextWrapped("Hovered face index is invalid.");
                }
            }
        }

        int selectedVertexIndex = m_renderer.GetModelRenderer().GetSelectedVertex();
        if (ImGui::CollapsingHeader("Selected Vertex", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (selectedVertexIndex == -1) {
                ImGui::TextWrapped("No vertex selected. Click on a vertex in the model viewer to select it.");
            } else {
                Vertex const* selectedVertex = modelData->GetVertex(selectedVertexIndex);
                if (selectedVertex) {
                    RenderVertexDetails("Selected", selectedVertexIndex, *selectedVertex);
                } else {
                    ImGui::TextWrapped("Selected vertex index is invalid.");
                }
            }
        }

        int hoveredVertexIndex = m_renderer.GetModelRenderer().GetHoveredVertex();
        if (ImGui::CollapsingHeader("Hovered Vertex", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (hoveredVertexIndex == -1) {
                ImGui::TextWrapped("No vertex hovered. Move cursor over a vertex in the model viewer.");
            } else {
                Vertex const* hoveredVertex = modelData->GetVertex(hoveredVertexIndex);
                if (hoveredVertex) {
                    RenderVertexDetails("Hovered", hoveredVertexIndex, *hoveredVertex);
                } else {
                    ImGui::TextWrapped("Hovered vertex index is invalid.");
                }
            }
        }
    }
    UI::EndPanel();
}

void ModelViewer::RenderFaceDetails(char const* label, int faceIndex, Face const& face, std::vector<Vertex> const& vertices)
{
    if (ImGui::BeginTable("FaceDetailsTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Face Index:");
        ImGui::TableNextColumn();
        ImGui::Text("%d", faceIndex);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Vertex Indices:");
        ImGui::TableNextColumn();
        ImGui::Text("%d, %d, %d", face.v1, face.v2, face.v3);

        if (face.v1 < vertices.size() && face.v2 < vertices.size() && face.v3 < vertices.size()) {
            Vertex const& v1 = vertices[face.v1];
            Vertex const& v2 = vertices[face.v2];
            Vertex const& v3 = vertices[face.v3];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Vertex 1:");
            ImGui::TableNextColumn();
            ImGui::Text("(%d, %d, %d)", v1.x, v1.y, v1.z);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Vertex 2:");
            ImGui::TableNextColumn();
            ImGui::Text("(%d, %d, %d)", v2.x, v2.y, v2.z);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Vertex 3:");
            ImGui::TableNextColumn();
            ImGui::Text("(%d, %d, %d)", v3.x, v3.y, v3.z);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Color (HSL):");
        ImGui::TableNextColumn();
        ImGui::Text("%d", face.color);

        uint32_t rgb = math::RunetekColor::HSLToRGB(face.color);
        float r = ((rgb >> 16) & 0xff) / 255.0f;
        float g = ((rgb >> 8) & 0xff) / 255.0f;
        float b = (rgb & 0xff) / 255.0f;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Color (RGB):");
        ImGui::TableNextColumn();
        UI::ColorButton("##faceColor", ImVec4(r, g, b, 1.0f));
        ImGui::SameLine();
        ImGui::Text("0x%06X", rgb & 0xFFFFFF);

        if (face.type) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Type:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.type);
        }

        if (face.priority) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Priority:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.priority);
        }

        if (face.label) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Label:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.label);
        }

        if (face.material) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Material:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.material);
        }

        if (face.mapping) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Mapping:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.mapping);
        }

        if (face.trans) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Transparency:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *face.trans);
        }

        ImGui::EndTable();
    }
}

void ModelViewer::RenderVertexDetails(char const* label, int vertexIndex, Vertex const& vertex)
{
    if (ImGui::BeginTable("VertexDetailsTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Vertex Index:");
        ImGui::TableNextColumn();
        ImGui::Text("%d", vertexIndex);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Position:");
        ImGui::TableNextColumn();
        ImGui::Text("%d, %d, %d", vertex.x, vertex.y, vertex.z);

        if (vertex.label) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Label:");
            ImGui::TableNextColumn();
            ImGui::Text("%d", *vertex.label);
        }

        ImGui::EndTable();
    }
}

void ModelViewer::ExportModel(ExportFormat format)
{
    if (!m_renderer.HasModelLoaded()) {
        ShowError("Export Error", "No model is currently loaded to export.");
        return;
    }

    std::shared_ptr<ModelData> modelData = m_renderer.GetModelData();

    std::vector<std::pair<std::string, std::string>> filters;
    char const* extension;
    if (format == ExportFormat::MQO) {
        filters.push_back({ "Metasequoia Files", "*.mqo" });
        extension = ".mqo";
    } else {
        filters.push_back({ "DAT Files", "*.dat" });
        extension = ".dat";
    }
    std::optional<std::filesystem::path> savePath = SaveFileDialog("Save MQO File", "", filters);
    if (!savePath.has_value()) {
        return;
    }

    std::filesystem::path& path = *savePath;
    if (path.extension() != extension) {
        path.replace_extension(extension);
    }

    bool success;
    if (format == ExportFormat::DAT) {
        success = ModelExporter::ExportV1(*modelData, path);
    } else {
        success = ModelExporter::ExportMQO(*modelData, path);
    }
    if (success) {
        UI::ShowAlert("Export Successful", "Model has been exported to " + path.string(), AlertType::Info);
    } else {
        ShowError("Export Failed", "Failed to export the model to " + path.string());
    }
}
}
