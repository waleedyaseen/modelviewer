#pragma once

#include "Model.h"

#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace imp {
enum class ColorMode : uint8_t {
    Diffuse,
    Priority,
    Label
};

class ModelRenderer {
public:
    ModelRenderer();
    ~ModelRenderer();

    void Initialize();
    void SetModelData(std::shared_ptr<ModelData> const& modelData);
    void Render(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix);

    int Pick(int x, int y, int viewportWidth, int viewportHeight, glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix);
    void SetViewportSize(int width, int height);

    int GetHoveredFace() const
    {
        return m_hoveredFace;
    }

    void SetHoveredFace(int faceIndex)
    {
        if (faceIndex < -1 || faceIndex >= m_faceCount) {
            m_hoveredFace = -1;
        } else {
            m_hoveredFace = faceIndex;
        }
    }

    int GetSelectedFace() const
    {
        return m_selectedFace;
    }

    void SetSelectedFace(int faceIndex)
    {
        if (faceIndex < -1 || faceIndex >= m_faceCount) {
            m_selectedFace = -1;
        } else {
            m_selectedFace = faceIndex;
        }
    }

    int GetHoveredVertex() const
    {
        return m_hoveredVertex;
    }

    void SetHoveredVertex(int vertexIndex)
    {
        if (vertexIndex < -1 || vertexIndex >= m_vertexCount) {
            m_hoveredVertex = -1;
        } else {
            m_hoveredVertex = vertexIndex;
        }
    }

    int GetSelectedVertex() const
    {
        return m_selectedVertex;
    }

    void SetSelectedVertex(int vertexIndex)
    {
        if (vertexIndex < -1 || vertexIndex >= m_vertexCount) {
            m_selectedVertex = -1;
        } else {
            m_selectedVertex = vertexIndex;
        }
    }

    std::shared_ptr<ModelData> const& GetModelData() const
    {
        return m_modelData;
    }

    void SetWireframeMode(bool enabled)
    {
        m_wireframeMode = enabled;
    }

    void SetVertexMode(bool enabled)
    {
        m_vertexMode = enabled;
    }

    void SetHighlightColor(glm::vec4 const& color)
    {
        m_highlightColor = color;
    }

    void SetSelectedColor(glm::vec4 const& color)
    {
        m_selectedColor = color;
    }

    void SetWireframeColor(glm::vec4 const& color)
    {
        m_wireframeColor = color;
    }

    ColorMode GetColorMode() const
    {
        return m_colorMode;
    }

    void SetColorMode(ColorMode mode)
    {
        m_colorMode = mode;
        if (m_modelData) {
            UpdateColorData();
        }
    }

private:
    void SetupShaders();
    void UpdateBuffers(ModelData const& modelData);
    void SetupPickingFramebuffer();
    void RenderWireframe(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix);
    void RenderPoints(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix);
    void RenderForPicking(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix);
    void AddFaceVertices(Face const& face, std::vector<Vertex> const& vertices, int faceIndex);
    void AddVertexColors(float r, float g, float b);
    void UploadVertexData();
    void UpdateColorData();

    ShaderProgram m_shaderProgram;
    ShaderProgram m_pickingShaderProgram;
    uint32_t m_vao { 0 };
    uint32_t m_vertexVBO { 0 };
    uint32_t m_colorVBO { 0 };
    uint32_t m_elementBuffer { 0 };
    std::vector<float> m_vertexData;
    std::vector<float> m_colorData;
    std::vector<uint32_t> m_indices;
    int32_t m_vertexCount { 0 };
    int32_t m_faceCount { 0 };
    std::shared_ptr<ModelData> m_modelData;
    glm::vec4 m_highlightColor { 1.0f, 0.8f, 0.2f, 1.0f };
    glm::vec4 m_selectedColor { 0.2f, 0.8f, 1.0f, 1.0f };
    glm::vec4 m_wireframeColor { 0.0f, 0.0f, 0.0f, 1.0f };
    int32_t m_hoveredFace { -1 };
    int32_t m_selectedFace { -1 };
    int32_t m_hoveredVertex { -1 };
    int32_t m_selectedVertex { -1 };
    uint32_t m_pickingFBO { 0 };
    uint32_t m_pickingTexture { 0 };
    uint32_t m_pickingDepthRBO { 0 };
    int32_t m_viewportWidth { 1 };
    int32_t m_viewportHeight { 1 };
    ColorMode m_colorMode { ColorMode::Diffuse };
    bool m_wireframeMode { false };
    bool m_vertexMode { false };
};
}
