#include "Dialogs.h"
#include <nfd.hpp>
#ifdef IMP_PLATFORM_WINDOWS
#    include <Windows.h>
#else
#    include <cstdio>
#endif
namespace imp {

void ShowFatalDialog(char const* title, char const* message)
{
#ifdef IMP_PLATFORM_WINDOWS
    MessageBoxA(nullptr, message, title, MB_ICONERROR | MB_OK);
#else
    fprintf(stderr, "%s: %s\n", title, message);
#endif
}

std::optional<std::filesystem::path> OpenFileDialog(std::vector<std::pair<std::string, std::string>> const& filters)
{
    NFD::Guard nfdGuard;

    nfdfilteritem_t* nfdFilters = nullptr;
    size_t filterCount = 0;

    if (!filters.empty()) {
        nfdFilters = new nfdfilteritem_t[filters.size()];
        filterCount = filters.size();

        for (size_t i = 0; i < filters.size(); i++) {
            nfdFilters[i].name = filters[i].first.c_str();
            nfdFilters[i].spec = filters[i].second.c_str();
        }
    }

    NFD::UniquePathU8 outPath;
    nfdresult_t result = NFD::OpenDialog(outPath, nfdFilters, filterCount);

    std::optional<std::filesystem::path> path;
    if (result == NFD_OKAY) {
        path = std::filesystem::path(outPath.get());
    }

    delete[] nfdFilters;
    return path;
}

std::optional<std::filesystem::path> SaveFileDialog(std::string const& title, std::string const& defaultPath, std::vector<std::pair<std::string, std::string>> const& filters)
{
    NFD::Guard nfdGuard;

    nfdfilteritem_t* nfdFilters = nullptr;
    size_t filterCount = 0;

    if (!filters.empty()) {
        nfdFilters = new nfdfilteritem_t[filters.size()];
        filterCount = filters.size();

        for (size_t i = 0; i < filters.size(); i++) {
            nfdFilters[i].name = filters[i].first.c_str();
            nfdFilters[i].spec = filters[i].second.c_str();
        }
    }

    NFD::UniquePathU8 outPath;
    nfdresult_t result = NFD::SaveDialog(outPath, nfdFilters, filterCount,
        defaultPath.empty() ? nullptr : defaultPath.c_str());

    std::optional<std::filesystem::path> path;
    if (result == NFD_OKAY) {
        path = std::filesystem::path(outPath.get());
    }

    delete[] nfdFilters;
    return path;
}

std::optional<std::filesystem::path> OpenDirectoryDialog()
{
    NFD::Guard nfdGuard;

    NFD::UniquePathU8 outPath;
    nfdresult_t result = NFD::PickFolder(outPath);

    std::optional<std::filesystem::path> path;
    if (result == NFD_OKAY) {
        path = std::filesystem::path(outPath.get());
    }
    return path;
}
}
