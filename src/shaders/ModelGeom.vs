static auto modelGeomVertexShader = R"(
#version 330 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec4 aVertexColor;
layout (location = 2) in float aVertexID;
layout (location = 3) in float aVertexFaceID;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uHighlightVertex;
uniform int uSelectedVertex;
uniform int uHighlightFace;
uniform int uSelectedFace;
uniform vec4 uHighlightColor;
uniform vec4 uSelectedColor;
uniform int uVertexMode;
uniform int uHighlight;

out vec4 vVertexColor;
out float vHighlight;

void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(aVertexPosition, 1.0);
    vVertexColor = aVertexColor;

    // Set highlighting value
    // 0 = no highlight, 1 = hover highlight, 2 = selected highlight
    if (uHighlight == 1) {
        if (uVertexMode == 1) {
            if (aVertexID == uSelectedVertex) {
                vHighlight = 2.0;
            } else if (aVertexID == uHighlightVertex) {
                vHighlight = 1.0;
            } else {
                vHighlight = 0.0;
            }
        } else {
            if (aVertexFaceID == uSelectedFace) {
                vHighlight = 2.0;
            } else if (aVertexFaceID == uHighlightFace) {
                vHighlight = 1.0;
            } else {
                vHighlight = 0.0;
            }
        }
    } else {
        vHighlight = 0.0;
    }
}
)";