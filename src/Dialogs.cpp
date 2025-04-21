#pragma once

#include "Dialogs.h"
#ifdef IMP_PLATFORM_WINDOWS
#    include <Windows.h>
#    include <codecvt>
#    include <shlobj.h>
#    include <shobjidl.h>
#endif

namespace imp {
void ShowFatalDialog(char const* title, char const* message)
{
#ifdef IMP_PLATFORM_WINDOWS
    MessageBoxA(nullptr, message, title, MB_OK | MB_ICONERROR);
#endif
}

std::optional<std::filesystem::path> OpenFileDialog(std::vector<std::pair<std::string, std::string>> const& filters)
{
#ifdef IMP_PLATFORM_WINDOWS
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

        if (SUCCEEDED(hr)) {
            if (!filters.empty()) {
                std::vector<COMDLG_FILTERSPEC> fileTypes;
                std::vector<std::wstring> filterNames;
                std::vector<std::wstring> filterSpecs;

                for (auto const& filter : filters) {
                    int nameLen = MultiByteToWideChar(CP_UTF8, 0, filter.first.c_str(), -1, nullptr, 0);
                    std::wstring wName(nameLen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, filter.first.c_str(), -1, &wName[0], nameLen);

                    int specLen = MultiByteToWideChar(CP_UTF8, 0, filter.second.c_str(), -1, nullptr, 0);
                    std::wstring wSpec(specLen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, filter.second.c_str(), -1, &wSpec[0], specLen);

                    if (!wName.empty() && wName.back() == L'\0')
                        wName.pop_back();
                    if (!wSpec.empty() && wSpec.back() == L'\0')
                        wSpec.pop_back();

                    filterNames.push_back(std::move(wName));
                    filterSpecs.push_back(std::move(wSpec));

                    COMDLG_FILTERSPEC fs;
                    fs.pszName = filterNames.back().c_str();
                    fs.pszSpec = filterSpecs.back().c_str();
                    fileTypes.push_back(fs);
                }

                pFileOpen->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data());
            }

            hr = pFileOpen->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR filePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                    if (SUCCEEDED(hr)) {
                        std::wstring wstrPath(filePath);
                        std::string strPath(wstrPath.begin(), wstrPath.end());
                        CoTaskMemFree(filePath);
                        pItem->Release();
                        pFileOpen->Release();
                        CoUninitialize();
                        return std::filesystem::path(strPath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
#endif
    return std::nullopt;
}
std::optional<std::filesystem::path> SaveFileDialog(std::string const& title, std::string const& defaultPath, std::vector<std::pair<std::string, std::string>> const& filters)
{
#ifdef IMP_PLATFORM_WINDOWS
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileSaveDialog* pFileSave;
        hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileSave));

        if (SUCCEEDED(hr)) {
            if (!filters.empty()) {
                std::vector<COMDLG_FILTERSPEC> fileTypes;
                std::vector<std::wstring> filterNames;
                std::vector<std::wstring> filterSpecs;

                for (auto const& filter : filters) {
                    int nameLen = MultiByteToWideChar(CP_UTF8, 0, filter.first.c_str(), -1, nullptr, 0);
                    std::wstring wName(nameLen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, filter.first.c_str(), -1, &wName[0], nameLen);

                    int specLen = MultiByteToWideChar(CP_UTF8, 0, filter.second.c_str(), -1, nullptr, 0);
                    std::wstring wSpec(specLen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, filter.second.c_str(), -1, &wSpec[0], specLen);

                    if (!wName.empty() && wName.back() == L'\0')
                        wName.pop_back();
                    if (!wSpec.empty() && wSpec.back() == L'\0')
                        wSpec.pop_back();

                    filterNames.push_back(std::move(wName));
                    filterSpecs.push_back(std::move(wSpec));

                    COMDLG_FILTERSPEC fs;
                    fs.pszName = filterNames.back().c_str();
                    fs.pszSpec = filterSpecs.back().c_str();
                    fileTypes.push_back(fs);
                }

                pFileSave->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data());
            }

            if (!defaultPath.empty()) {
                std::wstring wDefaultPath(defaultPath.begin(), defaultPath.end());
                pFileSave->SetFileName(wDefaultPath.c_str());
            }

            hr = pFileSave->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR filePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                    if (SUCCEEDED(hr)) {
                        std::wstring wstrPath(filePath);
                        std::string strPath(wstrPath.begin(), wstrPath.end());
                        CoTaskMemFree(filePath);
                        pItem->Release();
                        pFileSave->Release();
                        CoUninitialize();
                        return std::filesystem::path(strPath);
                    }
                    pItem->Release();
                }
            }
            pFileSave->Release();
        }
        CoUninitialize();
    }
#endif
    return std::nullopt;
}
std::optional<std::filesystem::path> OpenDirectoryDialog()
{
#ifdef IMP_PLATFORM_WINDOWS
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            hr = pFileOpen->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) {
                hr = pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
                if (SUCCEEDED(hr)) {
                    hr = pFileOpen->Show(nullptr);
                    if (SUCCEEDED(hr)) {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr)) {
                            PWSTR folderPath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);
                            if (SUCCEEDED(hr)) {
                                std::wstring wstrPath(folderPath);
                                std::string strPath(wstrPath.begin(), wstrPath.end());
                                CoTaskMemFree(folderPath);
                                pItem->Release();
                                pFileOpen->Release();
                                CoUninitialize();
                                return std::filesystem::path(strPath);
                            }
                            pItem->Release();
                        }
                    }
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
#endif
    return std::nullopt;
}
}
