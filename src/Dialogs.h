#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace imp {
void ShowFatalDialog(char const* title, char const* message);
std::optional<std::filesystem::path> OpenFileDialog(std::vector<std::pair<std::string, std::string>> const& filters);
std::optional<std::filesystem::path> SaveFileDialog(std::string const& title, std::string const& defaultPath, std::vector<std::pair<std::string, std::string>> const& filters);
std::optional<std::filesystem::path> OpenDirectoryDialog();
}
