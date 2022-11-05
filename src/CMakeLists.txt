# find all headers, optional but make them show up in IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${Yacvat_SOURCE_DIR}/include/yacvat/*.h")

# Dependencies
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})

# Make an automatic library - will be static or dynamic based on user setting
add_library(yacvatlib
app.cpp 
annotations.cpp
rectangle.cpp
notofont.cpp
fontawesome.cpp
../extern/imgui/imgui.cpp
../extern/imgui/imgui_draw.cpp
../extern/imgui/imgui_tables.cpp
../extern/imgui/imgui_widgets.cpp
../extern/imgui/imgui_demo.cpp
../extern/imgui/backends/imgui_impl_sdl.cpp
../extern/imgui/backends/imgui_impl_opengl3.cpp
../extern/ImGuiFileDialog/ImGuiFileDialog.cpp
${HEADER_LIST})

target_include_directories(yacvatlib PRIVATE 
../extern/imgui/ 
../extern/imgui/backends/ 
../extern/ImGuiFileDialog/ 
../extern/spdlog/include/
../extern/stb/ 
../extern/json/include/
../extern/fsm)

# Reach other dependencies using CmakeLists 
add_subdirectory(../extern/spdlog build_spdlog)
add_subdirectory(../extern/json build_json)

target_link_libraries(yacvatlib spdlog nlohmann_json -lpthread ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

# We need this directory, and users of our library will need it too
target_include_directories(yacvatlib PUBLIC ../include)

# All users of this library will need at least C++11
target_compile_options(yacvatlib PRIVATE -std=c++11 -g -Wall -Wformat)
target_compile_definitions(yacvatlib PRIVATE SPDLOG_COMPILED_LIB)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
