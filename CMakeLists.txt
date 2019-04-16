cmake_minimum_required(VERSION 3.13)
project(meloader C)

set(CMAKE_C_STANDARD 11)
set (CMAKE_C_FLAGS "-m32")
include_directories(include)

add_executable(meloader
        include/manifest.h
        manifest.c
        map.c
        include/meloader.h
        me_abi.c
        kernel/threads.c
        romlib/mmio.c
        include/romlib.h
        include/printf.h
        include/numfmt.h
        romlib/thunks.c
        kernel/simple_io.c
        kernel/memory.c
        util/numfmt.c
        util/printf.c
        periph/tracehub/gth.c
        include/tracehub.h
        periph/tracehub/rs1.c
        periph/tracehub/ftmr.c
        util/trace.c
        include/mipitrace.h
        syslib/kernelcall.c
        include/syslib.h
        syslib/tls.c
        syslib/thunks.c
        periph/sks/sks.c
        periph/spi/spi.c)
