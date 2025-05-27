cmake_minimum_required(VERSION 3.10)

project(soa LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

add_executable(test tests/test.cpp)
