cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(BUILD_TESTS ON)
set(RPI_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
	main.cpp
	Bev.hpp
	Bev.cpp
	Lka.cpp
	Lka.hpp
	Frame.hpp
	Frame.cpp
)

include_directories(comms mode_manager/remote middleware /home/rumachad/Seame/agl-app/sdk/sysroots/aarch64-agl-linux/usr/include/opencv4/ .)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/libkuksa ${CMAKE_BINARY_DIR}/libkuksa)
add_subdirectory(mode_manager)
add_subdirectory(middleware)
add_subdirectory(actuator)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(comms)
	add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode socketcan middleware kuksa actuator
		opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_videoio)
	install(TARGETS CarControl DESTINATION ${RPI_BIN_DIR})
else()
	if(BUILD_TESTS)
		add_subdirectory(tests)
	endif()
endif()
 
