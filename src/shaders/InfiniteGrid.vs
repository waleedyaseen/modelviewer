static auto gridVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aVertexPosition;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vWorldPosition;

void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(aVertexPosition, 1.0);
    vWorldPosition = aVertexPosition;
}
)";