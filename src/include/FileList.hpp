// FileList.hpp
// Main file list component for FileMgr
// 
// Displays files and directories in a table view with columns for name, size,
// and modification date. Supports navigation history (back/forward), directory
// refreshing, and file opening via ShellExecute.
// 
#pragma once

#include <imgui.h>
#include <filesystem>
#include <vector>
#include <functional>
#include <optional>
#include "IconCache.hpp"

// -----------------------------------------------------------------------------
// FileList class
// -----------------------------------------------------------------------------
class FileList {
public:
    // -------------------------------------------------------------------------
    // Construction / Initialization
    // -------------------------------------------------------------------------
    
    // Constructor
    // @param iconCache Pointer to shared icon cache (must outlive this object)
    explicit FileList(IconCache* iconCache);

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------
    
    // Set current directory (immediate navigation, no history modification)
    // @param newPath Directory to display
    void SetPath(const std::filesystem::path& newPath);
    
    // Draw the file list UI using ImGui
    void Draw();
    
    // Get current directory path
    // @return Reference to current path
    const std::filesystem::path& GetCurrentPath() const { return m_currentPath; }
    
    // Refresh current directory (re-scan files)
    void Refresh();
    
    // Navigate to new directory (adds to history)
    // @param newPath Directory to navigate to
    void NavigateTo(const std::filesystem::path& newPath);
    
    // Navigate back in history
    // @return True if navigation occurred, false if no history
    bool GoBack();
    
    // Navigate forward in history
    // @return True if navigation occurred, false if no forward history
    bool GoForward();
    
    // Check if back navigation is possible
    // @return True if there are valid directories in back history
    bool CanGoBack() const;
    
    // Check if forward navigation is possible
    // @return True if there are valid directories in forward history
    bool CanGoForward() const;
    
    // Request deferred navigation (used by sidebar tree)
    // @param path Directory to navigate to (processed during next Draw())
    void RequestNavigation(const std::filesystem::path& path);
    
private:
    // -------------------------------------------------------------------------
    // Internal structures
    // -------------------------------------------------------------------------
    
    // File entry for display
    struct FileEntry {
        std::filesystem::path path;           // Full path
        bool isDirectory;                      // True for directories
        std::uintmax_t size;                   // File size in bytes (0 for directories)
        std::filesystem::file_time_type lastWriteTime; // Last modification time
    };

    // -------------------------------------------------------------------------
    // Member variables
    // -------------------------------------------------------------------------
    
    IconCache* m_iconCache;                    // Shared icon cache
    std::filesystem::path m_currentPath;       // Currently displayed directory
    
    std::vector<FileEntry> m_entries;          // Files in current directory
    
    std::vector<std::filesystem::path> m_backStack;    // Back navigation history
    std::vector<std::filesystem::path> m_forwardStack; // Forward navigation history
    
    std::optional<std::filesystem::path> m_pendingNavigation; // Deferred navigation request

    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Open a file entry (directory navigation or file execution)
    // @param entry File/directory to open
    void OpenEntry(const FileEntry& entry);
    
    // Internal refresh implementation
    void RefreshImpl();
    
    // Set current path without modifying history
    // @param newPath Directory to set as current
    void SetCurrentPath(const std::filesystem::path& newPath);
    
    // Process any pending navigation request
    void ProcessPendingNavigation();
};