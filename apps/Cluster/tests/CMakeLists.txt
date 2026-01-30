cmake_minimum_required(VERSION 3.16)

project(QtAppTests LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Network SerialBus)
find_package(GTest REQUIRED)
find_package(gRPC CONFIG REQUIRED)
add_executable(qtAppTests
    cluster_test.cpp
    ../qtApp/src/generalInfo.cpp
    ../qtApp/src/systemInfo.cpp
)

target_include_directories(qtAppTests PRIVATE
        "../qtApp/src"
        "../../middleware"
)

target_link_libraries(qtAppTests
    PRIVATE
    Qt6::Test
    Qt6::Network
    Qt6::SerialBus
    GTest::gtest_main
    gRPC::grpc++
)

include(GoogleTest)
gtest_discover_tests(qtAppTests)
