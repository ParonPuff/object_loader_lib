cmake_minimum_required(VERSION 3.20)
project(ObjectLoader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option: build matlib tests?
option(MATLIB_BUILD_TESTS "Build tests in deps/matlib" OFF)

# Option: build object loader tests?
option(BUILD_TESTING "Build object loader tests" ON)

# Add the dependency library (header-only)
add_subdirectory(deps/matlib)

# Add the obj loader library
add_subdirectory(src)

# Add tests only for the object loader
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
