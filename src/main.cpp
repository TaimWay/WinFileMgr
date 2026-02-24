// main.cpp
// FileMgr - Lightweight GUI File Manager for Windows
// 
// Main application entry point and window management.
// 
// Features:
// - Modern UI using Dear ImGui with light/dark theme support
// - File list with icon view, size, and date columns
// - Sidebar tree view of drives and directories
// - Navigation history (back/forward)
// - Windows shell integration for file opening
// - Custom icon loading and caching
// 
// Build requirements:
// - C++17 compiler
// - GLFW 3.3+
// - OpenGL 3.3+
// - Windows SDK (for shell integration)
// 
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include "include/log.hpp"
#include <windows.h>
bool g_ConsoleOutput = false;
#include "include/SidebarTree.hpp"
#include "include/FileList.hpp"
#include "include/IconCache.hpp"

// Global clear color for background
static float g_ClearColor[3] = {0.94f, 0.94f, 0.94f};

// Theme switching functions
void SetLightTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Light theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.98f, 0.98f, 0.98f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.80f, 0.80f, 0.80f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.90f, 0.90f, 0.90f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.88f, 0.88f, 0.88f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.82f, 0.82f, 0.82f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.50f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 0.50f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.90f);
    colors[ImGuiCol_Button]                 = ImVec4(0.90f, 0.90f, 0.90f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.84f, 0.84f, 0.84f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.51f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 0.91f);
    colors[ImGuiCol_Separator]              = ImVec4(0.80f, 0.80f, 0.80f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.60f, 0.50f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.76f, 0.76f, 0.76f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.82f, 0.82f, 0.82f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);

    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    // Style variables
    style.WindowPadding            = ImVec2(8.0f, 8.0f);
    style.FramePadding             = ImVec2(6.0f, 4.0f);
    style.CellPadding              = ImVec2(4.0f, 2.0f);
    style.ItemSpacing              = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing         = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding        = ImVec2(0.0f, 0.0f);
    style.IndentSpacing            = 21.0f;
    style.ScrollbarSize            = 14.0f;
    style.GrabMinSize              = 10.0f;
    style.WindowBorderSize         = 1.0f;
    style.ChildBorderSize          = 1.0f;
    style.PopupBorderSize          = 1.0f;
    style.FrameBorderSize          = 0.0f;
    style.TabBorderSize            = 0.0f;
    style.WindowRounding           = 6.0f;
    style.ChildRounding            = 4.0f;
    style.FrameRounding            = 4.0f;
    style.PopupRounding            = 4.0f;
    style.ScrollbarRounding        = 4.0f;
    style.GrabRounding             = 4.0f;
    style.TabRounding              = 4.0f;
    style.WindowTitleAlign         = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign          = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign      = ImVec2(0.0f, 0.0f);

    g_ClearColor[0] = 0.94f;
    g_ClearColor[1] = 0.94f;
    g_ClearColor[2] = 0.94f;
}

void SetDarkTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Dark theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.14f, 0.14f, 0.14f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.28f, 0.28f, 0.28f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.32f, 0.32f, 0.32f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.10f, 0.10f, 0.10f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.60f, 0.60f, 0.60f, 0.50f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.50f, 0.50f, 0.50f, 0.90f);
    colors[ImGuiCol_Button]                 = ImVec4(0.28f, 0.28f, 0.28f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.51f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 0.91f);
    colors[ImGuiCol_Separator]              = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.16f, 0.16f, 0.16f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.40f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.12f, 0.12f, 0.12f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);

    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    // Style variables (same as light theme for consistency)
    style.WindowPadding            = ImVec2(8.0f, 8.0f);
    style.FramePadding             = ImVec2(6.0f, 4.0f);
    style.CellPadding              = ImVec2(4.0f, 2.0f);
    style.ItemSpacing              = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing         = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding        = ImVec2(0.0f, 0.0f);
    style.IndentSpacing            = 21.0f;
    style.ScrollbarSize            = 14.0f;
    style.GrabMinSize              = 10.0f;
    style.WindowBorderSize         = 1.0f;
    style.ChildBorderSize          = 1.0f;
    style.PopupBorderSize          = 1.0f;
    style.FrameBorderSize          = 0.0f;
    style.TabBorderSize            = 0.0f;
    style.WindowRounding           = 6.0f;
    style.ChildRounding            = 4.0f;
    style.FrameRounding            = 4.0f;
    style.PopupRounding            = 4.0f;
    style.ScrollbarRounding        = 4.0f;
    style.GrabRounding             = 4.0f;
    style.TabRounding              = 4.0f;
    style.WindowTitleAlign         = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign          = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign      = ImVec2(0.0f, 0.0f);

    g_ClearColor[0] = 0.12f;
    g_ClearColor[1] = 0.12f;
    g_ClearColor[2] = 0.12f;
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--console") == 0)
        {
            g_ConsoleOutput = true;
        }
    }

    // Initialize GLFW
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "File Explorer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SetLightTheme();

    // Load a nicer font (Segoe UI on Windows)
    ImFont* font = nullptr;
    std::string fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    if (std::filesystem::exists(fontPath))
    {
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
    }
    if (!font)
    {
        // Fallback to default font
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Create shared icon cache
    IconCache iconCache;

    // Load button icons
    ImTextureID iconBack = 0;
    ImTextureID iconForward = 0;
    ImTextureID iconUp = 0;
    ImTextureID iconRefresh = 0;
    try {
        namespace fs = std::filesystem;
        fs::path resourceDir = "src/resource";
        iconBack = iconCache.LoadIconFromICO(resourceDir / "icon_sub.ico");
        iconForward = iconCache.LoadIconFromICO(resourceDir / "icon_add.ico");
        iconUp = iconCache.LoadIconFromICO(resourceDir / "icon_up.ico");
        iconRefresh = iconCache.LoadIconFromICO(resourceDir / "icon_refresh.ico");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load button icons: %s", e.what());
    }

    // Create sidebar tree and file list, passing the icon cache
    SidebarTree sidebar(&iconCache);
    FileList fileList(&iconCache);

    // Set callback when a folder is selected in the sidebar
    sidebar.SetOnFolderSelected([&](const std::filesystem::path &folder)
                                { fileList.RequestNavigation(folder); });

    // Initially set file list to current working directory
    fileList.NavigateTo(std::filesystem::current_path());

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ----- Main Menu Bar -----
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Theme"))
            {
                if (ImGui::MenuItem("Light Mode"))
                    SetLightTheme();
                if (ImGui::MenuItem("Dark Mode"))
                    SetDarkTheme();
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // ----- Main Window (occupies entire viewport except menu bar) -----
        // Use ImGui::Begin with flags to fill remaining space
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - ImGui::GetFrameHeight()));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("MainWindow", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

        // Back button
        bool backClicked = false;
        if (iconBack) {
            backClicked = ImGui::ImageButton("##back", iconBack, ImVec2(16, 16));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Back");
        } else {
            backClicked = ImGui::Button("Back");
        }
        if (backClicked) {
            fileList.GoBack();
        }
        ImGui::SameLine();
        
        // Forward button
        bool forwardClicked = false;
        if (iconForward) {
            forwardClicked = ImGui::ImageButton("##forward", iconForward, ImVec2(16, 16));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Forward");
        } else {
            forwardClicked = ImGui::Button("Forward");
        }
        if (forwardClicked) {
            fileList.GoForward();
        }
        ImGui::SameLine();
        
        // Up button
        bool upClicked = false;
        if (iconUp) {
            upClicked = ImGui::ImageButton("##up", iconUp, ImVec2(16, 16));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Up");
        } else {
            upClicked = ImGui::Button("Up");
        }
        if (upClicked) {
            std::filesystem::path parent = fileList.GetCurrentPath().parent_path();
            if (parent != fileList.GetCurrentPath())
            {
                fileList.NavigateTo(parent); // 使用 NavigateTo，自动处理历史
            }
        }
        ImGui::SameLine();
        
        // Refresh button
        bool refreshClicked = false;
        if (iconRefresh) {
            refreshClicked = ImGui::ImageButton("##refresh", iconRefresh, ImVec2(16, 16));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Refresh");
        } else {
            refreshClicked = ImGui::Button("Refresh");
        }
        if (refreshClicked) {
            fileList.Refresh();
        }
        ImGui::SameLine();

        static char pathBuf[512];
        strncpy(pathBuf, fileList.GetCurrentPath().string().c_str(), sizeof(pathBuf) - 1);
        pathBuf[sizeof(pathBuf) - 1] = '\0';
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##Address", pathBuf, sizeof(pathBuf), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::filesystem::path newPath = std::filesystem::u8path(pathBuf);
            try
            {
                if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath))
                {
                    fileList.NavigateTo(newPath);
                }
                else
                {
                    LOG_ERROR("Invalid directory: %s", pathBuf);
                    // 无效时可将输入框内容恢复为当前路径（可选）
                    strncpy(pathBuf, fileList.GetCurrentPath().string().c_str(), sizeof(pathBuf) - 1);
                }
            }
            catch (const std::exception &e)
            {
                LOG_ERROR("Error parsing path: %s", e.what());
            }
        }

        ImGui::Separator();

        ImGui::Columns(2, "MainColumns", false);

        // Split into two columns: left (sidebar) and right (file list)
        ImGui::Columns(2, "MainColumns", false);
        ImGui::SetColumnWidth(0, 250.0f);

        // Left column: Sidebar tree
        ImGui::BeginChild("Sidebar", ImVec2(0, 0), true);
        sidebar.Draw();
        ImGui::EndChild();

        ImGui::NextColumn();

        // Right column: File list
        ImGui::BeginChild("FileList", ImVec2(0, 0), true);
        fileList.Draw();
        ImGui::EndChild();

        ImGui::End(); // MainWindow
        ImGui::PopStyleVar(2);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(g_ClearColor[0], g_ClearColor[1], g_ClearColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}