#pragma once

#include "ModelRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

namespace imp {
class Renderer {
public:
    Renderer();
    ~Renderer();

    void Initialize();
    void Destroy();

    void Render(float deltaTime);

    void SetCameraPosition(glm::vec3 const& position);
    void OrbitCamera(float deltaX, float deltaY);
    void ZoomCamera(float amount);
    void ResetCamera();

    void SetWireframeMode(bool enabled);
    void SetFOV(float fov);
    void SetNearPlane(float nearPlane);
    void SetFarPlane(float farPlane);

    int Pick(int x, int y);
    void SetViewportSize(int width, int height);

    glm::mat4 const& GetViewMatrix() const
    {
        return m_viewMatrix;
    }

    glm::mat4 const& GetProjectionMatrix() const
    {
        return m_projectionMatrix;
    }

    uint32_t GetRenderTextureID() const
    {
        return m_textureColorBuffer;
    }

    void SetTileGridSize(int size)
    {
        m_tileGridSize = size;
    }

    void SetCameraTarget(glm::vec3 const& target)
    {
        m_cameraTarget = target;
        m_viewMatrix = glm::lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
    }

    std::shared_ptr<ModelData> const& GetModelData() const
    {
        return m_modelRenderer.GetModelData();
    }

    bool HasModelLoaded() const
    {
        std::shared_ptr<ModelData> modelData = GetModelData();
        return modelData && !modelData->vertices.empty() && !modelData->faces.empty();
    }

    ModelRenderer& GetModelRenderer()
    {
        return m_modelRenderer;
    }

private:
    void SetupShaders();
    void SetupTileGridPlane();
    void SetupFramebuffer();
    void DrawTileGrid();
    void DrawInfiniteGrid();
    void UpdateProjectionMatrix();

    ShaderProgram m_tileShaderProgram;
    ShaderProgram m_gridShaderProgram;
    uint32_t m_planeVAO;
    uint32_t m_planeVBO;
    uint32_t m_planeEBO;
    uint32_t m_gridVAO;
    uint32_t m_gridVBO;

    glm::vec3 m_cameraPosition;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraUp;
    float m_cameraDistance;
    float m_cameraPhi;
    float m_cameraTheta;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    float m_aspectRatio;

    std::vector<float> m_planeVertices;
    std::vector<unsigned int> m_planeIndices;
    std::vector<float> m_gridVertices;

    int m_tileGridSize { 1 };
    float m_gridSize { 1.0f };

    uint32_t m_framebuffer;
    uint32_t m_textureColorBuffer;
    uint32_t m_renderBuffer;

    int m_viewportWidth;
    int m_viewportHeight;

    ModelRenderer m_modelRenderer;
};

}
