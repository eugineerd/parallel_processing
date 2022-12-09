cmake_minimum_required(VERSION 3.25)

project(pthreads_projects)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(tokenize tokenize.c)

add_subdirectory(lab1)