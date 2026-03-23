cmake_minimum_required(VERSION 3.14)
project(winterChallenge2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Exécutable principal (bot)
add_executable(bot src/main.cpp)
target_include_directories(bot PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Google Test via FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Tests unitaires Game
enable_testing()
add_executable(Game_test tests/Game_test.cpp tests/Game_scenario_30x16_test.cpp tests/Game_scenario_5x5_test.cpp tests/Game_scenario_26x14_test.cpp tests/Game_scenario_36x20_test.cpp)
target_include_directories(Game_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(Game_test GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Game_test)
