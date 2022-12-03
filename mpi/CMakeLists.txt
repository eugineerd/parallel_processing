cmake_minimum_required(VERSION 3.25)

project(mpitest)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(trap trap.c)
target_link_libraries(trap ${MPI_C_LIBRARIES})
add_executable(integral integral.c)
target_link_libraries(integral ${MPI_C_LIBRARIES})
add_executable(vecsum vecsum.c)
target_link_libraries(vecsum ${MPI_C_LIBRARIES})
add_executable(sort sort.c)
target_link_libraries(sort ${MPI_C_LIBRARIES})