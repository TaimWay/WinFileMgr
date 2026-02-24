// FileList.h
#pragma once
#include <imgui.h>
#include <filesystem>
#include <vector>
#include <functional>
#include <optional>
#include "IconCache.h"

class FileList {
public:
    FileList(IconCache* iconCache);

    void SetPath(const std::filesystem::path& newPath);
    void Draw();
    const std::filesystem::path& GetCurrentPath() const { return m_currentPath; }
    void Refresh();

    void NavigateTo(const std::filesystem::path& newPath);

    bool GoBack();
    bool GoForward();

    bool CanGoBack() const;
    bool CanGoForward() const;

    void RequestNavigation(const std::filesystem::path& path);
private:
    IconCache* m_iconCache;
    std::filesystem::path m_currentPath;

    struct FileEntry {
        std::filesystem::path path;
        bool isDirectory;
        std::uintmax_t size;
        std::filesystem::file_time_type lastWriteTime;
    };
    std::vector<FileEntry> m_entries;

    std::vector<std::filesystem::path> m_backStack;
    std::vector<std::filesystem::path> m_forwardStack;

    std::optional<std::filesystem::path> m_pendingNavigation;

    void OpenEntry(const FileEntry& entry);
    void RefreshImpl();
    void SetCurrentPath(const std::filesystem::path& newPath);
    void ProcessPendingNavigation();
};