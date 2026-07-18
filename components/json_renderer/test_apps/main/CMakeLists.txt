idf_component_register(
    SRCS "test_json_renderer_main.cpp"
    INCLUDE_DIRS "."
    REQUIRES 
        unity
        json_renderer
        svg_renderer
        lvgl_cpp_v9
)

# Enable C++17 for std::string, std::vector, etc.
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
