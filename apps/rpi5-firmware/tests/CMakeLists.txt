cmake_minimum_required(VERSION 3.10)

project(RaspTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build googletest
add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/googletest ${CMAKE_BINARY_DIR}/googletest)

include_directories(PUBLIC 
	${CMAKE_SOURCE_DIR}/../../libs/googletest/include
)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(integration)
else()
	add_subdirectory(unit)
endif()
