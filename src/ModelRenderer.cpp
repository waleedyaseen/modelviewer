#include "ModelRenderer.h"

#include "Dialogs.h"
#include "Renderer.h"
#include "RunetekColor.h"
// This comment is to prevent auto formatter from reorganizing glad to be after GLFW
#include <glad/glad.h>
// ---------------------------------------------------------------------------------
#include <GLFW/glfw3.h>

namespace imp {
#define PTR_OFFSET(x) ((char*)nullptr + (x))
#include "shaders/ModelGeom.fs"
#include "shaders/ModelGeom.vs"
#include "shaders/ModelPick.fs"
#include "shaders/ModelPick.vs"

ModelRenderer::ModelRenderer()
{
    // Do nothing.
}

ModelRenderer::~ModelRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vertexVBO);
    glDeleteBuffers(1, &m_colorVBO);
    glDeleteBuffers(1, &m_elementBuffer);
    glDeleteFramebuffers(1, &m_pickingFBO);
    glDeleteTextures(1, &m_pickingTexture);
    glDeleteRenderbuffers(1, &m_pickingDepthRBO);
}

void ModelRenderer::Initialize()
{
    SetupShaders();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vertexVBO);
    glGenBuffers(1, &m_colorVBO);
    glGenBuffers(1, &m_elementBuffer);

    SetupPickingFramebuffer();
}

void ModelRenderer::SetupShaders()
{
    m_shaderProgram.Create(modelGeomVertexShader, modelGeomFragmentShader);
    m_pickingShaderProgram.Create(modelPickVertexShader, modelPickFragmentShader);
}

