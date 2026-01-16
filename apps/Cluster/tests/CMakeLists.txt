cmake_minimum_required(VERSION 3.16)

project(QtAppTests LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qtAppTests
    cluster_test.cpp
    ../qtApp/src/generalInfo.cpp
    ../qtApp/src/systemInfo.cpp
)

target_link_libraries(qtAppTests
    PRIVATE
    Qt6::Test
    Qt6::Network
    Qt6::SerialBus
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(qtAppTests)
