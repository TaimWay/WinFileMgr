// IconCache.cpp
#include "IconCache.h"
#include "log.h"
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#pragma comment(lib, "comctl32.lib")

namespace fs = std::filesystem;

IconCache::IconCache() {
    // 初始化通用控件（确保图标提取正常）
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // ---------- 加载默认文件夹图标 ----------
    // 优先从实际文件夹获取（例如桌面）
    PWSTR desktopPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
        fs::path desktop(desktopPath);
        m_defaultFolderIcon = LoadIcon(desktop, true);
        CoTaskMemFree(desktopPath);
    }
    // 如果失败，从 shell32.dll 提取（索引 4 通常为文件夹）
    if (!m_defaultFolderIcon) {
        HICON hIcon = ExtractIconW(GetModuleHandle(NULL), L"shell32.dll", 4);
        if (hIcon) {
            m_defaultFolderIcon = HICONToTexture(hIcon);
            DestroyIcon(hIcon);
        }
    }
    // 最终保底：设为 0（后续使用时不显示图标）
    if (!m_defaultFolderIcon) {
        m_defaultFolderIcon = 0;
    }

    // ---------- 加载默认文件图标 ----------
    // 尝试从临时文件获取（例如 .txt）
    fs::path tempFile = fs::temp_directory_path() / L"dummy.txt";
    m_defaultFileIcon = LoadIcon(tempFile, false);
    if (!m_defaultFileIcon) {
        // 从 shell32.dll 提取文档图标（索引 2 通常是文档）
        HICON hIcon = ExtractIconW(GetModuleHandle(NULL), L"shell32.dll", 2);
        if (hIcon) {
            m_defaultFileIcon = HICONToTexture(hIcon);
            DestroyIcon(hIcon);
        }
    }
    if (!m_defaultFileIcon) {
        m_defaultFileIcon = 0;
    }
}

IconCache::~IconCache() {
    // 删除缓存的图标纹理
    for (auto& [key, info] : m_cache) {
        glDeleteTextures(1, (GLuint*)&info.textureId);
    }
    // 删除默认图标纹理
    if (m_defaultFolderIcon)
        glDeleteTextures(1, (GLuint*)&m_defaultFolderIcon);
    if (m_defaultFileIcon)
        glDeleteTextures(1, (GLuint*)&m_defaultFileIcon);
}

ImTextureID IconCache::LoadIconFromICO(const std::filesystem::path& icoPath) {
    // 检查缓存
    std::wstring key = icoPath.wstring();
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        return it->second.textureId;
    }

    // 使用 LoadImageW 加载 ICO 文件
    HICON hIcon = (HICON)LoadImageW(
        NULL,
        icoPath.wstring().c_str(),
        IMAGE_ICON,
        16, 16,  // 请求 16x16 图标
        LR_LOADFROMFILE | LR_SHARED
    );

    if (!hIcon) {
        // 尝试加载默认尺寸
        hIcon = (HICON)LoadImageW(
            NULL,
            icoPath.wstring().c_str(),
            IMAGE_ICON,
            0, 0,
            LR_LOADFROMFILE | LR_SHARED | LR_DEFAULTSIZE
        );
    }

    if (!hIcon) {
        LOG_ERROR("Failed to load icon from %s", icoPath.string().c_str());
        // 缓存失败结果（null）避免重复尝试
        m_cache[key] = {0, 0, 0};
        return 0;
    }

    ImTextureID tex = HICONToTexture(hIcon);
    DestroyIcon(hIcon);

    // 缓存结果
    m_cache[key] = {tex, 16, 16};
    return tex;
}

ImTextureID IconCache::GetTexture(const std::filesystem::path& path, bool isFolder) {
    // 文件夹使用路径作为键，文件使用扩展名（小写）作为键
    std::wstring key;
    if (isFolder) {
        key = path.wstring();               // 使用完整路径区分不同文件夹
    } else {
        key = path.extension().wstring();
        for (auto& c : key) c = towlower(c);
    }

    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        return it->second.textureId;
    }

    ImTextureID tex = LoadIcon(path, isFolder);
    // 缓存图标，即使为 0 也缓存，避免重复失败尝试
    m_cache[key] = { tex, 32, 32 };
    return tex;
}

ImTextureID IconCache::LoadIcon(const std::filesystem::path& path, bool isFolder) {
    SHFILEINFOW sfi = {0};
    UINT flags = SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
    DWORD attr = isFolder ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    std::wstring pathStr = path.wstring();
    DWORD_PTR result = SHGetFileInfoW(pathStr.c_str(), attr, &sfi, sizeof(sfi), flags);
    if (result && sfi.hIcon) {
        ImTextureID tex = HICONToTexture(sfi.hIcon);
        DestroyIcon(sfi.hIcon);
        return tex;
    }
    // 获取失败，返回默认图标
    return isFolder ? m_defaultFolderIcon : m_defaultFileIcon;
}

ImTextureID IconCache::HICONToTexture(HICON hIcon) {
    // 获取图标尺寸
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo))
        return 0;

    BITMAP bm;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);
    int width = bm.bmWidth;
    int height = bm.bmHeight;

    // 创建设备上下文和 DIB 段
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // 自上而下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits;
    HBITMAP hBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!hBitmap) {
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return 0;
    }

    SelectObject(memDC, hBitmap);
    DrawIconEx(memDC, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);

    // 生成 OpenGL 纹理
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bits);

    // 清理
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return (ImTextureID)(intptr_t)texID;
}