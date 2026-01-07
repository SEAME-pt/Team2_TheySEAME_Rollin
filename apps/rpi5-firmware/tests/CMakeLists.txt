cmake_minimum_required(VERSION 3.28)

project(RaspTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(RaspTests
	RemoteControlTest.cpp
)

# Link with GoogleTest and your library
# gtest_main already provides the main() entry point
target_link_libraries(RaspTests
    PRIVATE
	gtest_main
	gtest
	gmock
	comms
	mode
)
target_include_directories(RaspTests PUBLIC mode_manager/remote)

include(GoogleTest)
gtest_discover_tests(RaspTests)
