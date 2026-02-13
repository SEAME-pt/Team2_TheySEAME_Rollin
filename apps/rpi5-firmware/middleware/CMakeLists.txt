cmake_minimum_required(VERSION 3.10)

project(middleware)

set(SOURCES
	Subject.cpp
	Subject.hpp
	Observer.hpp
)

add_library(middleware STATIC ${SOURCES})
