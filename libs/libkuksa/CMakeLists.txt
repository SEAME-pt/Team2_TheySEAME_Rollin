cmake_minimum_required(VERSION 3.28)

project(libkuksa)

set(SOURCES KuksaLib.cpp KuksaLib.hpp)
set(PROTO_CPP_SRCS ./kuksa/val/v2/val.proto ./kuksa/val/v2/types.proto)
set(PROTO_GRPC_SRCS ./kuksa/val/v2/val.proto)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

add_library(kuksa STATIC ${SOURCES}) 

target_include_directories(kuksa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/kuksa/val/v2 
target_link_libraries(
	kuksa
	PUBLIC protobuf::libprotobuf grpc grpc++ gpr
	PRIVATE absl::log_internal_check_op absl::cord
)

protobuf_generate(
	TARGET kuksa
	LANGUAGE cpp
	PROTOS ${PROTO_CPP_SRCS}
)

protobuf_generate(
	TARGET kuksa
	LANGUAGE grpc
	GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
	PLUGIN protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin
	PROTOS ${PROTO_GRPC_SRCS}
)

