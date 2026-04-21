set(LV_DEMO_DIR ../managed_components/lvgl__lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

# Enable C++ support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect source files from organized folders
file(GLOB C_VERSION_SOURCES "c_version/*.c")
file(GLOB CPP_VERSION_SOURCES "cpp_version/*.cpp")
file(GLOB CORE_SOURCES "../components/lvgl_cpp_v9/src/*.cpp")
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")  # Phase 2: Widget testing examples
file(GLOB EXAMPLE_C_SOURCES "examples/*.c")  # C source files (like needle image)

# Ensure new demos are picked up even without re-running glob
list(APPEND EXAMPLE_SOURCES
    examples/color_demo.cpp
    examples/multitouch_demo.cpp
    examples/canvas_demo.cpp
    examples/style_demo.cpp
    examples/animation_demo.cpp
    examples/screen_demo.cpp
    examples/screen_manager_demo.cpp
    examples/layout_demo.cpp
)

# Collect all CPP source files (core library)
set(CPP_SOURCES ${CORE_SOURCES})

# Debug: Print found files
message(STATUS "CPP Sources: ${CPP_SOURCES}")
message(STATUS "Example Sources: ${EXAMPLE_SOURCES}")
message(STATUS "Example C Sources: ${EXAMPLE_C_SOURCES}")

idf_component_register(
    SRCS 
        # Main entry point (C++ - Refactored with clean OOP architecture)
        main.cpp
        # main_menu.cpp  # DISABLED - requires all examples to be compiled
        
        # System Manager (OOP base for any ESP32-P4 HMI project)
        cpp_version/SystemManager.cpp
        
        # VPI Application (VPI-specific app logic)
        cpp_version/VPIApplication.cpp
        
        # All C++ OOP widget implementations (core library)
        ${CPP_SOURCES}
        
        # VPI HMI Components (C sources)
        uart_receiver.c
        vpi_data_manager.c
        # ui_vpi_hmi.c  # OLD: Pure C version (replaced by VPIHMIUI.cpp - C++ OOP)
        img_pressure_needle.c
        # sd_card_test.c  # REPLACED by DataLogger (CSV logging)
        
        # C++ examples (Phase 1) - DISABLED to reduce binary size
        # ${CPP_VERSION_SOURCES}
        
        # Phase 2: Widget testing examples - DISABLED to reduce binary size
        # ${EXAMPLE_SOURCES}
        
        # C sources in examples (needle image, etc.) - DISABLED to reduce binary size
        # ${EXAMPLE_C_SOURCES}
        
        # LVGL demos (optional - can be removed for smaller binary)
        # ${LV_DEMOS_SOURCES}
        
    INCLUDE_DIRS
        ../components/lvgl_cpp_v9/include
        .
        include
        examples
        ${LV_DEMO_DIR}
        
    REQUIRES
        vpi_document  # VPI Document & DataLogger (CSV logging)
        esp32_p4_function_ev_board  # BSP with SD card support
        
    PRIV_REQUIRES
        nvs_flash  # NVS flash storage
        json  # cJSON library
        bsp_extra  # BSP extra functions
)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
        # -DLV_USE_DEMO_MUSIC  # DISABLED to reduce binary size
)
