cmake_minimum_required(VERSION 3.21)
project(slick)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -std=c11 -Wswitch-enum -pedantic ${CMAKE_C_FLAGS}")

add_executable(svm src/svm/svm.c)
add_executable(asmsc src/asms/asmsc.c)
add_executable(diasms src/diasms/diasms.c)