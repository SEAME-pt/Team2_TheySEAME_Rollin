cmake_minimum_required(VERSION 3.10)

project(RaspTests)

set(INSTALL_GTEST OFF)

# Build googletest
add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/googletest ${CMAKE_BINARY_DIR}/googletest)

include_directories(PUBLIC 
	${CMAKE_SOURCE_DIR}/../../libs/googletest/include
)

add_subdirectory(unit)

enable_testing()
