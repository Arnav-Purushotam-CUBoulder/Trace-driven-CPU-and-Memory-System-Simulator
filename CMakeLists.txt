cmake_minimum_required(VERSION 3.20)

project(
    TraceDrivenCPUMemorySimulator
    VERSION 1.0.0
    DESCRIPTION "Trace-driven superscalar CPU and cache simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    simlib
    src/branch_predictor.cpp
    src/cache.cpp
    src/core_simulator.cpp
    src/stats.cpp
    src/trace_reader.cpp
)

target_include_directories(simlib PUBLIC include)

if(MSVC)
    target_compile_options(simlib PRIVATE /W4 /permissive-)
else()
    target_compile_options(simlib PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

add_executable(td_sim src/main.cpp)
target_link_libraries(td_sim PRIVATE simlib)

enable_testing()

add_executable(
    sim_tests
    tests/test_branch_predictor.cpp
    tests/test_cache.cpp
    tests/test_main.cpp
    tests/test_simulator.cpp
    tests/test_trace_reader.cpp
)

target_link_libraries(sim_tests PRIVATE simlib)
add_test(NAME sim_tests COMMAND sim_tests)
