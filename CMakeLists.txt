cmake_minimum_required(VERSION 3.24)
project(symxx)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(include/symxx)

add_executable(symxx example/symxx.cpp)
