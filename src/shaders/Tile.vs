static auto tileVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec2 aVertexUV;

out vec2 vTextureUV;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
    vTextureUV = aVertexUV;
}
)";