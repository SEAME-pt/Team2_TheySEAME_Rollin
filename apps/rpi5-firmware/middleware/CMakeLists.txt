cmake_minimum_required(VERSION 3.10)

project(middleware)

set(SOURCES
	Subject.cpp
	Subject.hpp
	Observer.hpp
)

add_library(middleware STATIC ${SOURCES})
target_include_directories(middleware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
