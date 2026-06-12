cmake_minimum_required(VERSION 3.10)

project(comms)

set(SOURCES
	CAN.cpp
	CAN.hpp
	ICAN.hpp
)

add_library(comms STATIC ${SOURCES})
target_include_directories(comms PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(comms PRIVATE socketcan)

