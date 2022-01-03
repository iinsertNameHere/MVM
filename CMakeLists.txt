cmake_minimum_required(VERSION 3.21)
project(slick)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -std=c11 -Wswitch-enum -Wmissing-prototypes -pedantic ${CMAKE_C_FLAGS}")

add_executable(svm src/svm/svm.c)
add_executable(vasm src/vasm/vasm.c)
add_executable(devasm src/devasm/devasm.c)