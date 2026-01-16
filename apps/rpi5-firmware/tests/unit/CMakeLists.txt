cmake_minimum_required(VERSION 3.10)

project(unit_tests)

find_package(GTest REQUIRED)

set(SOURCES
	RemoteControlTest.cpp
)

add_executable(unit_tests ${SOURCES})
target_include_directories(unit_tests PUBLIC .)
target_link_libraries(unit_tests
		PRIVATE
		GTest::gtest
		GTest::gtest_main
		GTest::gmock
		mode
)
install(TARGETS unit_tests DESTINATION ${RPI_BIN_DIR})
