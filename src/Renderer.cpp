#include "Renderer.h"

#include "Dialogs.h"

#include <cmath>
#include <glad/glad.h>
#include <imgui.h>

namespace imp {
#include "shaders/InfiniteGrid.fs"
#include "shaders/InfiniteGrid.vs"
#include "shaders/Tile.fs"
#include "shaders/Tile.vs"

constexpr float kDefaultCameraDistance = 500.0f;
constexpr float kDefaultCameraPhi = 0.75f;
constexpr float kDefaultCameraTheta = 1.2f;

Renderer::Renderer()
    : m_planeVAO(0)
    , m_planeVBO(0)
    , m_planeEBO(0)
    , m_gridVAO(0)
    , m_gridVBO(0)
    , m_cameraPosition(0.0f, 3.0f, 3.0f)
    , m_cameraTarget(0.0f, 0.0f, 0.0f)
    , m_cameraUp(0.0f, 1.0f, 0.0f)
    , m_cameraDistance(kDefaultCameraDistance)
    , m_cameraPhi(kDefaultCameraPhi)
    , m_cameraTheta(kDefaultCameraTheta)
    , m_viewMatrix()
    , m_projectionMatrix()
    , m_fov(60.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(3548.0f)
    , m_aspectRatio(16.0f / 9.0f)
    , m_gridSize(10.0f)
    , m_framebuffer(0)
    , m_textureColorBuffer(0)
    , m_renderBuffer(0)
    , m_viewportWidth(0)
    , m_viewportHeight(0)
{
    // Do nothing.
}

Renderer::~Renderer()
{
    Destroy();
}

void Renderer::Initialize()
{
    SetupShaders();
    SetupTileGridPlane();
    SetupFramebuffer();
    ResetCamera();

    constexpr float gridPlaneSize = 10000.0f;
    m_gridVertices = {
        -gridPlaneSize, 0.0f, -gridPlaneSize,
        -gridPlaneSize, 0.0f, gridPlaneSize,
        gridPlaneSize, 0.0f, gridPlaneSize,
        gridPlaneSize, 0.0f, -gridPlaneSize
    };

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(m_gridVertices.size() * sizeof(float)), m_gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_viewportWidth = 1;
    m_viewportHeight = 1;
    m_aspectRatio = 1.0f;
    UpdateProjectionMatrix();

    m_modelRenderer.Initialize();
}

void Renderer::Destroy()
{
    if (m_planeVAO != 0) {
        glDeleteVertexArrays(1, &m_planeVAO);
        m_planeVAO = 0;
    }
    if (m_planeVBO != 0) {
        glDeleteBuffers(1, &m_planeVBO);
        m_planeVBO = 0;
    }
    if (m_planeEBO != 0) {
        glDeleteBuffers(1, &m_planeEBO);
        m_planeEBO = 0;
    }
    if (m_gridVAO != 0) {
        glDeleteVertexArrays(1, &m_gridVAO);
        m_gridVAO = 0;
    }
    if (m_gridVBO != 0) {
        glDeleteBuffers(1, &m_gridVBO);
        m_gridVBO = 0;
    }
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    if (m_textureColorBuffer != 0) {
        glDeleteTextures(1, &m_textureColorBuffer);
        m_textureColorBuffer = 0;
    }
    if (m_renderBuffer != 0) {
        glDeleteRenderbuffers(1, &m_renderBuffer);
        m_renderBuffer = 0;
    }
}

void Renderer::SetupShaders()
{
    m_gridShaderProgram.Create(gridVertexShaderSource, gridFragmentShaderSource);
    m_tileShaderProgram.Create(tileVertexShaderSource, tileFragmentShaderSource);
}

void Renderer::SetupTileGridPlane()
{
    // tile size is 128 in low revision or 512 in high revision
    constexpr float planeSize = 128.0f;
    constexpr float halfSize = planeSize / 2.0f;

    m_planeVertices = {
        -halfSize, 0.0f, -halfSize, 0.0f, 0.0f,
        -halfSize, 0.0f, halfSize, 0.0f, 1.0f,
        halfSize, 0.0f, halfSize, 1.0f, 1.0f,
        halfSize, 0.0f, -halfSize, 1.0f, 0.0f
    };

    m_planeIndices = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &m_planeVAO);
    glGenBuffers(1, &m_planeVBO);
    glGenBuffers(1, &m_planeEBO);

    glBindVertexArray(m_planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(m_planeVertices.size() * sizeof(float)), m_planeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(m_planeIndices.size() * sizeof(uint32_t)), m_planeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::SetupFramebuffer()
{
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glGenTextures(1, &m_textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);

    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ShowFatalDialog("Error", "Framebuffer is not complete");
        std::exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetViewportSize(int width, int height)
{
    if (width == m_viewportWidth && height == m_viewportHeight) {
        return;
    }

    m_viewportWidth = width;
    m_viewportHeight = height;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    UpdateProjectionMatrix();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glDeleteTextures(1, &m_textureColorBuffer);

    glGenTextures(1, &m_textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);

    glDeleteRenderbuffers(1, &m_renderBuffer);

    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ShowFatalDialog("Error", "Framebuffer is not complete");
        std::exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_modelRenderer.SetViewportSize(width, height);
}

void Renderer::Render(float deltaTime)
{
    (void)deltaTime;

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    ImVec4 const& color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_cameraPosition.x = m_cameraTarget.x + m_cameraDistance * std::sin(m_cameraTheta) * std::cos(m_cameraPhi);
    m_cameraPosition.y = m_cameraTarget.y + m_cameraDistance * std::cos(m_cameraTheta);
    m_cameraPosition.z = m_cameraTarget.z + m_cameraDistance * std::sin(m_cameraTheta) * std::sin(m_cameraPhi);

    m_viewMatrix = glm::lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);

    DrawInfiniteGrid();
    DrawTileGrid();

    m_modelRenderer.Render(m_viewMatrix, m_projectionMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawTileGrid()
{
    if (m_tileGridSize < 1) {
        return;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    m_tileShaderProgram.Bind();

    m_tileShaderProgram.SetUniform("uViewMatrix", m_viewMatrix);
    m_tileShaderProgram.SetUniform("uProjectionMatrix", m_projectionMatrix);

    glBindVertexArray(m_planeVAO);
    float gridWidth = static_cast<float>(m_tileGridSize) * 128.0f;
    float gridHeight = static_cast<float>(m_tileGridSize) * 128.0f;
    float centerX = gridWidth / 2.0f;
    float centerZ = gridHeight / 2.0f;

    for (int i = 0; i < m_tileGridSize * m_tileGridSize; ++i) {
        int row = i / m_tileGridSize;
        int col = i % m_tileGridSize;

        float xOffset = (static_cast<float>(col) * 128.0f) + 64.0f;
        float zOffset = (static_cast<float>(row) * 128.0f) + 64.0f;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset - centerX, 0.0f, zOffset - centerZ));
        m_tileShaderProgram.SetUniform("uModelMatrix", model);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_planeIndices.size()), GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

void Renderer::DrawInfiniteGrid()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    m_gridShaderProgram.Bind();
    m_gridShaderProgram.SetUniform("uViewMatrix", m_viewMatrix);
    m_gridShaderProgram.SetUniform("uProjectionMatrix", m_projectionMatrix);
    m_gridShaderProgram.SetUniform("uCameraPosition", m_cameraPosition);
    m_gridShaderProgram.SetUniform("uGridSize", m_gridSize);
    m_gridShaderProgram.SetUniform("uFarPlane", m_farPlane);
    m_gridShaderProgram.SetUniform("uCameraZoom", m_cameraDistance);

    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

int Renderer::Pick(int x, int y)
{
    return m_modelRenderer.Pick(x, y, m_viewportWidth, m_viewportHeight, m_viewMatrix, m_projectionMatrix);
}

void Renderer::SetCameraPosition(glm::vec3 const& position)
{
    m_cameraPosition = position;
    m_viewMatrix = glm::lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
}

void Renderer::OrbitCamera(float deltaX, float deltaY)
{
    m_cameraPhi += deltaX * 0.01f;
    m_cameraTheta += deltaY * 0.01f;
    m_cameraTheta = glm::clamp(m_cameraTheta, 0.1f, 3.0f);
}

void Renderer::PanCamera(float deltaX, float deltaY)
{
    glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPosition);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_cameraUp));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    float panSpeed = m_cameraDistance * 0.002f;

    m_cameraTarget += (-right * deltaX * panSpeed) + (up * -deltaY * panSpeed);
}

void Renderer::ZoomCamera(float amount)
{
    m_cameraDistance += amount;
    m_cameraDistance = glm::clamp(m_cameraDistance, 1.0f, 1000.0f);
}

void Renderer::ResetCamera()
{
    m_cameraDistance = kDefaultCameraDistance;
    m_cameraPhi = kDefaultCameraPhi;
    m_cameraTheta = kDefaultCameraTheta;
    m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    UpdateProjectionMatrix();
}

void Renderer::SetWireframeMode(bool enabled)
{
    m_modelRenderer.SetWireframeMode(enabled);
}

void Renderer::SetFOV(float fov)
{
    m_fov = fov;
    UpdateProjectionMatrix();
}

void Renderer::SetNearPlane(float nearPlane)
{
    m_nearPlane = nearPlane;
    UpdateProjectionMatrix();
}

void Renderer::SetFarPlane(float farPlane)
{
    m_farPlane = farPlane;
    UpdateProjectionMatrix();
}

void Renderer::UpdateProjectionMatrix()
{
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}
}
