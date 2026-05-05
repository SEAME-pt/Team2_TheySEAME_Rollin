cmake_minimum_required(VERSION 3.10)

project(mode)

set(SOURCES
	remote/RemoteControl.cpp
	remote/Evdev.cpp
)

add_library(mode STATIC ${SOURCES})
target_include_directories(mode PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/remote
)
target_link_libraries(mode PRIVATE middleware)
