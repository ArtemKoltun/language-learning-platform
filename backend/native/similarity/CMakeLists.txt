add_library(llp_similarity STATIC
    src/similarity.cpp
)
target_include_directories(llp_similarity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(llp_similarity PUBLIC cxx_std_20)

option(BUILD_SIMILARITY_TESTS "Build similarity tests" ON)
if(BUILD_SIMILARITY_TESTS)
    add_executable(test_similarity tests/test_similarity.cpp)
    target_link_libraries(test_similarity PRIVATE llp_similarity)
    add_test(NAME test_similarity COMMAND test_similarity)
endif()