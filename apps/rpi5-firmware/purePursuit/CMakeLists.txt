cmake_minimum_required(VERSION 3.10)

project(purePursuit)

set(SOURCES
	PurePursuit.cpp
	PurePursuit.hpp
	pythonPurePursuit.cpp
)

add_library(purePursuit SHARED ${SOURCES})
set_target_properties(purePursuit PROPERTIES PREFIX "")
target_include_directories(
	purePursuit
	PUBLIC
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/opencv4/ 
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/python3.12/
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/boost/
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(purePursuit PRIVATE middleware
	opencv_core boost_python312 python3.12
)

