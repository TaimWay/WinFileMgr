// IconCache.h
#pragma once
#include <imgui.h>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <windows.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

class IconCache {
public:
    IconCache();
    ~IconCache();

    // Retrieve texture ID for a file/folder path. If not cached, load it.
    ImTextureID GetTexture(const std::filesystem::path& path, bool isFolder);

private:
    struct IconInfo {
        ImTextureID textureId;
        int width, height;
    };

    std::unordered_map<std::wstring, IconInfo> m_cache; // Keyed by extension or special marker for folders

    // Default icons
    ImTextureID m_defaultFolderIcon;
    ImTextureID m_defaultFileIcon;

    // Load icon from file path and convert to OpenGL texture
    ImTextureID LoadIcon(const std::filesystem::path& path, bool isFolder);

    // Helper to convert HICON to OpenGL texture (RGBA)
    ImTextureID HICONToTexture(HICON hIcon);
};