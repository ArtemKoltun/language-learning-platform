add_library(llp_controller STATIC
    src/controller.cpp
)
target_include_directories(llp_controller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(llp_controller PUBLIC llp_memory)
target_compile_features(llp_controller PUBLIC cxx_std_20)

option(BUILD_CONTROLLER_TESTS "Build controller tests" ON)
if(BUILD_CONTROLLER_TESTS)
    add_executable(test_controller tests/test_controller.cpp)
    target_link_libraries(test_controller PRIVATE llp_controller)
    add_test(NAME test_controller COMMAND test_controller)
endif()