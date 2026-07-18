# Test application for svg_renderer component
cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF unit test components
set(EXTRA_COMPONENT_DIRS 
    "$ENV{IDF_PATH}/tools/unit-test-app/components"
    "${CMAKE_CURRENT_LIST_DIR}/../.."
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(test_svg_renderer)
