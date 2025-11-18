cmake_minimum_required(VERSION 3.20)
project(ObjectLoader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option: build matlib tests?
option(MATLIB_BUILD_TESTS "Build tests in deps/matlib" OFF)

# Option: build object loader tests?
option(BUILD_TESTING "Build object loader tests" ON)

# Add matlib dependency
add_subdirectory(deps/matlib)

# Build objloader library
add_library(objloader
    src/obj_loader.cpp
)

target_include_directories(objloader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(objloader PUBLIC matlib)

# Tests
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
