cmake_minimum_required(VERSION 3.20)
project(objectLoader LANGUAGES CXX)

# If monorepo, add top-level matvec (relative)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/matvec/CMakeLists.txt")
  add_subdirectory(deps/matvec)
else()
  # assume parent will add matvec; or you can use find_package later
endif()

add_library(objloader STATIC src/obj_loader.cpp)
target_include_directories(objloader PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(objloader PUBLIC matvec)
