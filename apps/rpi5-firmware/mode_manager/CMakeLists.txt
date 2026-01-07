add_library(mode STATIC remote/RemoteControl.cpp remote/Evdev.cpp)
target_include_directories(mode PRIVATE)
