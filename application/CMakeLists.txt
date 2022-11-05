# Dependencies
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})

add_executable(yacvat main.cpp)

target_include_directories(yacvat PRIVATE 
../extern/imgui/ 
../extern/imgui/backends/ 
../extern/ImGuiFileDialog/ 
../extern/spdlog/include/
../extern/stb/ 
../extern/json/include/
../extern/fsm)

target_compile_features(yacvat PRIVATE cxx_std_17)
target_link_libraries(yacvat PRIVATE yacvatlib -lpthread ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

# compile options
target_compile_options(yacvat PRIVATE -std=c++11 -g -Wall -Wformat)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)