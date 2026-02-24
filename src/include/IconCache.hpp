// IconCache.hpp
// Windows icon caching system for FileMgr
// 
// Provides efficient loading and caching of file/folder icons from the Windows shell.
// Icons are extracted using Windows API (SHGetFileInfo) and converted to OpenGL textures
// for use with Dear ImGui's image rendering.
// 
#pragma once

#include <imgui.h>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <windows.h>

// OpenGL constants (fallback definitions if not already defined by GL headers)
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

// -----------------------------------------------------------------------------
// IconCache class
// -----------------------------------------------------------------------------
class IconCache {
public:
    // Constructor - initializes common controls and loads default icons
    IconCache();
    
    // Destructor - cleans up OpenGL textures (if needed)
    ~IconCache();

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------
    
    // Retrieve texture ID for a file or folder path
    // @param path     Filesystem path to get icon for
    // @param isFolder True if the path represents a directory
    // @return ImTextureID ready for use with ImGui::Image() or 0 if icon unavailable
    ImTextureID GetTexture(const std::filesystem::path& path, bool isFolder);

    // Load a custom icon from an ICO file and convert to texture
    // Used for button icons (back, forward, refresh, etc.)
    // @param icoPath Path to .ico file
    // @return ImTextureID or 0 on failure
    ImTextureID LoadIconFromICO(const std::filesystem::path& icoPath);

private:
    // -------------------------------------------------------------------------
    // Internal structures
    // -------------------------------------------------------------------------
    
    // Cached icon information
    struct IconInfo {
        ImTextureID textureId;    // OpenGL texture ID
        int width, height;        // Icon dimensions
    };

    // -------------------------------------------------------------------------
    // Member variables
    // -------------------------------------------------------------------------
    
    // Icon cache keyed by file extension (or "<FOLDER>" for folders)
    std::unordered_map<std::wstring, IconInfo> m_cache;

    // Default icons used when specific icon cannot be loaded
    ImTextureID m_defaultFolderIcon;
    ImTextureID m_defaultFileIcon;

    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------
    
    // Load icon for a specific file or folder (internal implementation)
    // @param path     Filesystem path
    // @param isFolder True for folders, false for files
    // @return ImTextureID or 0 on failure
    ImTextureID LoadIcon(const std::filesystem::path& path, bool isFolder);

    // Convert Windows HICON to OpenGL texture (RGBA format)
    // @param hIcon Windows icon handle
    // @return ImTextureID or 0 on failure
    ImTextureID HICONToTexture(HICON hIcon);
};