idf_component_register(
    SRCS "test_svg_path_parser.cpp"
    INCLUDE_DIRS "."
    REQUIRES 
        unity
        svg_renderer
        lvgl_cpp_v9
)

# Enable C++17
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
