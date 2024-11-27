cmake_minimum_required(VERSION 3.30)
project(vector)

set(CMAKE_CXX_STANDARD 23)

add_executable(vector main.cpp
        containers/vector/vector.hpp
        containers/vector/vectorBool.hpp
)
