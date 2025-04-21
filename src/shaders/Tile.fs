static auto tileFragmentShaderSource = R"(
#version 330 core

out vec4 fColor;

in vec2 vTextureUV;

void main()
{
    float cellSize = 2.0;
    float x = floor(vTextureUV.s * cellSize);
    float y = floor(vTextureUV.t * cellSize);
    float checker = mod(x + y, 2.0);

    vec3 color = checker < 0.5 ? vec3(0.3, 0.3, 0.3) : vec3(0.8, 0.8, 0.8);
    fColor = vec4(color, 1.0);
}
)";