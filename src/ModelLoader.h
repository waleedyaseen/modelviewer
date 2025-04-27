#pragma once

#include "MQOFile.h"
#include "Model.h"
#include "Packet.h"
#include <filesystem>

namespace imp {
class ModelLoader {
public:
    static bool LoadFromFile(ModelData& model, std::filesystem::path const& filePath);

private:
    static bool LoadAny(ModelData& model, Packet& packet);
    static bool LoadV1(ModelData& model, Packet& packet);
    static bool LoadV4(ModelData& model, Packet& packet);
    static bool LoadMQO(ModelData& model, std::filesystem::path const& filePath);

    static bool ConvertFromMQO(ModelData& model, MQOFile const& mqoFile);
};
}
