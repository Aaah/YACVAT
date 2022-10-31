cmake_minimum_required(VERSION 3.0.0)
project(yacvat VERSION 0.1.0)

# todo : set version in header file and parse it
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(OpenGL_GL_PREFERENCE LEGACY)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})

add_executable(yacvat main.cpp 
    app.cpp 
    annotations.cpp
    rectangle.cpp
    notofont.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/backends/imgui_impl_sdl.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_include_directories(yacvat PRIVATE 
    lib/imgui/ 
    lib/imgui/backends/ 
    lib/ImGuiFileDialog/ 
    lib/spdlog/include/
    lib/stb/ 
    lib/json/include/
    lib/fsm)

add_subdirectory(lib/spdlog)
add_subdirectory(lib/json)

target_link_libraries(yacvat spdlog nlohmann_json -lpthread ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

# compile options
target_compile_options(yacvat PRIVATE -std=c++11 -g -Wall -Wformat)

# definitions
target_compile_definitions(yacvat PRIVATE SPDLOG_COMPILED_LIB)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
