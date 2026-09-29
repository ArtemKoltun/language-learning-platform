pybind11_add_module(llp_core
    src/bindings.cpp
)

target_link_libraries(llp_core PRIVATE
    llp_memory
    llp_controller
)

target_include_directories(llp_core PRIVATE
    ${CMAKE_SOURCE_DIR}/memory/include
    ${CMAKE_SOURCE_DIR}/controller/include
)

# Не добавляем префикс "lib", чтобы получить llp_core.cpython-312-x86_64-linux-gnu.so
set_target_properties(llp_core PROPERTIES
    PREFIX ""
    OUTPUT_NAME "llp_core"
)