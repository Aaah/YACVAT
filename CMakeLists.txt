cmake_minimum_required(VERSION 3.14...3.24)

# Project name and a few useful settings. Other commands can pick up the results
project(
    Yacvat 
    VERSION 1.2.9
    DESCRIPTION "Yet Another Computer Vision Annotation Tool"
    LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(application)
