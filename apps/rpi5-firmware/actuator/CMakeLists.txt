cmake_minimum_required(VERSION 3.10)

project(actuator)

set(SOURCES
	CarActuator.hpp
	ActuatorCAN.cpp
    ActuatorCAN.hpp
    ActuatorController.cpp
    ActuatorController.hpp
    ActuatorDecorator.cpp
    ActuatorDecorator.hpp
    ActuatorKuksa.cpp
    ActuatorKuksa.hpp
	pythonActuator.cpp
)

add_library(actuator SHARED ${SOURCES})
set_target_properties(actuator PROPERTIES PREFIX "")
target_include_directories(actuator PUBLIC
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/python3.12/
	${SDK_PATH}/sysroots/aarch64-agl-linux/usr/include/boost/
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(actuator PRIVATE kuksa comms mode middleware purePursuit boost_python312 python3.12)

