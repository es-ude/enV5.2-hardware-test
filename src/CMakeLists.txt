add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/microphone)

add_executable(extension-board
        ${CMAKE_CURRENT_LIST_DIR}/ExtensionBoard.c)
target_link_libraries(extension-board
        pico_stdlib
        pico_bootrom
        hardware_i2c

        unity

        common_lib
        enV5_hwc
        sensor_lib_adxl345b
        esp_lib
        network_lib

        microphone_lib
)
create_enV5_executable(extension-board)

add_executable(main-board-basic
        ${CMAKE_CURRENT_LIST_DIR}/MainBoard_Basic.c)
target_link_libraries(main-board-basic
        pico_stdlib
        pico_bootrom
        hardware_i2c
        hardware_spi

        unity

        common_lib
        flash_lib
        middleware_lib
        enV5_hwc
        sensor_lib_pac193x
)
create_enV5_executable(main-board-basic)

add_executable(main-board-full
        ${CMAKE_CURRENT_LIST_DIR}/MainBoard_Full.c)
target_link_libraries(main-board-full
        pico_stdlib
        pico_bootrom
        hardware_i2c

        unity

        sensor_lib_adxl345b
        esp_lib
        network_lib

        microphone_lib
)
create_enV5_executable(main-board-full)
