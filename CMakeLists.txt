cmake_minimum_required(VERSION 3.16)
project(MPI_Array)

set(CMAKE_CXX_STANDARD 14)

add_executable(MPI_Array Mpi_array.cpp)

# Require MPI for this project:
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpic++)
set(SOURCE_FILES Mpi_array.cpp)