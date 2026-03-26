set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -fPIC -O3 -DNDEBUG" CACHE STRING "" FORCE)
