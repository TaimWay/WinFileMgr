// FileList.cpp

#include "FileList.h"
#include "log.h"
#include <shellapi.h>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

// 构造函数（无需改动，仅初始化 m_iconCache）
FileList::FileList(IconCache* iconCache) : m_iconCache(iconCache) {
    // 初始时可将当前工作目录设为当前路径
    m_currentPath = fs::current_path();
    RefreshImpl();
}

// 私有：直接设置路径并刷新，不修改历史
void FileList::SetCurrentPath(const fs::path& newPath) {
    if (newPath == m_currentPath) return;
    if (fs::exists(newPath) && fs::is_directory(newPath)) {
        m_currentPath = newPath;
        RefreshImpl();
    } else {
        LOG_ERROR("SetCurrentPath: invalid directory: %s", newPath.string().c_str());
    }
}

// 公共导航：压栈 + 清前进 + 设置新路径
void FileList::NavigateTo(const fs::path& newPath) {
    if (newPath == m_currentPath) return;
    if (!fs::exists(newPath) || !fs::is_directory(newPath)) {
        LOG_ERROR("NavigateTo: invalid directory: %s", newPath.string().c_str());
        return;
    }

    // 将当前路径压入后退栈
    if (!m_currentPath.empty()) {
        m_backStack.push_back(m_currentPath);
    }
    // 清空前进栈（新路径产生新的前进分支）
    m_forwardStack.clear();
    // 设置新路径
    SetCurrentPath(newPath);
}

// 后退：跳过无效路径，直到找到有效路径或栈空
bool FileList::GoBack() {
    if (m_backStack.empty()) return false;

    fs::path current = m_currentPath; // 保存当前路径，用于压入前进栈
    while (!m_backStack.empty()) {
        fs::path prev = m_backStack.back();
        m_backStack.pop_back();

        if (fs::exists(prev) && fs::is_directory(prev)) {
            // 有效路径：将当前路径压入前进栈，然后切换到 prev
            m_forwardStack.push_back(current);
            SetCurrentPath(prev);
            return true;
        }
        // 无效路径：丢弃，继续循环
    }
    // 没有有效路径可回退，当前路径保持不变
    return false;
}

// 前进：类似后退，处理无效路径
bool FileList::GoForward() {
    if (m_forwardStack.empty()) return false;

    fs::path current = m_currentPath;
    while (!m_forwardStack.empty()) {
        fs::path next = m_forwardStack.back();
        m_forwardStack.pop_back();

        if (fs::exists(next) && fs::is_directory(next)) {
            m_backStack.push_back(current);
            SetCurrentPath(next);
            return true;
        }
    }
    return false;
}

// 检查后退栈中是否存在至少一个有效路径
bool FileList::CanGoBack() const {
    for (auto it = m_backStack.rbegin(); it != m_backStack.rend(); ++it) {
        if (fs::exists(*it) && fs::is_directory(*it))
            return true;
    }
    return false;
}

bool FileList::CanGoForward() const {
    for (auto it = m_forwardStack.rbegin(); it != m_forwardStack.rend(); ++it) {
        if (fs::exists(*it) && fs::is_directory(*it))
            return true;
    }
    return false;
}

// 刷新：重新扫描当前目录（不改变历史）
void FileList::Refresh() {
    RefreshImpl();
}

// 内部刷新实现（添加异常保护）
void FileList::RefreshImpl() {
    m_entries.clear();
    if (m_currentPath.empty())
        return;

    try {
        for (auto& entry : fs::directory_iterator(m_currentPath)) {
            FileEntry fe;
            fe.path = entry.path();
            fe.isDirectory = entry.is_directory();
            if (!fe.isDirectory && entry.is_regular_file()) {
                fe.size = entry.file_size();
            } else {
                fe.size = 0;
            }
            fe.lastWriteTime = entry.last_write_time();
            m_entries.push_back(fe);
        }
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error in Refresh for %s: %s", m_currentPath.string().c_str(), e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("Standard exception in Refresh: %s", e.what());
    } catch (...) {
        LOG_ERROR("Unknown exception in Refresh for %s", m_currentPath.string().c_str());
    }

    // 排序：目录在前，文件在后，按文件名排序
    std::sort(m_entries.begin(), m_entries.end(),
        [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory)
                return a.isDirectory;
            return a.path.filename() < b.path.filename();
        });
}

// 双击打开条目（文件夹用 NavigateTo，文件用 ShellExecute）
void FileList::OpenEntry(const FileEntry& entry) {
    if (entry.isDirectory) {
        NavigateTo(entry.path);
    } else {
        ShellExecuteW(nullptr, L"open", entry.path.c_str(), nullptr, nullptr, SW_SHOW);
    }
}

void FileList::Draw() {
    if (m_currentPath.empty()) {
        ImGui::Text("No folder selected.");
        return;
    }

    // 使用表格布局
    if (ImGui::BeginTable("FileListTable", 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        for (auto& entry : m_entries) {
            // 为每一行分配唯一 ID
            ImGui::PushID(entry.path.string().c_str());

            ImGui::TableNextRow();

            // 第0列：图标和文件名
            ImGui::TableSetColumnIndex(0);
            ImTextureID tex = m_iconCache->GetTexture(entry.path, entry.isDirectory);
            ImGui::Image(tex, ImVec2(16, 16));
            ImGui::SameLine();

            std::string displayName = entry.path.filename().string();
            ImGui::Selectable(displayName.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                OpenEntry(entry);
            }

            // 第1列：大小
            ImGui::TableSetColumnIndex(1);
            if (!entry.isDirectory && entry.size > 0) {
                if (entry.size < 1024)
                    ImGui::Text("%llu B", entry.size);
                else if (entry.size < 1024 * 1024)
                    ImGui::Text("%.1f KB", entry.size / 1024.0);
                else if (entry.size < 1024 * 1024 * 1024)
                    ImGui::Text("%.1f MB", entry.size / (1024.0 * 1024.0));
                else
                    ImGui::Text("%.1f GB", entry.size / (1024.0 * 1024.0 * 1024.0));
            } else {
                ImGui::Text("");
            }

            // 第2列：修改时间
            ImGui::TableSetColumnIndex(2);
            auto ftime = entry.lastWriteTime;
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            std::tm* tm = std::localtime(&cftime);
            if (tm) {
                char buffer[64];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm);
                ImGui::Text("%s", buffer);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}
