cmake_minimum_required(VERSION 3.10)

project(mode)

set(SOURCES
	remote/RemoteControl.cpp
	remote/Evdev.cpp
)

add_library(mode STATIC ${SOURCES})
