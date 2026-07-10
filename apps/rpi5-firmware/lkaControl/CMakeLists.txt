cmake_minimum_required(VERSION 3.10)

project(lkaControl)

set(SOURCES
	LkaControl.cpp
	LkaControl.hpp
	pythonLkaControl.cpp
)

add_library(lkaControl SHARED ${SOURCES})
set_target_properties(lkaControl PROPERTIES PREFIX "")
target_include_directories(
	lkaControl
	PUBLIC
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/opencv4/ 
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/python3.12/
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/boost/
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(lkaControl PRIVATE middleware
	opencv_core boost_python312 python3.12
)

