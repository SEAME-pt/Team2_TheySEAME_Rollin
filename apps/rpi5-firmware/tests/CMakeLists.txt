cmake_minimum_required(VERSION 3.10)

project(RaspTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
	RemoteControlTest.cpp
	CANTest.cpp
)

add_custom_target(
	googletest
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}../../../libs/googletest"
	COMMAND ${CMAKE_COMMAND} -S "." -B "./build"
	COMMAND ${CMAKE_COMMAND} --build "./build"
)

add_executable(RaspTests ${SOURCES})
add_dependencies(RaspTests googletest)
target_include_directories(RaspTests PUBLIC
	../../../libs/googletest/googletest/include
	../../../libs/googletest/googlemock/include
)
target_link_libraries(RaspTests
    PRIVATE
	${CMAKE_SOURCE_DIR}/../../libs/googletest/build/lib/libgmock.a
	${CMAKE_SOURCE_DIR}/../../libs/googletest/build/lib/libgtest.a
	${CMAKE_SOURCE_DIR}/../../libs/googletest/build/lib/libgtest_main.a
	comms
	mode
	socketcan
)

include(GoogleTest)
