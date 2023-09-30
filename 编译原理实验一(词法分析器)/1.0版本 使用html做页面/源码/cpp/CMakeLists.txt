cmake_minimum_required(VERSION 3.25)
project(cLexer)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(cLexer main.cpp)
