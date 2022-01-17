cmake_minimum_required(VERSION 3.21)
project(slick)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic ${CMAKE_C_FLAGS}")


math(EXPR CUSTOM_MASM_MEMORY "100 * 1000 * 1000") # 100MB
# add_compile_definitions(CMAKE_CUSTOM_MASM_MEMORY CUSTOM_MASM_MEMORY)


add_executable(mvm src/mvm/mvm.c)
add_executable(masm src/masm/masm.c)
add_executable(demasm src/demasm/demasm.c)