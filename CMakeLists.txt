cmake_minimum_required(VERSION 3.21.1)
project(symxx)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(tests)

add_executable(symxx example/symxx.cpp)
