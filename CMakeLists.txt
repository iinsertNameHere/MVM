cmake_minimum_required(VERSION 3.21)
project(slick)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic ${CMAKE_C_FLAGS}")

add_executable(svm src/svm/svm.c)
add_executable(masm src/masm/masm.c)
add_executable(demasm src/demasm/demasm.c)