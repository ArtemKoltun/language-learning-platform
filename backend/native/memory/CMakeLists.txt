add_library(llp_memory STATIC
    src/memory.cpp
)
target_include_directories(llp_memory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(llp_memory PUBLIC cxx_std_20)

# Тесты
option(BUILD_MEMORY_TESTS "Build memory tests" ON)
if(BUILD_MEMORY_TESTS)
    add_executable(test_memory tests/test_memory.cpp)
    target_link_libraries(test_memory PRIVATE llp_memory)
    add_test(NAME test_memory COMMAND test_memory)
endif()