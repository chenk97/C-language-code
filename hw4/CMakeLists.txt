cmake_minimum_required(VERSION 3.15)
project(hw4)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(hw4
        include/debug.h
        include/helper.h
        include/jobber.h
        include/sf_readline.h
        include/task.h
        lib/sf_event.o
        src/helper.c
        src/jobs.c
        src/main.c
        src/sf_readline.c
        src/task.c
        tests/hw4_tests.c)
