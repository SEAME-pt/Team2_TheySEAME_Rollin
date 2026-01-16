cmake_minimum_required(VERSION 3.16)
project(Team2_Overall_Project LANGUAGES CXX)

#Global Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Find Dependencies (Once for everyone)
find_package(Qt6 REQUIRED COMPONENTS Test Network SerialBus)
find_package(GTest REQUIRED)

enable_testing()

# Add your scattered directories
# This will look for a CMakeLists.txt inside these folders
add_subdirectory(apps/Cluster/tests)
add_subdirectory(apps/rpi5-firmware)
# add_subdirectory(apps/OtherModule/tests)
