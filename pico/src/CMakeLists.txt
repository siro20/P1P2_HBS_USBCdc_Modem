file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(p1p2 ${SRC_FILES})
pico_set_binary_type(p1p2 copy_to_ram)
pico_generate_pio_header(p1p2 ${CMAKE_CURRENT_LIST_DIR}/p1p2_uart_tx.pio)
pico_generate_pio_header(p1p2 ${CMAKE_CURRENT_LIST_DIR}/twos_complement.pio)

target_link_libraries(p1p2 pico_stdlib pico_multicore hardware_adc hardware_dma hardware_pio)
# The compiler must make no assumptions about the build.
set_target_properties(p1p2 PROPERTIES LINK_FLAGS "-nostdlib++")
	
pico_enable_stdio_usb(p1p2 1)
pico_enable_stdio_uart(p1p2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(p1p2)

# Link with newlib-nano.
target_link_options(p1p2 PRIVATE
    -Wl,--print-memory-usage --specs=nano.specs
)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics -fno-use-cxa-atexit -fno-rtti")

