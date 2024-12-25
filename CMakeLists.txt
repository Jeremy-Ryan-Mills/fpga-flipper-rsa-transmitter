target_sources(app_my_project PRIVATE my_project.c)

target_link_libraries(app_my_project PRIVATE
        furi
        furi_hal
        furi_hal_subghz
        furi_hal_gpio
)