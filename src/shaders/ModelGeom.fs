static auto modelGeomFragmentShader = R"(
#version 330 core

in vec4 vVertexColor;
in float vHighlight;
in float vVertexIndex;

uniform vec4 uHighlightColor;
uniform vec4 uSelectedColor;
uniform int uHighlightVertex;
uniform int uSelectedVertex;

out vec4 FragColor;

uniform bool uOverrideColorEnabled;
uniform vec4 uOverrideColor;

void main()
{
    if (vHighlight > 1.5) {
        FragColor = mix(vVertexColor, uSelectedColor, 0.5);
    } else if (vHighlight > 0.5) {
        FragColor = mix(vVertexColor, uHighlightColor, 0.5);
    } else {
        if (uOverrideColorEnabled) {
            FragColor = uOverrideColor;
        } else {
            FragColor = vVertexColor;
        }
    }
}
)";