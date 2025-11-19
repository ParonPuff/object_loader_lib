cmake_minimum_required(VERSION 3.20)
project(objectLoader LANGUAGES CXX)

# ---- Dependency: matvec ----
# Option allows parent project to disable bundled matvec
option(OBJECTLOADER_USE_INTERNAL_MATVEC
       "Use vendored matvec located in deps/matvec" ON)

# If parent already defines matvec, prefer that automatically
if (TARGET matvec)
    set(OBJECTLOADER_USE_INTERNAL_MATVEC OFF)
endif()

if (OBJECTLOADER_USE_INTERNAL_MATVEC)
    # Use bundled dependency
    message(STATUS "[objectLoader] Using internal matvec (deps/matvec)")
    add_subdirectory(deps/matvec)
else()
    message(STATUS "[objectLoader] Using external matvec provided by parent")
endif()

# ---- Main library ----
add_library(objloader STATIC src/obj_loader.cpp)

target_include_directories(objloader PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link to whichever matvec we selected
target_link_libraries(objloader PUBLIC matvec)
