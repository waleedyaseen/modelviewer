#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace imp {
struct FileNode {
    std::string name;
    std::filesystem::path path;
    bool isDirectory { false };
    bool isExpanded { false };
    bool isScanned { false };
    std::vector<FileNode> children;
    int depth { 0 };
};
class ModelViewer;
class FileExplorer {
public:
    explicit FileExplorer(ModelViewer& app)
        : m_app(app)
    {
        //  Do nothing.
    }

    void Render();

    friend class ModelViewer;

private:
    void RenderSearchBar();
    void RenderNodes();
    void RenderNode(FileNode& node);
    void ScanDirectory(FileNode& node);
    void RefreshFilter();
    void RemoveNode(FileNode* node);

    ModelViewer& m_app;
    std::vector<FileNode> m_rootNodes;
    std::vector<FileNode*> m_filteredNodes;
    std::string m_searchQuery;
    char m_searchBuffer[256] {};
    bool m_focusSearchNextFrame { false };
    bool m_dirty { true };
};
}
