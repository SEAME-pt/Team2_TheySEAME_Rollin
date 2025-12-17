set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qtAppTests
    cluster_test.cpp
    ../src/generalInfo.hpp
    ../src/systemInfo.hpp
)

target_link_libraries(qtAppTests
    PRIVATE
    qtAppLib
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::SerialBus
    Qt6::DBus
    Qt6::Test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(qtAppTests)
