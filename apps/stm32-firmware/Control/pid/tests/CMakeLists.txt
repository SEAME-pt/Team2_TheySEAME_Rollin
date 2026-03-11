cmake_minimum_required(VERSION 3.10)

project(pid_tests C)

add_executable(test_pid
    tests.c
    ../pid.c
)

target_include_directories(test_pid
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_pid
    PRIVATE
    m
)

enable_testing()
add_test(NAME test_pid COMMAND test_pid)