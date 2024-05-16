# add your own executable
add_executable(extension-board
        ${CMAKE_CURRENT_LIST_DIR}/ExtensionBoard.c)
target_include_directories(extension-board PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(extension-board
        pico_stdlib
        freeRtosUtils
        esp_lib
        network_lib
        http_lib
        uart_interface)
create_enV5_executable(extension-board)
