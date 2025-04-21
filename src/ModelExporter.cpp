#include "ModelExporter.h"

#include "Dialogs.h"
#include "Packet.h"
#include "RunetekColor.h"

#include <fstream>
#include <random>
#include <unordered_map>

namespace imp {
static uint32_t PackFaceColorAndAlpha(Face const& face)
{
    return face.color | (face.trans.value_or(0) & 0xff) << 8;
}

static bool WriteBinary(std::filesystem::path const& path, Packet const& packet)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<char const*>(packet.GetData()), packet.GetSize());
    return true;
}

bool ModelExporter::ExportMQO(ModelData const& model, std::filesystem::path const& outputPath)
{
    if (model.vertices.empty() || model.faces.empty()) {
        return false;
    }
    MQOFile mqoFile;

    bool hasFaceLabels = false;
    bool hasFacePriority = false;

    for (auto const& face : model.faces) {
        if (face.label.has_value()) {
            hasFaceLabels = true;
        }
        if (face.priority.has_value()) {
            hasFacePriority = true;
        }
        if (hasFaceLabels && hasFacePriority) {
            break;
        }
    }

    if (hasFaceLabels || hasFacePriority) {
        AddMQOHelperMaterials(mqoFile, 800);
    }

    std::unordered_map<uint32_t, uint32_t> materialLookup;

    MQOObject& geom = mqoFile.AddObject("GEOM");
    AddMQOGeometry(geom, model);
    AddMQOColors(materialLookup, mqoFile, geom, model);

    if (hasFaceLabels) {
        MQOObject& tskin = mqoFile.AddObject("TSKIN");
        AddMQOGeometry(tskin, model);
        for (size_t i = 0; i < model.faces.size(); ++i) {
            if (Face const& face = model.faces[i]; face.label.has_value()) {
                tskin.faces[i].materialIndex = face.label.value();
            }
        }
    }
    if (hasFacePriority) {
        MQOObject& pri = mqoFile.AddObject("PRI");
        AddMQOGeometry(pri, model);
        for (size_t i = 0; i < model.faces.size(); ++i) {
            if (Face const& face = model.faces[i]; face.priority.has_value()) {
                pri.faces[i].materialIndex = face.priority.value();
            }
        }
    }
    return mqoFile.Write(outputPath);
}

void ModelExporter::AddMQOGeometry(MQOObject& mqoObject, ModelData const& model)
{
    mqoObject.vertices.reserve(model.vertices.size());
    for (auto const& vertex : model.vertices) {
        MQOVertex& mqoVertex = mqoObject.vertices.emplace_back();
        mqoVertex.x = vertex.x;
        mqoVertex.y = static_cast<float>(-vertex.y);
        mqoVertex.z = static_cast<float>(-vertex.z);
        if (vertex.label.has_value()) {
            mqoVertex.weit = static_cast<float>(vertex.label.value()) / 1000.0f;
        }
    }

    mqoObject.faces.reserve(model.faces.size());
    for (auto const& face : model.faces) {
        MQOFace& mqoFace = mqoObject.faces.emplace_back();
        mqoFace.v1 = face.v1;
        mqoFace.v2 = face.v2;
        mqoFace.v3 = face.v3;
    }
}

void ModelExporter::AddMQOColors(std::unordered_map<uint32_t, uint32_t>& materials, MQOFile& mqoFile, MQOObject& mqoObject, ModelData const& model)
{
    for (size_t i = 0; i < model.faces.size(); ++i) {
        Face const& face = model.faces[i];
        uint32_t packed = PackFaceColorAndAlpha(face);
        uint16_t color = face.color;
        uint8_t trans = face.trans.value_or(0) & 0xff;

        auto const& findIt = materials.find(color);
        uint32_t index;
        if (findIt == materials.end()) {
            float alpha = (255.0f - static_cast<float>(trans)) / 255.0f;

            uint32_t rgb = math::RunetekColor::HSLToRGB(color);
            float r = static_cast<float>(rgb >> 16 & 0xFF) / 255.0f;
            float g = static_cast<float>(rgb >> 8 & 0xFF) / 255.0f;
            float b = static_cast<float>(rgb & 0xFF) / 255.0f;

            MQOMaterial material;
            material.name = "mat" + std::to_string(color);
            material.r = r;
            material.g = g;
            material.b = b;
            material.alpha = alpha;
            index = mqoFile.AddMaterial(material);
            materials[packed] = index;
        } else {
            index = findIt->second;
        }

        mqoObject.faces[i].materialIndex = static_cast<int>(index);
    }
}

