#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace imp {

struct MQOScene {
    float posX { 0.0f };
    float posY { 0.0f };
    float posZ { 1500.0f };
    float lookatX { 0.0f };
    float lookatY { 0.0f };
    float lookatZ { 0.0f };
    float head { 0.2164f };
    float pitch { 0.8036f };
    int32_t ortho { 0 };
    float zoom2 { 1.9531f };
    float ambR { 0.5f };
    float ambG { 0.5f };
    float ambB { 0.5f };
};

struct MQOMaterial {
    std::string name;
    int32_t shader { 3 };
    float r { 1.0f };
    float g { 1.0f };
    float b { 1.0f };
    float alpha { 1.0f };
    float diffuse { 0.8f };
    float ambient { 0.0f };
    float emission { 0.75f };
    float specular { 0.0f };
    float power { 5.0f };
};

struct MQOVertex {
    float x { 0.0f };
    float y { 0.0f };
    float z { 0.0f };
    std::optional<float> weit;
};

struct MQOFace {
    int32_t v1 { 0 };
    int32_t v2 { 0 };
    int32_t v3 { 0 };
    int32_t materialIndex { 0 };
};

struct MQOObject {
    std::string name;
    int32_t depth { 0 };
    int32_t folding { 0 };
    float scaleX { 1.0f };
    float scaleY { 1.0f };
    float scaleZ { 1.0f };
    float rotationX { 0.0f };
    float rotationY { 0.0f };
    float rotationZ { 0.0f };
    float translationX { 0.0f };
    float translationY { 0.0f };
    float translationZ { 0.0f };
    int32_t visible { 15 };
    int32_t locking { 0 };
    int32_t shading { 1 };
    float facet { 120.0f };
    float colorR { 0.0f };
    float colorG { 0.0f };
    float colorB { 0.0f };
    int32_t colorType { 0 };

    std::vector<MQOVertex> vertices;
    std::vector<MQOFace> faces;
};

class MQOFile {
public:
    MQOFile();

    bool Write(std::filesystem::path const& outputPath);

    void SetScene(MQOScene const& scene);

    int32_t AddMaterial(MQOMaterial const& material);
    void SetMaterial(int32_t index, MQOMaterial const& material);
    MQOMaterial const* GetMaterial(int32_t index) const;

    MQOObject& AddObject(std::string const& name);
    int32_t AddObject(MQOObject const& object);
    void SetObject(int32_t index, MQOObject const& object);
    MQOObject const* FindObject(char const* name) const;

    friend class ModelLoader;

private:
    void WriteHeader(std::ofstream& file);
    void WriteScene(std::ofstream& file);
    void WriteMaterials(std::ofstream& file);
    void WriteObjects(std::ofstream& file);
    void WriteObject(std::ofstream& file, MQOObject const& object);

    MQOScene m_scene;
    std::vector<MQOObject> m_objects;
    std::vector<MQOMaterial> m_materials;
};
}
