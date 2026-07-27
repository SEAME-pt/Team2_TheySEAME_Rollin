cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(BUILD_TESTS ON)
set(RPI_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(SOURCES
	main.cpp
	Tsr.cpp
	Tsr.hpp
	HazardDetector.hpp
	HazardDetector.cpp
	Utils.hpp
)

set(SOURCES_TSR
	mainTsr.cpp
	Tsr.hpp
	Tsr.cpp
	HazardDetector.hpp
	HazardDetector.cpp
	Utils.hpp
)

include_directories(
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/python3.12/
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/boost/
	.
)

add_compile_options(-fPIC)
# add_link_options(-fsanitize=address)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/libkuksa ${CMAKE_BINARY_DIR}/libkuksa)
add_subdirectory(mode_manager)
add_subdirectory(middleware)
add_subdirectory(actuator)
add_subdirectory(lkaControl)
find_package(PahoMqttCpp REQUIRED)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(comms)
	# Manual Car Executable
	add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode middleware kuksa actuator lkaControl jsoncpp PahoMqttCpp::paho-mqttpp3)
	add_executable(Tsr ${SOURCES_TSR})
	target_link_libraries(Tsr PUBLIC comms mode middleware kuksa actuator lkaControl jsoncpp PahoMqttCpp::paho-mqttpp3)
	install(TARGETS CarControl DESTINATION ${RPI_BIN_DIR})
	install(TARGETS Tsr DESTINATION ${RPI_BIN_DIR})
else()
	if(BUILD_TESTS)
		add_subdirectory(tests)
	endif()
endif()
 
