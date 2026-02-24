// SidebarTree.h
#pragma once
#include <imgui.h>
#include <filesystem>
#include <functional>
#include <vector>
#include "IconCache.h"

class SidebarTree {
public:
    SidebarTree(IconCache* iconCache);
    void Draw();

    // Callback when a folder is selected (double-click or single-click? we'll use single-click activation)
    void SetOnFolderSelected(std::function<void(const std::filesystem::path&)> callback) {
        m_onFolderSelected = callback;
    }

private:
    IconCache* m_iconCache;
    std::function<void(const std::filesystem::path&)> m_onFolderSelected;

    // Recursively draw a directory node
    void DrawTreeNode(const std::filesystem::path& path, const std::string& displayName);

    // Cache of subdirectories for a given path (to avoid re-scanning each frame)
    struct DirCache {
        std::vector<std::filesystem::path> subDirs;
        std::filesystem::file_time_type lastWriteTime; // Not used now, could be used for invalidation
    };
    std::unordered_map<std::wstring, DirCache> m_dirCache;

    // Get subdirectories of a path (with caching)
    const std::vector<std::filesystem::path>& GetSubDirectories(const std::filesystem::path& path);
};