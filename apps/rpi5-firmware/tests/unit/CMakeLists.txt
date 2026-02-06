cmake_minimum_required(VERSION 3.10)

project(unit_tests)

set(SOURCES
	RemoteControlTests.cpp
	SubjectTests.cpp
)

add_executable(unit_tests ${SOURCES})
target_include_directories(unit_tests PUBLIC ../../middleware)
target_link_libraries(unit_tests
	PRIVATE
	gtest
	gtest_main
	gmock
	mode
	middleware
)
install(TARGETS unit_tests DESTINATION ${RPI_BIN_DIR})
