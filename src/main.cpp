// main.cpp
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include "log.h"
bool g_ConsoleOutput = false;
#include "SidebarTree.h"
#include "FileList.h"
#include "IconCache.h"

// Global clear color for background
static float g_ClearColor[3] = {0.94f, 0.94f, 0.94f};

// Theme switching functions
void SetLightTheme()
{
    ImGui::StyleColorsLight();
    g_ClearColor[0] = 0.94f;
    g_ClearColor[1] = 0.94f;
    g_ClearColor[2] = 0.94f;
}

void SetDarkTheme()
{
    ImGui::StyleColorsDark();
    g_ClearColor[0] = 0.1f;
    g_ClearColor[1] = 0.1f;
    g_ClearColor[2] = 0.1f;
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

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Create shared icon cache
    IconCache iconCache;

    // Create sidebar tree and file list, passing the icon cache
    SidebarTree sidebar(&iconCache);
    FileList fileList(&iconCache);

    // Set callback when a folder is selected in the sidebar
    sidebar.SetOnFolderSelected([&](const std::filesystem::path &folder)
                                { fileList.NavigateTo(folder); });

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

        if (ImGui::Button("Back"))
        {
            fileList.GoBack();
        }
        ImGui::SameLine();
        if (ImGui::Button("Forward"))
        {
            fileList.GoForward();
        }
        ImGui::SameLine();
        if (ImGui::Button("Up"))
        {
            std::filesystem::path parent = fileList.GetCurrentPath().parent_path();
            if (parent != fileList.GetCurrentPath())
            {
                fileList.NavigateTo(parent); // 使用 NavigateTo，自动处理历史
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
        {
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