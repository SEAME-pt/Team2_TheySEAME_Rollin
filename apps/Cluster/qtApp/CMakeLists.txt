cmake_minimum_required(VERSION 3.16)
project(Cluster LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "/build/qt6/pi" CACHE PATH "Qt6 installation path")
endif()

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Qml Quick Quick3D QuickControls2 Positioning Test)

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc++_unsecure grpc_unsecure)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
pkg_check_modules(SYSTEMD IMPORTED_TARGET libsystemd)
pkg_check_modules(UPB IMPORTED_TARGET upb upb_collections_lib upb_json_lib upb_textformat_lib)
pkg_check_modules(ADDRESS_SORTING IMPORTED_TARGET address_sorting)

add_library(qtAppLib
    src/generalInfo.cpp
    src/systemInfo.cpp
)

target_link_libraries(qtAppLib
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Test
        Qt6::Network
        Qt6::Positioning
        PkgConfig::GRPC
        PkgConfig::PROTOBUF
        PkgConfig::ZLIB
        cares
        upb
        upb_collections_lib
        upb_json_lib
        upb_textformat_lib
        address_sorting
)

if(SYSTEMD_FOUND)
    target_link_libraries(qtAppLib PRIVATE PkgConfig::SYSTEMD)
endif()

target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/val.grpc.pb.cc
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/KuksaLib.cpp
  
)

add_executable(qtAppExec
    main.cpp
    qml.qrc
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/qml/3d-assets/carro/LowPolyFiatUNO.qml
  COMMAND ${CMAKE_COMMAND} -E echo "Generating LowPolyFiatUNO.mesh via balsam"
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen balsam ${CMAKE_CURRENT_SOURCE_DIR}/qml/3d-assets/carro/LowPolyFiatUNO.obj --outputPath ${CMAKE_CURRENT_SOURCE_DIR}/qml/3d-assets/carro
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qml/3d-assets/carro/LowPolyFiatUNO.obj
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_target(car_mesh ALL
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qml/3d-assets/carro/LowPolyFiatUNO.qml
)

add_dependencies(qtAppExec car_mesh)

target_include_directories(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2
  ${CMAKE_BINARY_DIR}
)
target_include_directories(qtAppExec PRIVATE
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2
)
target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/../../../libs/libkuksa/build/kuksa/val/v2/val.grpc.pb.cc
)

target_link_libraries(qtAppExec
    PRIVATE 
    Qt6::Gui
    Qt6::Qml
  Qt6::Quick3D
    Qt6::Test
    Qt6::Network
    Qt6::Positioning
    PkgConfig::GRPC
    PkgConfig::PROTOBUF
    PkgConfig::ZLIB
    cares
    upb
    upb_collections_lib
    upb_json_lib
    upb_textformat_lib
    address_sorting
    qtAppLib
)

if(SYSTEMD_FOUND)
    target_link_libraries(qtAppExec PRIVATE PkgConfig::SYSTEMD)
endif()


######################### Tests #########################

enable_testing()

