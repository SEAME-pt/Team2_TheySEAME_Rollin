cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(BUILD_TESTS ON)
set(RPI_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES main.cpp ICar.hpp CarCAN.hpp CarCAN.cpp CarKuksa.hpp CarKuksa.cpp
	./kuksa/val/v2/val.grpc.pb.cc ./kuksa/val/v2/types.pb.cc ./kuksa/val/v2/val.pb.cc ./kuksa/val/v2/KuksaLib.cpp)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(PUBLIC comms mode_manager/remote middleware ./kuksa/val/v2 .)

add_subdirectory(mode_manager)
add_subdirectory(middleware)

if(CMAKE_CROSSCOMPILING)
	add_subdirectory(comms)
	add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode socketcan middleware protobuf::libprotobuf
		grpc gpr grpc++ PRIVATE absl::log_internal_check_op absl::cord)
	install(TARGETS CarControl DESTINATION ${RPI_BIN_DIR})
else()
	if(BUILD_TESTS)
		add_subdirectory(tests)
	endif()
endif()
 
