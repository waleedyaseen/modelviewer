static auto gridFragmentShaderSource = R"(
#version 330 core

uniform vec3 uCameraPosition;
uniform float uGridSize;
uniform float uFarPlane;
uniform float uCameraZoom;

in vec3 vWorldPosition;
out vec4 fColor;

float getGrid(vec3 position, float size)
{
    vec2 s = position.xz / size;
    vec2 t = abs(fract(s - 0.5) - 0.5) / fwidth(s);
    return 1.0 - min(min(t.x, t.y), 1.0);
}

float getAxisHighlight(float coord)
{
    float width = 0.015 * (1.0 + log2(max(1.0, uCameraZoom)) * 0.05);
    float n = abs(coord) / fwidth(coord);
    return 1.0 - min(1.0, n / (width * 100.0));
}

void main()
{
    const vec3 kGridColor = vec3(0.9, 0.9, 1.0);

    float dist = distance(vWorldPosition, uCameraPosition);
    float fadeDistance = uFarPlane * 0.7;

    float zoomFactor = log2(max(1.0, uCameraZoom));
    float zoomScale = max(0.5, zoomFactor * 0.1);

    float scaledGridSize = uGridSize * zoomScale;

    float smallGrid = getGrid(vWorldPosition, scaledGridSize) * 0.2;
    float mediumGrid = getGrid(vWorldPosition, scaledGridSize * 5.0) * 0.5;
    float largeGrid = getGrid(vWorldPosition, scaledGridSize * 25.0) * 0.8;

    float nearWeight = 1.0 - smoothstep(50.0, 200.0, uCameraZoom);
    float midWeight = smoothstep(50.0, 200.0, uCameraZoom) * (1.0 - smoothstep(200.0, 500.0, uCameraZoom));
    float farWeight = smoothstep(200.0, 500.0, uCameraZoom);

    float gridTotal = smallGrid * nearWeight + mediumGrid * midWeight + largeGrid * farWeight;

    float fade = 1.0 - smoothstep(fadeDistance * 0.3, fadeDistance, dist);
    float proximity = 1.0 - smoothstep(0.0, 50.0, dist);

    float xAxisHighlight = getAxisHighlight(vWorldPosition.z);
    float zAxisHighlight = getAxisHighlight(vWorldPosition.x);

    const vec3 xAxisColor = vec3(1.0, 0.0, 0.0);
    const vec3 zAxisColor = vec3(0.0, 0.0, 1.0);

    vec3 lineColor = kGridColor;
    if (xAxisHighlight > 0.0) {
        lineColor = mix(kGridColor, xAxisColor, xAxisHighlight);
    }
    if (zAxisHighlight > 0.0 && xAxisHighlight <= zAxisHighlight) {
        lineColor = mix(kGridColor, zAxisColor, zAxisHighlight);
    }

    float alpha = clamp(gridTotal * (fade + proximity * 0.2), 0.0, 1.0);

    float axisHighlight = max(xAxisHighlight, zAxisHighlight);
    alpha = mix(alpha, min(1.0, alpha * 3.0), axisHighlight);

    fColor = vec4(lineColor, alpha);
    if (fColor.a < 0.01) {
        discard;
    }
}
)";