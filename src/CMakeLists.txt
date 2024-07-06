
add_compile_definitions(
        S15 # uncomment for enV5 with Spartan-7 S15
        #S50 # uncomment for enV5 with Spartan-7 S50

        URL_BASE="127.0.0.1:5000" # replace with the URL shown by Flask after starting the `BinFileUpload.py` script
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/microphone)

link_libraries(
        pico_stdlib
        pico_bootrom
        hardware_i2c

        unity

        sensor_lib_adxl345b
        esp_lib
        network_lib
        common_lib
        flash_lib
        middleware_lib
        enV5_hw_configuration
        enV5_hw_controller
        sensor_lib_pac193x
        fpga_configuration_handler

        microphone_lib
)

add_executable(extension-board
        ${CMAKE_CURRENT_LIST_DIR}/HardwareTest.c)
target_compile_definitions(extension-board
        PRIVATE EXTENSION_BOARD
)
create_enV5_executable(extension-board)

add_executable(main-board-standalone
        ${CMAKE_CURRENT_LIST_DIR}/HardwareTest.c)
target_compile_definitions(main-board-standalone
        PRIVATE MAIN_BOARD
)
create_enV5_executable(main-board-standalone)

add_executable(main-board-full
        ${CMAKE_CURRENT_LIST_DIR}/HardwareTest.c)
target_compile_definitions(main-board-full
        PRIVATE MAIN_BOARD
        PRIVATE EXTENSION_BOARD
)
create_enV5_executable(main-board-full)
