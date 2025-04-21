#include "MQOFile.h"

#include <fstream>
#include <iostream>

namespace imp {
MQOFile::MQOFile()
{
    // Do nothing.
}

void MQOFile::SetScene(MQOScene const& scene)
{
    m_scene = scene;
}

int MQOFile::AddMaterial(MQOMaterial const& material)
{
    m_materials.push_back(material);
    return static_cast<int>(m_materials.size() - 1);
}

void MQOFile::SetMaterial(int index, MQOMaterial const& material)
{
    if (index >= 0 && index < m_materials.size()) {
        m_materials[index] = material;
    }
}

MQOMaterial const* MQOFile::GetMaterial(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_materials.size())) {
        return nullptr;
    }
    return &m_materials[index];
}

MQOObject& MQOFile::AddObject(std::string const& name)
{
    MQOObject& object = m_objects.emplace_back();
    object.name = name;
    return object;
}

int MQOFile::AddObject(MQOObject const& object)
{
    m_objects.push_back(object);
    return static_cast<int>(m_objects.size() - 1);
}

void MQOFile::SetObject(int index, MQOObject const& object)
{
    if (index >= 0 && index < m_objects.size()) {
        m_objects[index] = object;
    }
}

MQOObject const* MQOFile::FindObject(char const* name) const
{
    for (auto const& object : m_objects) {
        if (object.name == name) {
            return &object;
        }
    }
    return nullptr;
}

bool MQOFile::Write(std::filesystem::path const& outputPath)
{
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        return false;
    }

    WriteHeader(file);
    WriteScene(file);
    WriteMaterials(file);
    WriteObjects(file);

    file << "Eof" << "\n";
    file.close();
    return true;
}

void MQOFile::WriteHeader(std::ofstream& file)
{
    file << "Metasequoia Document" << "\n";
    file << "Format Text Ver 1.1" << "\n";
    file << "\n";
}

void MQOFile::WriteScene(std::ofstream& file)
{
    file << "Scene {" << "\n";
    file << "\tpos " << m_scene.posX << " " << m_scene.posY << " " << m_scene.posZ << "\n";
    file << "\tlookat " << m_scene.lookatX << " " << m_scene.lookatY << " " << m_scene.lookatZ << "\n";
    file << "\thead " << m_scene.head << "\n";
    file << "\tpich " << m_scene.pitch << "\n";
    file << "\tortho " << m_scene.ortho << "\n";
    file << "\tzoom2 " << m_scene.zoom2 << "\n";
    file << "\tamb " << m_scene.ambR << " " << m_scene.ambG << " " << m_scene.ambB << "\n";
    file << "}" << "\n";
}

void MQOFile::WriteMaterials(std::ofstream& file)
{
    if (m_materials.empty()) {
        return;
    }

    file << "Material " << m_materials.size() << " {" << "\n";

    for (auto const& material : m_materials) {
        file << "\t\"" << material.name << "\" shader(" << material.shader << ") col(";
        file << material.r << " " << material.g << " " << material.b << " " << material.alpha;
        file << ") dif(" << material.diffuse << ") amb(" << material.ambient << ") ";
        file << "emi(" << material.emission << ") spc(" << material.specular << ") ";
        file << "power(" << material.power << ")" << "\n";
    }

    file << "}" << "\n";
}

void MQOFile::WriteObjects(std::ofstream& file)
{
    for (auto const& object : m_objects) {
        WriteObject(file, object);
    }
}

void MQOFile::WriteObject(std::ofstream& file, MQOObject const& object)
{
    file << "Object \"" << object.name << "\" {" << "\n";
    file << "\tdepth " << object.depth << "\n";
    file << "\tfolding " << object.folding << "\n";
    file << "\tscale " << object.scaleX << " " << object.scaleY << " " << object.scaleZ << "\n";
    file << "\trotation " << object.rotationX << " " << object.rotationY << " " << object.rotationZ << "\n";
    file << "\ttranslation " << object.translationX << " " << object.translationY << " " << object.translationZ << "\n";
    file << "\tvisible " << object.visible << "\n";
    file << "\tlocking " << object.locking << "\n";
    file << "\tshading " << object.shading << "\n";
    file << "\tfacet " << object.facet << "\n";
    file << "\tcolor " << object.colorR << " " << object.colorG << " " << object.colorB << "\n";
    file << "\tcolor_type " << object.colorType << "\n";

    if (!object.vertices.empty()) {
        file << "\tvertex " << object.vertices.size() << " {" << "\n";

        for (auto const& vertex : object.vertices) {
            file << "\t\t" << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
        }

        file << "\t}" << "\n";

        bool hasWeights = false;
        for (auto const& vertex : object.vertices) {
            if (vertex.weit.has_value()) {
                hasWeights = true;
                break;
            }
        }

        if (hasWeights) {
            file << "\tvertexattr {" << "\n";
            file << "\t\tweit {" << "\n";
            for (size_t i = 0; i < object.vertices.size(); ++i) {
                MQOVertex const& vertex = object.vertices[i];
                if (vertex.weit.has_value()) {
                    file << "\t\t\t" << i << " " << vertex.weit.value() << "\n";
                }
            }
            file << "\t\t}" << "\n";
            file << "\t}" << "\n";
        }
    }

    if (!object.faces.empty()) {
        file << "\tface " << object.faces.size() << " {" << "\n";
        for (auto const& face : object.faces) {
            file << "\t\t3 V(" << face.v3 << " " << face.v2 << " " << face.v1 << ") M(" << face.materialIndex << ")" << "\n";
        }
        file << "\t}" << "\n";
    }
    file << "}" << "\n";
}
}
