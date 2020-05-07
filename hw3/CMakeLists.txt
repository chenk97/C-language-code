cmake_minimum_required(VERSION 3.15)
project(hw3)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(hw3
        include/debug.h
        include/sfmm.h
        lib/sfutil.o
        src/main.c
        src/sfmm.c
        tests/sfmm_tests.c)
