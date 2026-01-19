cmake_minimum_required(VERSION 3.10)

project(RaspTests)

set(INSTALL_GTEST OFF)

# Build googletest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/googletest ${CMAKE_BINARY_DIR}/googletest)

include_directories(PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/googletest/include
)

include(GoogleTest)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(integration)
	gtest_discover_tests(integration_tests)
else()
	add_subdirectory(unit)
	gtest_discover_tests(unit_tests)
endif()
