cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(BUILD_TESTS ON)
set(RPI_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
	mainRemote.cpp
)

set(SOURCES_LKA
	mainLka.cpp
)

add_compile_options(-fPIC)
add_link_options()

include_directories(
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/opencv4/ 
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/python3.12/
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/boost/
	.
)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/libkuksa ${CMAKE_BINARY_DIR}/libkuksa)
add_subdirectory(mode_manager)
add_subdirectory(middleware)
add_subdirectory(actuator)
add_subdirectory(purePursuit)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(comms)
	# Manual Car Executable
	add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode middleware kuksa actuator purePursuit)
	# Autonomous Car Executable
	add_executable(Lka ${SOURCES_LKA})
	target_link_libraries(Lka PUBLIC comms mode middleware kuksa actuator purePursuit
		opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_videoio)
	install(TARGETS CarControl Lka DESTINATION ${RPI_BIN_DIR})
else()
	if(BUILD_TESTS)
		add_subdirectory(tests)
	endif()
endif()
 
