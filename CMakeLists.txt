cmake_minimum_required(VERSION 3.10)
project(FileMgr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_DIR ${CMAKE_SOURCE_DIR}/lib/glfw64)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)

include_directories(
    ${GLFW_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

file(GLOB_RECURSE IMGUI_SRCS ${IMGUI_DIR}/*.cpp)
file(GLOB_RECURSE APP_SRCS src/*.cpp)
file(GLOB APP_RC src/source/*.rc)

add_executable(FileMgr WIN32 ${APP_SRCS} ${IMGUI_SRCS} ${APP_RC})

target_include_directories(FileMgr PRIVATE
    ${GLFW_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/src/include
)

target_link_directories(FileMgr PRIVATE ${GLFW_DIR}/lib-mingw-w64)

target_link_libraries(FileMgr
    glfw3
    opengl32
    gdi32
    shell32
    comctl32
    comdlg32
    advapi32
    user32
)

if (MINGW)
    target_link_options(FileMgr PRIVATE -static-libgcc -static-libstdc++)
endif()
