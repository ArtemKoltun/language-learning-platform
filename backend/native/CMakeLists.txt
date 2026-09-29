cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0148)
    cmake_policy(SET CMP0148 OLD)
endif()

project(llp_core
    VERSION 0.1.0
    DESCRIPTION "Language Learning Platform — C++ core"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native)
    # NDEBUG намеренно НЕ определяем, чтобы assert в тестах работал
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

option(BUILD_TESTS "Build C++ tests" ON)

# enable_testing ДО add_subdirectory — иначе add_test игнорируется
if(BUILD_TESTS)
    enable_testing()
endif()

find_package(pybind11 CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_subdirectory(memory)
add_subdirectory(controller)
add_subdirectory(bindings)
# add_subdirectory(similarity)
# add_subdirectory(metrics)
# add_subdirectory(simulator)

message(STATUS "")
message(STATUS "=== llp_core configuration ===")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Python:         ${Python3_EXECUTABLE}")
message(STATUS "  pybind11:       ${pybind11_VERSION}")
message(STATUS "  Tests:          ${BUILD_TESTS}")
message(STATUS "===============================")
message(STATUS "")