cmake_minimum_required(VERSION 3.19)
project(freedb C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "-Wshadow ${CMAKE_CXX_FLAGS}")

set(LMDB_H /usr/local/include/)
include_directories(${LMDB_H})
set(LMDB_LIB /usr/local/lib/)
link_directories(${LMDB_LIB})

link_libraries(lmdb)
file(GLOB SRC_LIST "src/*.c" "bees/*.c" "resp/*.c" "util/*.c" "third/bstrlib/*.c")
add_executable(freedb main.c ${SRC_LIST})

add_executable(freedb-cli freedb-cli/main.c ${SRC_LIST})

aux_source_directory(utest UT_LIST)
aux_source_directory(cutest CUTEST_LIB_LIST)
add_executable(utest ${SRC_LIST} ${UT_LIST} ${CUTEST_LIB_LIST})