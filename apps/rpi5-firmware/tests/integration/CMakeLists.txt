cmake_minimum_required(VERSION 3.10)

project(integration_tests)

set(SOURCES
	CANTest.cpp
)

add_executable(integration_tests ${SOURCES})
target_link_libraries(integration_tests
	PRIVATE
	gtest
	gtest_main
	comms
	socketcan
)
install(TARGETS integration_tests DESTINATION ${RPI_BIN_DIR})
