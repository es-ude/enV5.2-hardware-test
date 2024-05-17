add_library(microphone_lib INTERFACE)
target_include_directories(microphone_lib INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)
target_sources(microphone_lib INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/Microphone.c)
target_link_libraries(microphone_lib INTERFACE
        common_lib
        sleep_interface
        gpio_interface
        hardware_adc)
