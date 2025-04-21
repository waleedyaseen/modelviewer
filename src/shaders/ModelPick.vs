static auto modelPickVertexShader = R"(
#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec4 aVertexColor;
layout (location = 2) in float aVertexID;
layout (location = 3) in float aVertexFaceID;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

flat out int vFaceID;
flat out int vVertexID;

void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(aVertexPosition, 1.0);
    vFaceID = int(aVertexFaceID);
    vVertexID = int(aVertexID);
}
)";