void ModelExporter::AddMQOHelperMaterials(MQOFile& mqoFile, int count)
{
    for (int i = 0; i < count; ++i) {
        uint32_t rgb = math::RunetekColor::HelperToRGB(static_cast<uint16_t>(i));

        MQOMaterial material;
        material.name = std::to_string(i);
        material.r = static_cast<float>(rgb >> 16 & 0xff) / 255.0f;
        material.g = static_cast<float>(rgb >> 8 & 0xff) / 255.0f;
        material.b = static_cast<float>(rgb & 0xff) / 255.0f;

        mqoFile.AddMaterial(material);
    }
}

bool ModelExporter::ExportV1(ModelData const& model, std::filesystem::path const& outputPath)
{
    uint32_t vertexCount = static_cast<uint32_t>(model.vertices.size());
    uint32_t faceCount = static_cast<uint32_t>(model.faces.size());
    uint32_t mappingCount = static_cast<uint32_t>(model.textures.size());

    uint32_t defaultPriority = 0;
    if (!model.faces.empty()) {
        std::optional<uint32_t> firstPriority;
        for (Face const& face : model.faces) {
            if (face.priority.has_value()) {
                if (firstPriority.has_value()) {
                    if (firstPriority.value() != face.priority.value()) {
                        defaultPriority = 255;
                        break;
                    }
                } else {
                    firstPriority = face.priority.value();
                }
            }
        }
    }

    bool hasFacesLabel = false;
    bool hasFaceType = false;
    bool hasFacesTrans = false;
    for (Face const& face : model.faces) {
        hasFacesLabel |= face.label.has_value();

        hasFaceType |= face.type.value_or(0) == 1;
        hasFaceType |= face.material.value_or(-1) != -1;

        hasFacesTrans |= face.trans.has_value();
        if (hasFacesLabel && hasFaceType && hasFacesTrans) {
            break;
        }
    }

    bool hasVerticesLabel = false;
    for (Vertex const& vertex : model.vertices) {
        hasVerticesLabel |= vertex.label.has_value();
        if (hasVerticesLabel) {
            break;
        }
    }

    Packet vertexAxisBlock(vertexCount);
    Packet facesCompressionBlock(faceCount);
    Packet facesPriorityBlock(defaultPriority == 255 ? faceCount : 0);
    Packet facesLabelBlock(hasFacesLabel ? faceCount : 0);
    Packet facesTypeBlock(hasFaceType ? faceCount : 0);
    Packet verticesLabelBlock(hasVerticesLabel ? vertexCount : 0);
    Packet facesTransBlock(hasFacesTrans ? faceCount : 0);
    Packet facesIndexBlock(faceCount * 6);
    Packet facesHslBlock(faceCount * 2);
    Packet mappingsBlock(mappingCount * 6);
    Packet verticesXBlock(vertexCount * 2);
    Packet verticesYBlock(vertexCount * 2);
    Packet verticesZBlock(vertexCount * 2);

    int32_t baseX = 0;
    int32_t baseY = 0;
    int32_t baseZ = 0;
    for (Vertex const& vertex : model.vertices) {
        uint8_t axis = 0;
        int32_t xOffset = vertex.x - baseX;
        if (xOffset != 0) {
            axis |= 0x1;
            verticesXBlock.pSmart1or2s(xOffset);
            baseX = vertex.x;
        }
        int32_t yOffset = vertex.y - baseY;
        if (yOffset != 0) {
            axis |= 0x2;
            verticesYBlock.pSmart1or2s(yOffset);
            baseY = vertex.y;
        }
        int32_t zOffset = vertex.z - baseZ;
        if (zOffset != 0) {
            axis |= 0x4;
            verticesZBlock.pSmart1or2s(zOffset);
            baseZ = vertex.z;
        }
        vertexAxisBlock.p1(axis);
        if (hasVerticesLabel) {
            verticesLabelBlock.p1(vertex.label.value_or(255));
        }
    }
    for (Face const& face : model.faces) {
        int16_t materialId = face.material.value_or(-1);
        if (materialId != -1) {
            facesHslBlock.p2(static_cast<uint16_t>(face.material.value()));
        } else {
            facesHslBlock.p2(face.color);
        }
        if (hasFaceType) {
            int32_t packed = 0;
            if (face.type.value_or(0) == 1) {
                packed |= 0x1;
            }
            if (materialId != -1) {
                packed |= 0x2;
                packed |= (face.mapping.value_or(0) << 2);
            }
            facesTypeBlock.p1(packed);
        }
        if (defaultPriority == 255) {
            facesPriorityBlock.p1(face.priority.value_or(0));
        }
        if (hasFacesTrans) {
            facesTransBlock.p1(face.trans.value_or(0));
        }
        if (hasFacesLabel) {
            facesLabelBlock.p1(face.label.value_or(255));
        }
    }
    int16_t a = -65000;
    int16_t b = -65000;
    int16_t c = -65000;
    int32_t acc = 0;

    for (int32_t i = 0; i < faceCount; i++) {
        Face const& face = model.faces[i];

        uint8_t type;
        if (face.v1 == a && face.v2 == c) {
            type = 2;
        } else if (face.v1 == c && face.v2 == b) {
            type = 3;
        } else if (face.v1 == b && face.v2 == a) {
            type = 4;
        } else {
            type = 1;
        }
        facesCompressionBlock.p1(type);

        switch (type) {
        case 1:
            facesIndexBlock.pSmart1or2s(face.v1 - acc);
            acc = face.v1;
            facesIndexBlock.pSmart1or2s(face.v2 - acc);
            acc = face.v2;
            facesIndexBlock.pSmart1or2s(face.v3 - acc);
            acc = face.v3;
            break;
        case 2:
            facesIndexBlock.pSmart1or2s(face.v3 - acc);
            acc = face.v3;
            break;
        case 3:
            facesIndexBlock.pSmart1or2s(face.v3 - acc);
            acc = face.v3;
            break;
        case 4:
            facesIndexBlock.pSmart1or2s(face.v3 - acc);
            acc = face.v3;
            break;
        default:
            assert(false);
        }
    }
    for (Texture const& texture : model.textures) {
        mappingsBlock.p2(texture.p);
        mappingsBlock.p2(texture.m);
        mappingsBlock.p2(texture.n);
    }
    size_t totalSize = 0;
    totalSize += vertexAxisBlock.GetSize();
    totalSize += facesCompressionBlock.GetSize();
    totalSize += facesPriorityBlock.GetSize();
    totalSize += facesLabelBlock.GetSize();
    totalSize += facesTypeBlock.GetSize();
    totalSize += verticesLabelBlock.GetSize();
    totalSize += facesTransBlock.GetSize();
    totalSize += facesIndexBlock.GetSize();
    totalSize += facesHslBlock.GetSize();
    totalSize += mappingsBlock.GetSize();
    totalSize += verticesXBlock.GetSize();
    totalSize += verticesYBlock.GetSize();
    totalSize += verticesZBlock.GetSize();
    Packet combined(totalSize + 18);
    combined.pArr(vertexAxisBlock.GetData(), vertexAxisBlock.GetSize());
    combined.pArr(facesCompressionBlock.GetData(), facesCompressionBlock.GetSize());
    combined.pArr(facesPriorityBlock.GetData(), facesPriorityBlock.GetSize());
    combined.pArr(facesLabelBlock.GetData(), facesLabelBlock.GetSize());
    combined.pArr(facesTypeBlock.GetData(), facesTypeBlock.GetSize());
    combined.pArr(verticesLabelBlock.GetData(), verticesLabelBlock.GetSize());
    combined.pArr(facesTransBlock.GetData(), facesTransBlock.GetSize());
    combined.pArr(facesIndexBlock.GetData(), facesIndexBlock.GetSize());
    combined.pArr(facesHslBlock.GetData(), facesHslBlock.GetSize());
    combined.pArr(mappingsBlock.GetData(), mappingsBlock.GetSize());
    combined.pArr(verticesXBlock.GetData(), verticesXBlock.GetSize());
    combined.pArr(verticesYBlock.GetData(), verticesYBlock.GetSize());
    combined.pArr(verticesZBlock.GetData(), verticesZBlock.GetSize());
    combined.p2(vertexCount);
    combined.p2(faceCount);
    combined.p1(mappingCount);
    combined.p1(hasFaceType);
    combined.p1(defaultPriority);
    combined.p1(hasFacesTrans);
    combined.p1(hasFacesLabel);
    combined.p1(hasVerticesLabel);
    combined.p2(verticesXBlock.GetSize());
    combined.p2(verticesYBlock.GetSize());
    combined.p2(verticesZBlock.GetSize());
    combined.p2(facesIndexBlock.GetSize());

    return WriteBinary(outputPath, combined);
}
}
