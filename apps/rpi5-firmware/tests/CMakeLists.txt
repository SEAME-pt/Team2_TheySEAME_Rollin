cmake_minimum_required(VERSION 3.28)

project(RaspTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
	RemoteControlTest.cpp
)

add_executable(RaspTests ${SOURCES})
target_link_libraries(RaspTests
    PRIVATE
	gtest_main
	gtest
	gmock
	comms
	mode
)

include(GoogleTest)
gtest_discover_tests(RaspTests)
