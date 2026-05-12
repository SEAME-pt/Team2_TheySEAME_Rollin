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
)

add_library(actuator STATIC ${SOURCES})
target_include_directories(actuator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(actuator PRIVATE kuksa comms mode middleware)

