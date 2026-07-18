idf_component_register(
    SRCS "test_json_parser.cpp"
    INCLUDE_DIRS "."
    REQUIRES 
        unity
        json_renderer
        svg_renderer
        lvgl_cpp_v9
)

# Add test data directory
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    TEST_DATA_DIR="/sdcard/test_data"
)

# Enable C++17 for std::string, std::vector, etc.
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
