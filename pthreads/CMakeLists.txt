cmake_minimum_required(VERSION 3.25)

project(pthreads_projects)

set(CMAKE_BUILD_TYPE Debug)

add_executable(tokenize tokenize.c)

add_subdirectory(lab1)