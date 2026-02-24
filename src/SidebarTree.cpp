// SidebarTree.cpp
#include "SidebarTree.h"
#include <filesystem>
#include <vector>
#include <windows.h> // for GetLogicalDrives
#include <algorithm>
#include "log.h"

namespace fs = std::filesystem;

SidebarTree::SidebarTree(IconCache* iconCache) : m_iconCache(iconCache) {}

void SidebarTree::Draw() {
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            wchar_t root[4] = { wchar_t(L'A' + i), L':', L'\\', L'\0' };
            fs::path drivePath(root);
            std::string display = std::string(1, char('A' + i)) + ": Drive";
            DrawTreeNode(drivePath, display);
        }
    }
}

void SidebarTree::DrawTreeNode(const fs::path& path, const std::string& displayName) {
    ImGui::PushID(path.string().c_str());

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    bool hasChildren = false;
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (auto& entry : fs::directory_iterator(path)) {
                if (entry.is_directory()) {
                    hasChildren = true;
                    break;
                }
            }
        }
    } catch (...) {
    }

    if (!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    ImTextureID iconTex = m_iconCache->GetTexture(path, true);
    ImGui::Image(iconTex, ImVec2(16, 16)); ImGui::SameLine();

    bool nodeOpen = ImGui::TreeNodeEx(path.c_str(), flags, "%s", displayName.c_str());

    if (ImGui::IsItemClicked() && m_onFolderSelected) {
        m_onFolderSelected(path);
    }

    if (nodeOpen) {
        if (hasChildren) {
            const auto& subDirs = GetSubDirectories(path);
            for (const auto& subPath : subDirs) {
                std::string subName = subPath.filename().u8string();
                DrawTreeNode(subPath, subName);
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

const std::vector<fs::path>& SidebarTree::GetSubDirectories(const fs::path& path) {
    auto it = m_dirCache.find(path.wstring());
    if (it != m_dirCache.end()) {
        return it->second.subDirs;
    }

    DirCache cache;
    try {
        for (auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                cache.subDirs.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error in GetSubDirectories for path %s: %s", path.string().c_str(), e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("Standard exception in GetSubDirectories: %s", e.what());
    } catch (...) {
        LOG_ERROR("Unknown exception in GetSubDirectories for path %s", path.string().c_str());
    }

    std::sort(cache.subDirs.begin(), cache.subDirs.end(),
        [](const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });

    m_dirCache[path.wstring()] = std::move(cache);
    return m_dirCache[path.wstring()].subDirs;
}