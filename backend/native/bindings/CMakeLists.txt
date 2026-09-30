pybind11_add_module(llp_core
    src/bindings.cpp
)

target_link_libraries(llp_core PRIVATE
    llp_memory
    llp_controller
    llp_similarity
)

target_include_directories(llp_core PRIVATE
    ${CMAKE_SOURCE_DIR}/memory/include
    ${CMAKE_SOURCE_DIR}/controller/include
    ${CMAKE_SOURCE_DIR}/similarity/include
)

set_target_properties(llp_core PROPERTIES
    PREFIX ""
    OUTPUT_NAME "llp_core"
)