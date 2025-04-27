#include "ModelLoader.h"

#include "Dialogs.h"
#include "MQOFile.h"
#include "MQOParser.h"
#include "RunetekColor.h"
#include "UI.h"
#include <fstream>
#include <regex>
#include <string>

namespace imp {
bool ModelLoader::LoadAny(ModelData& model, Packet& packet)
{
    if (packet.GetSize() < 2) {
        return false;
    }
    int8_t sig1 = packet.GetData()[packet.GetSize() - 1];
    int8_t sig2 = packet.GetData()[packet.GetSize() - 2];
    if (sig1 == -3 && sig2 == -1) {
        return LoadOldschoolV3(model, packet);
    } else if (sig1 == -1 && sig2 == -1) {
        return false;
    } else {
        return LoadV1(model, packet);
    }
}

bool ModelLoader::LoadOldschoolV3(ModelData& model, Packet& packet)
{
    if (packet.GetSize() < 26) {
        return false;
    }
    Packet first = packet.View();
    Packet second = packet.View();
    Packet third = packet.View();
    Packet fourth = packet.View();
    Packet fifth = packet.View();
    Packet sixth = packet.View();
    Packet seventh = packet.View();

    first.SetPos(first.GetSize() - 26);

    int32_t vertexCount = first.g2();
    int32_t faceCount = first.g2();
    int32_t mappingCount = first.g1();
    int32_t hasFacesType = first.g1();
    int32_t defaultPriority = first.g1();
    int32_t hasFacesTrans = first.g1();
    int32_t hasFacesSkins = first.g1();
    int32_t hasFacesMaterial = first.g1();
    int32_t hasVertexLabels = first.g1();
    int32_t hasVertexBones = first.g1();
    int32_t verticesXBlockSize = first.g2();
    int32_t verticesYBlockSize = first.g2();
    int32_t verticesZBlockSize = first.g2();
    int32_t facesIndexBlockSize = first.g2();
    int32_t faceMappingBlockSize = first.g2();
    int32_t vertexLabelBlockSize = first.g2();

    int32_t planarMappingCount = 0;
    int32_t roundMappingCount = 0;
    int32_t cuboidMappingCount = 0;

    model.textures.resize(mappingCount);
    if (mappingCount > 0) {
        first.SetPos(0);
        for (int32_t pos = 0; pos < mappingCount; ++pos) {
            Texture& texture = model.textures[pos];
            int8_t type = texture.type = first.g1s();
            if (type == 0) {
                ++planarMappingCount;
            }
            if (type >= 1 && type <= 3) {
                ++roundMappingCount;
            }
            if (type == 2) {
                ++cuboidMappingCount;
            }
        }
    }

    int32_t pos = 0;
    pos += mappingCount;

    int32_t vertexAxisBlockPos = pos;
    pos += vertexCount;

    int32_t facesTypeBlockPos = pos;
    if (hasFacesType == 1) {
        pos += faceCount;
    }

    int32_t facesCompressionBlockPos = pos;
    pos += faceCount;

    int32_t facesPriorityBlockPos = pos;
    if (defaultPriority == 255) {
        pos += faceCount;
    }

    int32_t facesLabelBlockPos = pos;
    if (hasFacesSkins == 1) {
        pos += faceCount;
    }

    int32_t vertexLabelBlockPos = pos;
    pos += vertexLabelBlockSize;

    int32_t facesTransBlockPos = pos;
    if (hasFacesTrans == 1) {
        pos += faceCount;
    }

    int32_t facesIndexBlockPos = pos;
    pos += facesIndexBlockSize;

    int32_t facesMaterialBlockPos = pos;
    if (hasFacesMaterial == 1) {
        pos += faceCount * 2;
    }

    int32_t facesMappingBlockPos = pos;
    pos += faceMappingBlockSize;

    int32_t facesHslBlockPos = pos;
    pos += faceCount * 2;

    int32_t verticesXBlockPos = pos;
    pos += verticesXBlockSize;

    int32_t verticesYBlockPos = pos;
    pos += verticesYBlockSize;

    int32_t verticesZBlockPos = pos;
    pos += verticesZBlockSize;

    int32_t mappingsPlanarPMNBlockPos = pos;
    pos += planarMappingCount * 6;

    int32_t mappingsPMNBlockPos = pos;
    pos += roundMappingCount * 6;

    int32_t mappingsScaleBlockPos = pos;
    pos += roundMappingCount * 6;

    int32_t mappingsRotationBlockPos = pos;
    pos += roundMappingCount * 2;

    int32_t mappingsDirectionBlockPos = pos;
    pos += roundMappingCount;

    int32_t mappingsTranslateBlockPos = pos;
    pos = pos + roundMappingCount * 2 + cuboidMappingCount * 2;

    model.vertices.resize(vertexCount);
    model.faces.resize(faceCount);

    first.SetPos(vertexAxisBlockPos);
    second.SetPos(verticesXBlockPos);
    third.SetPos(verticesYBlockPos);
    fourth.SetPos(verticesZBlockPos);
    fifth.SetPos(vertexLabelBlockPos);

    int32_t baseX = 0;
    int32_t baseY = 0;
    int32_t baseZ = 0;

    for (int32_t i = 0; i < vertexCount; ++i) {
        int32_t axis = first.g1();
        int32_t xOffset = 0;
        if ((axis & 0x1) != 0) {
            xOffset = second.gSmart1or2s();
        }
        int32_t yOffset = 0;
        if ((axis & 0x2) != 0) {
            yOffset = third.gSmart1or2s();
        }
        int32_t zOffset = 0;
        if ((axis & 0x4) != 0) {
            zOffset = fourth.gSmart1or2s();
        }

        Vertex& vertex = model.vertices[i];
        vertex.x = static_cast<int16_t>(baseX + xOffset);
        vertex.y = static_cast<int16_t>(baseY + yOffset);
        vertex.z = static_cast<int16_t>(baseZ + zOffset);
        baseX = vertex.x;
        baseY = vertex.y;
        baseZ = vertex.z;
        if (hasVertexLabels == 1) {
            if (uint8_t label = fifth.g1(); label != 255) {
                vertex.label = label;
            }
        }
    }
    if (hasVertexBones == 1) {
        for (int32_t i = 0; i < vertexCount; ++i) {
            uint8_t count = fifth.g1();
            for (int32_t j = 0; j < count; ++j) {
                fifth.g1();
                fifth.g1();
            }
        }
    }

    first.SetPos(facesHslBlockPos);
    second.SetPos(facesTypeBlockPos);
    third.SetPos(facesPriorityBlockPos);
    fourth.SetPos(facesTransBlockPos);
    fifth.SetPos(facesLabelBlockPos);
    sixth.SetPos(facesMaterialBlockPos);
    seventh.SetPos(facesMappingBlockPos);

    for (int32_t i = 0; i < faceCount; ++i) {
        Face& face = model.faces[i];
        face.color = first.g2s();
        if (hasFacesType == 1) {
            face.type = second.g1();
        }
        if (defaultPriority == 255) {
            face.priority = third.g1();
        }
        if (hasFacesTrans == 1) {
            face.trans = fourth.g1();
        }
        if (hasFacesSkins == 1) {
            face.label = fifth.g1();
        }
        if (hasFacesMaterial == 1) {
            face.material = (sixth.g2() - 1);
        }
        if (face.material != -1) {
            face.mapping = seventh.g1() - 1;
        }
    }

    first.SetPos(facesIndexBlockPos);
    second.SetPos(facesCompressionBlockPos);

    int16_t a = 0;
    int16_t b = 0;
    int16_t c = 0;
    int32_t acc = 0;
    for (int32_t i = 0; i < faceCount; ++i) {
        Face& face = model.faces[i];
        uint8_t type = second.g1();
        if (type == 1) {
            a = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = a;
            b = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = b;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 2) {
            b = c;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 3) {
            a = c;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 4) {
            int16_t tmp = a;
            a = b;
            b = tmp;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
    }
    first.SetPos(mappingsPlanarPMNBlockPos);
    second.SetPos(mappingsPMNBlockPos);
    third.SetPos(mappingsScaleBlockPos);
    fourth.SetPos(mappingsRotationBlockPos);
    fifth.SetPos(mappingsDirectionBlockPos);
    sixth.SetPos(mappingsTranslateBlockPos);

    for (int32_t i = 0; i < mappingCount; ++i) {
        Texture& texture = model.textures[i];
        if (texture.type == 0) {
            texture.p = first.g2();
            texture.m = first.g2();
            texture.n = first.g2();
        }
    }

    first.SetPos(pos);
    int32_t lsCount = first.g1();
    if (lsCount != 0) {
        first.g2();
        first.g2();
        first.g2();
        first.g4();
    }

    return true;
}

bool ModelLoader::LoadV1(ModelData& model, Packet& packet)
{
    if (packet.GetSize() < 18) {
        return false;
    }
    Packet first = packet.View();
    Packet second = packet.View();
    Packet third = packet.View();
    Packet fourth = packet.View();
    Packet fifth = packet.View();

    first.SetPos(first.GetSize() - 18);

    int32_t vertexCount = first.g2();
    int32_t faceCount = first.g2();
    int32_t mappingCount = first.g1();
    int32_t hasLighting = first.g1();
    int32_t defaultPriority = first.g1();
    int32_t hasFacesTrans = first.g1();
    int32_t hasFacesSkins = first.g1();
    int32_t hasVertexLabels = first.g1();
    int32_t verticesXBlockSize = first.g2();
    int32_t verticesYBlockSize = first.g2();
    int32_t verticesZBlockSize = first.g2();
    int32_t facesIndexBlockSize = first.g2();

    int32_t pos = 0;

    int32_t vertexAxisBlockPos = pos;
    pos += vertexCount;

    int32_t facesCompressionBlockPos = pos;
    pos += faceCount;

    int32_t facesPriorityBlockPos = pos;
    if (defaultPriority == 255) {
        pos += faceCount;
    }

    int32_t facesLabelBlockPos = pos;
    if (hasFacesSkins == 1) {
        pos += faceCount;
    }

    int32_t facesTypeBlockPos = pos;
    if (hasLighting == 1) {
        pos += faceCount;
    }

    int32_t verticesLabelBlockPos = pos;
    if (hasVertexLabels == 1) {
        pos += vertexCount;
    }

    int32_t facesTransBlockPos = pos;
    if (hasFacesTrans == 1) {
        pos += faceCount;
    }

    int32_t facesIndexBlockPos = pos;
    pos += facesIndexBlockSize;

    int32_t facesHslBlockPos = pos;
    pos += faceCount * 2;

    int32_t mappingsBlockPos = pos;
    pos += mappingCount * 6;

    int32_t verticesXBlockPos = pos;
    pos += verticesXBlockSize;

    int32_t verticesYBlockPos = pos;
    pos += verticesYBlockSize;

    int32_t verticesZBlockPos = pos;
    pos += verticesZBlockSize;

    model.vertices.resize(vertexCount);
    model.faces.resize(faceCount);
    model.textures.resize(mappingCount);

    first.SetPos(vertexAxisBlockPos);
    second.SetPos(verticesXBlockPos);
    third.SetPos(verticesYBlockPos);
    fourth.SetPos(verticesZBlockPos);
    fifth.SetPos(verticesLabelBlockPos);

    int32_t baseX = 0;
    int32_t baseY = 0;
    int32_t baseZ = 0;
    for (int32_t i = 0; i < vertexCount; i++) {
        int32_t axis = first.g1();
        int32_t xOffset = 0;
        if ((axis & 0x1) != 0) {
            xOffset = second.gSmart1or2s();
        }
        int32_t yOffset = 0;
        if ((axis & 0x2) != 0) {
            yOffset = third.gSmart1or2s();
        }
        int32_t zOffset = 0;
        if ((axis & 0x4) != 0) {
            zOffset = fourth.gSmart1or2s();
        }

        Vertex& vertex = model.vertices[i];
        vertex.x = static_cast<int16_t>(baseX + xOffset);
        vertex.y = static_cast<int16_t>(baseY + yOffset);
        vertex.z = static_cast<int16_t>(baseZ + zOffset);
        baseX = vertex.x;
        baseY = vertex.y;
        baseZ = vertex.z;
        if (hasVertexLabels == 1) {
            if (uint8_t label = fifth.g1(); label != 255) {
                vertex.label = label;
            }
        }
    }

    first.SetPos(facesHslBlockPos);
    second.SetPos(facesTypeBlockPos);
    third.SetPos(facesPriorityBlockPos);
    fourth.SetPos(facesTransBlockPos);
    fifth.SetPos(facesLabelBlockPos);

    for (int32_t i = 0; i < faceCount; i++) {
        Face& face = model.faces[i];
        face.color = first.g2s();
        if (hasLighting == 1) {
            int32_t packed = second.g1();
            if ((packed & 0x1) == 1) {
                face.type = 1;
            } else {
                face.type = 0;
            }
            if ((packed & 0x2) == 2) {
                face.mapping = packed >> 2;
                face.material = face.color;
                face.color = 127;
                if (face.material == -1) {
                    face.material.reset();
                }
            }
        }
        if (defaultPriority == 255) {
            face.priority = third.g1s();
        }
        if (hasFacesTrans == 1) {
            face.trans = fourth.g1s();
        }
        if (hasFacesSkins == 1) {
            face.label = fifth.g1();
        }
    }

    first.SetPos(facesIndexBlockPos);
    second.SetPos(facesCompressionBlockPos);
    int16_t a = 0;
    int16_t b = 0;
    int16_t c = 0;
    int32_t acc = 0;
    for (int32_t i = 0; i < faceCount; i++) {
        Face& face = model.faces[i];
        uint8_t type = second.g1();
        if (type == 1) {
            a = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = a;
            b = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = b;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 2) {
            b = c;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 3) {
            a = c;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
        if (type == 4) {
            int16_t tmp = a;
            a = b;
            b = tmp;
            c = static_cast<int16_t>(first.gSmart1or2s() + acc);
            acc = c;
            face.v1 = a;
            face.v2 = b;
            face.v3 = c;
        }
    }

    first.SetPos(mappingsBlockPos);
    model.textures.resize(mappingCount);
    for (int texture = 0; texture < mappingCount; texture++) {
        Texture& textureData = model.textures[texture];
        textureData.type = 0;
        textureData.p = first.g2();
        textureData.m = first.g2();
        textureData.n = first.g2();
    }

    return true;
}

bool ModelLoader::LoadFromFile(ModelData& model, std::filesystem::path const& filePath)
{
    if (!std::filesystem::exists(filePath)) {
        UI::ShowAlert("Error", "File does not exist: " + filePath.string(), AlertType::Error);
        return false;
    }

    std::string extension = filePath.extension().string();
    for (auto& c : extension) {
        c = std::tolower(c);
    }

    if (extension == ".mqo") {
        return LoadMQO(model, filePath);
    } else {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            UI::ShowErrorAlert("Error", "Failed to open file: " + filePath.string());
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        Packet packet(fileSize);
        file.read(reinterpret_cast<char*>(packet.GetData()), fileSize);
        file.close();

        return LoadAny(model, packet);
    }
}

bool ModelLoader::LoadMQO(ModelData& model, std::filesystem::path const& filePath)
{
    if (!std::filesystem::exists(filePath)) {
        UI::ShowErrorAlert("Error", "MQO file does not exist: " + filePath.string());
        return false;
    }

    MQOParser parser(filePath);
    if (!parser.Good()) {
        UI::ShowErrorAlert("Error", "Failed to open MQO file: " + filePath.string());
        return false;
    }

    MQOFile mqoFile;
    if (!parser.Parse(mqoFile)) {
        UI::ShowErrorAlert("Error", parser.GetErrorMessage() + ": " + filePath.string());
        return false;
    }

    return ConvertFromMQO(model, mqoFile);
}

bool ModelLoader::ConvertFromMQO(ModelData& model, MQOFile const& mqoFile)
{
    if (mqoFile.m_objects.empty()) {
        UI::ShowErrorAlert("Invalid MQO", "No valid objects found in MQO file.");
        return false;
    }

    model.vertices.clear();
    model.faces.clear();
    model.textures.clear();

    static char const* mainObjectNames[] { "GEOM", "Model", "obj1" }; // for backward compatability
    MQOObject const* mainObject = nullptr;
    for (char const* name : mainObjectNames) {
        mainObject = mqoFile.FindObject(name);
        if (mainObject != nullptr) {
            break;
        }
    }
    if (mainObject == nullptr) {
        UI::ShowErrorAlert("Invalid MQO", "No valid GEOM object found in MQO file.");
        return false;
    }

    MQOObject const* tskinObject = mqoFile.FindObject("TSKIN");
    if (tskinObject == nullptr) {
        tskinObject = mqoFile.FindObject("TSKIN:");
    }

    MQOObject const* priObject = mqoFile.FindObject("PRI");
    if (priObject == nullptr) {
        priObject = mqoFile.FindObject("PRI:");
    }
    model.vertices.reserve(mainObject->vertices.size());

    for (MQOVertex const& mqoVertex : mainObject->vertices) {
        Vertex vertex;
        vertex.x = static_cast<int16_t>(mqoVertex.x);
        vertex.y = static_cast<int16_t>(-mqoVertex.y);
        vertex.z = static_cast<int16_t>(-mqoVertex.z);

        if (mqoVertex.weit.has_value()) {
            vertex.label = static_cast<uint8_t>(mqoVertex.weit.value() * 1000.0f);
        }

        model.vertices.push_back(vertex);
    }

    static char const* vskinNames[] { "VSKIN1:", "VSKIN2:", "VSKIN3:" };
    for (char const* name : vskinNames) {
        MQOObject const* vskinObject = mqoFile.FindObject(name);
        if (vskinObject == nullptr) {
            continue;
        }
        for (size_t i = 0; i < vskinObject->vertices.size(); ++i) {
            MQOVertex const& mqoVertex = vskinObject->vertices[i];
            Vertex& vertex = model.vertices[i];
            if (mqoVertex.weit.has_value()) {
                uint8_t labelOff = static_cast<uint8_t>(mqoVertex.weit.value() * 100.0f);
                vertex.label = vertex.label.value_or(0) + labelOff;
            }
        }
    }

    model.faces.reserve(mainObject->faces.size());
    for (MQOFace const& mqoFace : mainObject->faces) {
        Face face;
        face.v1 = mqoFace.v1;
        face.v2 = mqoFace.v2;
        face.v3 = mqoFace.v3;

        if (MQOMaterial const* material = mqoFile.GetMaterial(mqoFace.materialIndex)) {
            uint8_t r = static_cast<uint8_t>(material->r * 255.0f + 0.5f);
            uint8_t g = static_cast<uint8_t>(material->g * 255.0f + 0.5f);
            uint8_t b = static_cast<uint8_t>(material->b * 255.0f + 0.5f);
            uint8_t alpha = static_cast<uint8_t>(material->alpha * 255.0f + 0.5f);
            uint8_t trans = static_cast<uint8_t>(255 - alpha);

            face.color = math::RunetekColor::RGBToHSL(r << 16 | g << 8 | b);
            if (trans != 0) {
                face.trans = trans;
            }
        }

        model.faces.push_back(face);
    }
    if (tskinObject) {
        for (size_t i = 0; i < tskinObject->faces.size(); ++i) {
            int materialIndex = tskinObject->faces[i].materialIndex;
            if (materialIndex >= 0) {
                model.faces[i].label = materialIndex;
            }
        }
    }
    if (priObject) {
        for (size_t i = 0; i < priObject->faces.size(); ++i) {
            int materialIndex = priObject->faces[i].materialIndex;
            if (materialIndex >= 0) {
                model.faces[i].priority = materialIndex;
            }
        }
    }
    return true;
}
}