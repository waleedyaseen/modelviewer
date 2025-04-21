#pragma once

#include "MQOFile.h"
#include "Model.h"
#include <filesystem>
#include <unordered_map>

namespace imp {
class ModelExporter {
public:
    static bool ExportMQO(ModelData const& model, std::filesystem::path const& outputPath);
    static bool ExportV1(ModelData const& model, std::filesystem::path const& outputPath);

private:
    static void AddMQOGeometry(MQOObject& mqoObject, ModelData const& model);
    static void AddMQOColors(std::unordered_map<uint32_t, uint32_t>& materials, MQOFile& mqoFile, MQOObject& mqoObject, ModelData const& model);
    static void AddMQOHelperMaterials(MQOFile& mqoFile, int count);
};
}
