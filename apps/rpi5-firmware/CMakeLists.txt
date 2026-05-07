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
	SlidingWindow.hpp
	SlidingWindow.cpp
)

set(SOURCES_LKA
	mainLka.cpp
	Bev.hpp
	Bev.cpp
	Lka.cpp
	Lka.hpp
	Frame.hpp
	Frame.cpp
	SlidingWindow.hpp
	SlidingWindow.cpp
)

set(SOURCES_TSR
	mainTsr.cpp
	Tsr.hpp
	Tsr.cpp
)

# add_compile_options(-fsanitize=address -g)
# add_link_options(-fsanitize=address)

include_directories(${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/opencv4/ .)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/libkuksa ${CMAKE_BINARY_DIR}/libkuksa)
add_subdirectory(mode_manager)
add_subdirectory(middleware)
add_subdirectory(actuator)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(comms)
	add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode middleware kuksa actuator
		opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_videoio)
	add_executable(Lka ${SOURCES_LKA})
	target_link_libraries(Lka PUBLIC comms mode middleware kuksa actuator
		opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_videoio)
	add_executable(Tsr ${SOURCES_TSR})
	target_link_libraries(Tsr PUBLIC comms mode middleware kuksa actuator
		opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_videoio)
	install(TARGETS CarControl DESTINATION ${RPI_BIN_DIR})
	install(TARGETS Lka DESTINATION ${RPI_BIN_DIR})
	install(TARGETS Tsr DESTINATION ${RPI_BIN_DIR})
else()
	if(BUILD_TESTS)
		add_subdirectory(tests)
	endif()
endif()
 
