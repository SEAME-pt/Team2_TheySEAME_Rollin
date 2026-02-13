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

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Positioning Test)

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc++ grpc)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)

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
)

target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/val.grpc.pb.cc
<<<<<<< HEAD
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/KuksaLib.cpp
=======
>>>>>>> 92140775 (Release/1.0.0 (#335))
)

add_executable(qtAppExec
    main.cpp
    qml.qrc
)

target_include_directories(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../middleware
)
target_include_directories(qtAppExec PRIVATE
  ${CMAKE_SOURCE_DIR}/../../middleware
)
target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/../../middleware/kuksa/val/v2/val.grpc.pb.cc
)

target_link_libraries(qtAppExec
    PRIVATE 
    Qt6::Gui
    Qt6::Qml
    Qt6::Test
    Qt6::Network
    Qt6::Positioning
    PkgConfig::GRPC
    PkgConfig::PROTOBUF
    qtAppLib
)


######################### Tests #########################

enable_testing()

