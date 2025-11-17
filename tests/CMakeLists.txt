# Catch2 integration using FetchContent
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0   # or latest stable
)

FetchContent_MakeAvailable(Catch2)

# Build test executable
add_executable(test_math
    test_vec.cpp
)

# Include your math library headers
target_include_directories(test_math PRIVATE ../src)

# Link Catch2 library (with main included)
target_link_libraries(test_math PRIVATE Catch2::Catch2WithMain)

# Enable automatic test discovery
include(Catch)
catch_discover_tests(test_math)

