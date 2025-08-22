#include "FileExplorer.h"

#include "ModelViewer.h"

#include "imgui_internal.h"

#include <imgui.h>

#ifdef IMP_PLATFORM_WINDOWS
#    include <ObjectArray.h>
#    include <shellapi.h>
#    include <windows.h>
static void ShowInExplorer(std::filesystem::path const& path)
{
    std::wstring widePath = path.wstring();
    std::wstring params = L"/select,\"" + widePath + L"\"";
    ShellExecuteW(nullptr, L"open", L"explorer.exe", params.c_str(), nullptr, SW_SHOWNORMAL);
}
#endif

namespace imp {
void FileExplorer::Render()
{
    ImGui::Begin("File Explorer");

    RenderSearchBar();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_dirty) {
        RefreshFilter();
        m_dirty = false;
    }

    RenderNodes();

    ImGui::End();
}

void FileExplorer::RenderSearchBar()
{
    float searchWidth = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(28, 6));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::SetNextItemWidth(searchWidth);

    if (m_focusSearchNextFrame) {
        m_focusSearchNextFrame = false;
        ImGui::SetKeyboardFocusHere();
    }

    bool searchChanged = ImGui::InputTextWithHint("##search", "Search files...",
        m_searchBuffer, sizeof(m_searchBuffer));

    ImVec2 iconPos = ImGui::GetItemRectMin();
    iconPos.x += 8.0f;
    iconPos.y += ImGui::GetItemRectSize().y * 0.5f - 6.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    constexpr float iconSize = 12.0f;
    ImU32 iconColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    drawList->AddCircle(ImVec2(iconPos.x + iconSize * 0.4f, iconPos.y + iconSize * 0.4f),
        iconSize * 0.4f, iconColor, 8, 1.5f);
    drawList->AddLine(ImVec2(iconPos.x + iconSize * 0.7f, iconPos.y + iconSize * 0.7f),
        ImVec2(iconPos.x + iconSize * 1.0f, iconPos.y + iconSize * 1.0f),
        iconColor, 1.5f);

    if (searchChanged) {
        m_searchQuery = m_searchBuffer;
        m_dirty = true;
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void FileExplorer::RenderNodes()
{
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_filteredNodes.size()), 18.0f);
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            FileNode* node = m_filteredNodes[i];
            RenderNode(*node);
        }
    }
    clipper.End();
}

void FileExplorer::RenderNode(FileNode& node)
{
    constexpr float indentSize = 8.0f;
    ImGui::PushID(&node);
    if (node.depth > 0) {
        ImGui::Indent(node.depth * indentSize);
    }
    if (node.isDirectory) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
        if (node.isExpanded) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 2.0f));
        bool open = ImGui::TreeNodeEx(node.name.c_str(), flags);
        ImGui::PopStyleVar();
        if (open) {
            ImGui::TreePop();
        }
        if (open != node.isExpanded) {
            node.isExpanded = open;
            m_dirty = true;
        }
        if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > 0.5f) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(node.path.string().c_str());
            ImGui::EndTooltip();
        }
        if (open) {
            if (!node.isScanned) {
                ScanDirectory(node);
                node.isScanned = true;
                m_dirty = true;
            }
        }
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Remove")) {
                RemoveNode(&node);
            }
            ImGui::EndPopup();
        }
    } else {
        bool selected = m_app.m_currentLoadedModelPath == node.path;
        if (ImGui::Selectable(node.name.c_str(), selected, 0, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            m_app.LoadModel(node.path);
        }
        if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > 0.5f) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(node.path.string().c_str());
            ImGui::EndTooltip();
        }
        if (ImGui::BeginPopupContextItem()) {
#ifdef IMP_PLATFORM_WINDOWS
            if (ImGui::MenuItem("Show in Explorer")) {
                ShowInExplorer(node.path);
            }
#endif
            if (ImGui::MenuItem("Remove")) {
                RemoveNode(&node);
            }
            ImGui::EndPopup();
        }
    }

    if (node.depth > 0) {
        ImGui::Unindent(node.depth * indentSize);
    }
    ImGui::PopID();
}

void FileExplorer::ScanDirectory(FileNode& node)
{
    for (auto const& entry : std::filesystem::directory_iterator(node.path)) {
        FileNode child {
            .name = entry.path().filename().string(),
            .path = entry.path(),
            .isDirectory = entry.is_directory(),
        };
        node.children.push_back(std::move(child));
    }
    std::ranges::sort(node.children, [](auto const& a, auto const& b) {
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        return a.name < b.name;
    });
}

void FileExplorer::RefreshFilter()
{
    m_filteredNodes.clear();
    auto visit = [&](auto&& self, FileNode& node) -> void {
        if (m_searchQuery.empty() || node.name.find(m_searchQuery) != std::string::npos) {
            m_filteredNodes.push_back(&node);
        }
        if (node.isDirectory && node.isExpanded) {
            int subDepth = node.depth + 1;
            for (auto& child : node.children) {
                child.depth = subDepth;
                self(self, child);
            }
        }
    };
    for (auto& root : m_rootNodes) {
        root.depth = 0;
        visit(visit, root);
    }
}

void FileExplorer::RemoveNode(FileNode* node)
{
    // TODO(Walied): Find hte node parent and remove it instead.
    m_dirty = true;
}
}
