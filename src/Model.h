#pragma once

#include <optional>
#include <vector>

namespace imp {
struct Vertex {
    int16_t x;
    int16_t y;
    int16_t z;
    std::optional<uint8_t> label;
};

struct Face {
    uint16_t v1;
    uint16_t v2;
    uint16_t v3;
    uint16_t color;
    std::optional<uint8_t> type;
    std::optional<int8_t> priority;
    std::optional<int8_t> trans;
    std::optional<uint8_t> label;
    std::optional<int16_t> material;
    std::optional<uint8_t> mapping;
};

struct Texture {
    uint8_t type;
    uint16_t p;
    uint16_t m;
    uint16_t n;
};

struct ModelData {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Texture> textures;

    std::optional<uint8_t> priority;

    Vertex const* GetVertex(int index) const
    {
        if (index < 0 || index >= vertices.size()) {
            return nullptr;
        }
        return &vertices[index];
    }

    Face const* GetFace(int index) const
    {
        if (index < 0 || index >= faces.size()) {
            return nullptr;
        }
        return &faces[index];
    }
};
}
