project(pico_test C CXX ASM)
add_subdirectory("../googletest" "googletest")

enable_testing()
include(GoogleTest)

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(package_add_test_with_libraries TESTNAME FILES LIBRARIES)
    add_executable(${TESTNAME} ${FILES})
    include_directories(../src/ .)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})
    add_definitions(-DWITH_GOOGLE_TEST=1)
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

set(FILES test_main.cpp shiftreg_test.cpp firfilter_test.cpp ../src/fir_filter.cpp 
    resample_test.cpp uart_test.cpp tx_statemachine_test.cpp ../src/uart.cpp 
    ../src/message.cpp message_test.cpp ../src/uart_bit_detect_fast.cpp uart_bit_detect_test.cpp
     dc_block_test.cpp ../src/dcblock.cpp fifo_test.cpp)
set(LIBRARIES Threads::Threads)
include_directories(../src)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Wno-narrowing
        -fstack-protector-all
        -g -O0
        )

package_add_test_with_libraries(test_all "${FILES}" "${LIBRARIES}")