void ModelRenderer::SetupPickingFramebuffer()
{
    glGenFramebuffers(1, &m_pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);

    glGenTextures(1, &m_pickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_viewportWidth, m_viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pickingTexture, 0);

    glGenRenderbuffers(1, &m_pickingDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_pickingDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_viewportWidth, m_viewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pickingDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ShowFatalDialog("Error", "Picking framebuffer is not complete!");
        std::exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModelRenderer::SetViewportSize(int width, int height)
{
    if (width == m_viewportWidth && height == m_viewportHeight) {
        return;
    }

    m_viewportWidth = width;
    m_viewportHeight = height;

    glBindTexture(GL_TEXTURE_2D, m_pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, m_pickingDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
}

void ModelRenderer::SetModelData(std::shared_ptr<ModelData> const& modelData)
{
    m_modelData = modelData;
    UpdateBuffers(*modelData);
}

void ModelRenderer::UpdateBuffers(ModelData const& modelData)
{
    m_vertexData.clear();
    m_colorData.clear();
    m_indices.clear();

    std::vector<Vertex> const& vertices = modelData.vertices;
    std::vector<Face> const& faces = modelData.faces;
    m_vertexCount = static_cast<int32_t>(vertices.size());
    m_faceCount = static_cast<int32_t>(faces.size());

    uint32_t indexCounter = 0;
    for (size_t i = 0; i < faces.size(); ++i) {
        Face const& face = faces[i];

        uint32_t rgb = math::RunetekColor::HSLToRGB(face.color);
        float r = static_cast<float>(rgb >> 16 & 0xff) / 255.0f;
        float g = static_cast<float>(rgb >> 8 & 0xff) / 255.0f;
        float b = static_cast<float>(rgb & 0xff) / 255.0f;

        AddFaceVertices(face, vertices, static_cast<int>(i));
        AddVertexColors(r, g, b);

        m_indices.push_back(indexCounter + 2);
        m_indices.push_back(indexCounter + 1);
        m_indices.push_back(indexCounter);
        indexCounter += 3;
    }

    UploadVertexData();

    if (m_colorMode != ColorMode::Diffuse) {
        UpdateColorData();
    }
}

void ModelRenderer::AddFaceVertices(Face const& face, std::vector<Vertex> const& vertices, int faceIndex)
{
    Vertex const& v1 = vertices[face.v1];
    Vertex const& v2 = vertices[face.v2];
    Vertex const& v3 = vertices[face.v3];

    m_vertexData.push_back(v1.x);
    m_vertexData.push_back(static_cast<float>(-v1.y));
    m_vertexData.push_back(v1.z);
    m_vertexData.push_back(face.v1);
    m_vertexData.push_back(static_cast<float>(faceIndex));

    m_vertexData.push_back(v2.x);
    m_vertexData.push_back(static_cast<float>(-v2.y));
    m_vertexData.push_back(v2.z);
    m_vertexData.push_back(face.v2);
    m_vertexData.push_back(static_cast<float>(faceIndex));

    m_vertexData.push_back(v3.x);
    m_vertexData.push_back(static_cast<float>(-v3.y));
    m_vertexData.push_back(v3.z);
    m_vertexData.push_back(face.v3);
    m_vertexData.push_back(static_cast<float>(faceIndex));
}

void ModelRenderer::AddVertexColors(float r, float g, float b)
{
    for (int i = 0; i < 3; i++) {
        m_colorData.push_back(r);
        m_colorData.push_back(g);
        m_colorData.push_back(b);
        m_colorData.push_back(1.0f);
    }
}

void ModelRenderer::UploadVertexData()
{
    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vertexVBO);
        glGenBuffers(1, &m_colorVBO);
        glGenBuffers(1, &m_elementBuffer);
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float), m_vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), PTR_OFFSET(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), PTR_OFFSET(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), PTR_OFFSET(4 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    glBufferData(GL_ARRAY_BUFFER, m_colorData.size() * sizeof(float), m_colorData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), PTR_OFFSET(0));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ModelRenderer::Render(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
{
    if (m_indices.empty()) {
        return;
    }

    m_shaderProgram.Bind();
    m_shaderProgram.SetUniform("uViewMatrix", viewMatrix);
    m_shaderProgram.SetUniform("uProjectionMatrix", projectionMatrix);
    m_shaderProgram.SetUniform("uHighlightFace", m_hoveredFace);
    m_shaderProgram.SetUniform("uSelectedFace", m_selectedFace);
    m_shaderProgram.SetUniform("uHighlightVertex", m_hoveredVertex);
    m_shaderProgram.SetUniform("uSelectedVertex", m_selectedVertex);
    m_shaderProgram.SetUniform("uHighlightColor", m_highlightColor);
    m_shaderProgram.SetUniform("uSelectedColor", m_selectedColor);
    m_shaderProgram.SetUniform("uOverrideColorEnabled", 0);
    m_shaderProgram.SetUniform("uVertexMode", m_vertexMode ? 1 : 0);
    m_shaderProgram.SetUniform("uHighlight", m_vertexMode ? 0 : 1);

    glBindVertexArray(m_vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);

    if (m_wireframeMode || m_vertexMode) {
        RenderWireframe(viewMatrix, projectionMatrix);
    }
    if (m_vertexMode) {
        RenderPoints(viewMatrix, projectionMatrix);
    }
    glBindVertexArray(0);
}

void ModelRenderer::RenderWireframe(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
{
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_shaderProgram.SetUniform("uOverrideColorEnabled", 1);
    m_shaderProgram.SetUniform("uOverrideColor", m_wireframeColor);
    m_shaderProgram.SetUniform("uHighlightVertex", m_hoveredVertex);
    m_shaderProgram.SetUniform("uSelectedVertex", m_selectedVertex);
    m_shaderProgram.SetUniform("uHighlight", 0);
    glLineWidth(1.0f);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDepthMask(GL_TRUE);
}

void ModelRenderer::RenderPoints(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
{
    m_shaderProgram.Bind();
    m_shaderProgram.SetUniform("uViewMatrix", viewMatrix);
    m_shaderProgram.SetUniform("uProjectionMatrix", projectionMatrix);
    m_shaderProgram.SetUniform("uOverrideColorEnabled", 1);
    m_shaderProgram.SetUniform("uOverrideColor", m_wireframeColor);
    m_shaderProgram.SetUniform("uHighlight", m_vertexMode ? 1 : 0);

    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    glBindVertexArray(m_vao);
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_vertexData.size() / 4));
    glBindVertexArray(0);
    glDisable(GL_POLYGON_OFFSET_LINE);
}

void ModelRenderer::RenderForPicking(glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
{
    if (m_indices.empty()) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pickingShaderProgram.Bind();

    m_pickingShaderProgram.SetUniform("uViewMatrix", viewMatrix);
    m_pickingShaderProgram.SetUniform("uProjectionMatrix", projectionMatrix);
    m_pickingShaderProgram.SetUniform("uVertexMode", m_vertexMode ? 1 : 0);

    glBindVertexArray(m_vao);
    if (m_vertexMode) {
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_vertexData.size() / 4));
    } else {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int ModelRenderer::Pick(int x, int y, int viewportWidth, int viewportHeight, glm::mat4 const& viewMatrix, glm::mat4 const& projectionMatrix)
{
    if (m_indices.empty()) {
        return -1;
    }
    SetViewportSize(viewportWidth, viewportHeight);
    RenderForPicking(viewMatrix, projectionMatrix);

    unsigned char data[4];
    glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);
    y = viewportHeight - y;

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int selection = data[0] | (data[1] << 8) | (data[2] << 16);
    if (selection == 0 && (data[0] == 0 && data[1] == 0 && data[2] == 0)) {
        return -1;
    }
    if (m_vertexMode) {
        if (selection >= m_vertexCount) {
            return -1;
        }
    } else {
        if (selection >= m_faceCount) {
            return -1;
        }
    }
    return selection;
}

void ModelRenderer::UpdateColorData()
{
    if (!m_modelData || m_modelData->faces.empty()) {
        return;
    }

    auto const& faces = m_modelData->faces;

    m_colorData.clear();

    for (size_t i = 0; i < faces.size(); ++i) {
        auto const& face = faces[i];
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        uint32_t rgb = 0;

        switch (m_colorMode) {
        case ColorMode::Diffuse:
            rgb = math::RunetekColor::HSLToRGB(face.color);
            break;

        case ColorMode::Priority:
            if (face.priority) {
                rgb = math::RunetekColor::HelperToRGB(*face.priority);
            } else {
                r = 0.7f;
                g = 0.7f;
                b = 0.7f;
                AddVertexColors(r, g, b);
                continue;
            }
            break;

        case ColorMode::Label:
            if (face.label) {
                rgb = math::RunetekColor::HelperToRGB(*face.label);
            } else {
                r = 0.5f;
                g = 0.5f;
                b = 0.5f;
                AddVertexColors(r, g, b);
                continue;
            }
            break;
        }

        r = static_cast<float>(rgb >> 16 & 0xff) / 255.0f;
        g = static_cast<float>(rgb >> 8 & 0xff) / 255.0f;
        b = static_cast<float>(rgb & 0xff) / 255.0f;

        AddVertexColors(r, g, b);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    glBufferData(GL_ARRAY_BUFFER, m_colorData.size() * sizeof(float), m_colorData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
}
