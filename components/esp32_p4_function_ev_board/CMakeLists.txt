
idf_component_register(
    SRCS "esp32_p4_function_ev_board.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver esp_lcd_jd9365 esp_lcd_touch_gsl3680
    PRIV_REQUIRES esp_lcd usb spiffs fatfs
)
