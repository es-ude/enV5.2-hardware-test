add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/microphone)

add_executable(extension-board
        ${CMAKE_CURRENT_LIST_DIR}/ExtensionBoard.c)
target_include_directories(extension-board PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(extension-board
        pico_stdlib
        pico_bootrom
        hardware_i2c

        unity

        sensor_lib_adxl345b
        esp_lib
        network_lib

        microphone_lib
)
create_enV5_executable(extension-board)
