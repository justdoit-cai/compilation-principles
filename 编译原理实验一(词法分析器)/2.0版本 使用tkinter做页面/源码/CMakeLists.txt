cmake_minimum_required(VERSION 3.25)
project(demo01)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(demo01 main.cpp)
