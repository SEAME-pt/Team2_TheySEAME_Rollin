cmake_minimum_required(VERSION 3.10)

project(comms)

set(SOURCES
	CAN.cpp
)

add_library(comms STATIC ${SOURCES})
