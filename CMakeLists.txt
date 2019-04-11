cmake_minimum_required(VERSION 3.13)
project(meloader C)

set(CMAKE_C_STANDARD 11)
set (CMAKE_C_FLAGS "-m32")
include_directories(include)

add_executable(meloader
        include/manifest.h
        manifest.c
        map.c include/meloader.h me_abi.c kernel/threads.c romlib/mmio.c include/romlib.h romlib/thunks.c kernel/simple_io.c kernel/memory.c)
