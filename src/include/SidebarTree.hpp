// SidebarTree.hpp
// Sidebar tree component for FileMgr
// 
// Displays a hierarchical tree view of drives and directories on Windows.
// Uses caching to avoid repeated filesystem scans and provides folder selection
// callbacks for integration with the main file list.
// 
#pragma once

#include <imgui.h>
#include <filesystem>
#include <functional>
#include <vector>
#include "IconCache.hpp"

// -----------------------------------------------------------------------------
// SidebarTree class
// -----------------------------------------------------------------------------
class SidebarTree {
public:
    // -------------------------------------------------------------------------
    // Construction / Initialization
    // -------------------------------------------------------------------------
    
    // Constructor
    // @param iconCache Pointer to shared icon cache (must outlive this object)
    explicit SidebarTree(IconCache* iconCache);
    
    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------
    
    // Draw the sidebar tree UI using ImGui
    void Draw();
    
    // Set callback for folder selection
    // @param callback Function called when user clicks on a folder in the tree
    void SetOnFolderSelected(std::function<void(const std::filesystem::path&)> callback) {
        m_onFolderSelected = callback;
    }
    
private:
    // -------------------------------------------------------------------------
    // Member variables
    // -------------------------------------------------------------------------
    
    IconCache* m_iconCache;                            // Shared icon cache
    std::function<void(const std::filesystem::path&)> m_onFolderSelected; // Selection callback
    
    // -------------------------------------------------------------------------
    // Internal structures
    // -------------------------------------------------------------------------
    
    // Cached directory information
    struct DirCache {
        std::vector<std::filesystem::path> subDirs;    // Immediate subdirectories
        std::filesystem::file_time_type lastWriteTime; // For cache invalidation (not currently used)
    };
    
    // Cache of directory contents (keyed by wide string path)
    std::unordered_map<std::wstring, DirCache> m_dirCache;
    
    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Recursively draw a directory node in the tree
    // @param path        Directory path
    // @param displayName Name to display in the tree node
    void DrawTreeNode(const std::filesystem::path& path, const std::string& displayName);
    
    // Get subdirectories of a path (with caching)
    // @param path Directory path
    // @return Reference to vector of subdirectory paths
    const std::vector<std::filesystem::path>& GetSubDirectories(const std::filesystem::path& path);
};