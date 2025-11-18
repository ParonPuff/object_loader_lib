include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_obj test_obj.cpp)

target_link_libraries(test_obj PUBLIC objloader Catch2::Catch2WithMain)

# Pass the fixtures path to tests
target_compile_definitions(test_obj PRIVATE
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

include(Catch)
catch_discover_tests(test_obj)
