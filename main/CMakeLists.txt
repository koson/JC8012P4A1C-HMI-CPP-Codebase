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
        tests/test_json_basic.cpp    
        tests/test_svg_basic.cpp     

        # System Manager (OOP base for any ESP32-P4 HMI project)
        cpp_version/SystemManager.cpp
        
        # Demo Manager (OOP demo gallery system)
        cpp_version/DemoManager.cpp
        
        # Menu Demo Application (Demo gallery app)
        cpp_version/MenuDemoApplication.cpp
        
        # File Manager Application (Web-based file upload/download)
        cpp_version/FileManagerApplication.cpp
        
        # File Viewer UI (LVGL UI for FileManager - Option 1.5)
        cpp_version/FileViewerUI.cpp

        # HMI Navigator (Splash → Home → Library, uses lvgl_cpp_v9 ScreenManager)
        cpp_version/HMINavigator.cpp

        # Lesson Player (multi-page interactive lesson engine)
        cpp_version/LessonPlayer.cpp

        # UartBridge: command TX/RX to STM32H7 measurement module
        cpp_version/UartBridge.cpp

        # VerificationEngine: truth-table-based IC verification over UartBridge
        cpp_version/VerificationEngine.cpp
        
        # LabHappy SVG Path Renderer Demo
        cpp_version/LabHappySvgDemo.cpp
        
        # LabBuddy JSON Circuit Viewer Demo
        cpp_version/LabBuddyJsonDemo.cpp
        
        examples/colorwheel_demo.cpp

        # All C++ OOP widget implementations (core library)
        ${CPP_SOURCES}
        
        # Note: Example demos disabled to reduce binary size
        # ${CPP_VERSION_SOURCES}
        # ${EXAMPLE_SOURCES}
        # ${EXAMPLE_C_SOURCES}
        
        # Note: VPI HMI Components disabled in this branch
        # uart_receiver.c
        # vpi_data_manager.c
        # img_pressure_needle.c
        
        # LVGL demos (optional - can be removed for smaller binary)
        # ${LV_DEMOS_SOURCES}
        
    INCLUDE_DIRS
        ../components/lvgl_cpp_v9/include
        .
        include
        tests
        examples
        ${LV_DEMO_DIR}
        
    REQUIRES
        unity
        esp32_p4_function_ev_board  # BSP with SD card support
        fatfs  # For LVStorage component
        svg_renderer  # LabHappy SVG Path Renderer
        json_renderer  # LabBuddy JSON Circuit Viewer
        esp_timer  # High-resolution timer for perf tracking
        font_thai  # TH Niramit Thai+Latin fonts for LessonPlayer
        usb_cdc_host  # USB CDC-ACM Host transport to STM32H7 (replaces UART bridge)
        
    PRIV_REQUIRES
        bsp_extra  # BSP extra functions
        esp_wifi_remote  # WiFi Remote Library
        esp_wifi  # WiFi component
        esp_http_server  # HTTP server
        nvs_flash  # NVS storage for WiFi
        json  # cJSON parsing for LessonPlayer
        driver  # GPIO/peripheral driver (still needed by BSP and other components)
        # NOTE: 'usb' is intentionally NOT listed here — usb_cdc_host idf_component.yml
        # declares it as a managed dependency.  Listing it explicitly causes USB host
        # system-init functions to run before LVGL allocates its SPIRAM framebuffers,
        # resulting in esp_cache_msync null-pointer crashes on display flush.
)
