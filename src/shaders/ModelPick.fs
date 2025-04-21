static auto modelPickFragmentShader = R"(
#version 330 core

flat in int vFaceID;
flat in int vVertexID;

out vec4 FragColor;

uniform int uVertexMode;

void main()
{
    if (uVertexMode == 1) {
        int r = vVertexID & 0xFF;
        int g = (vVertexID >> 8) & 0xFF;
        int b = (vVertexID >> 16) & 0xFF;
        FragColor = vec4(r / 255.0, g / 255.0, b / 255.0, 1.0);
    } else {
        int r = vFaceID & 0xFF;
        int g = (vFaceID >> 8) & 0xFF;
        int b = (vFaceID >> 16) & 0xFF;
        FragColor = vec4(r / 255.0, g / 255.0, b / 255.0, 1.0);
    }
}
)";