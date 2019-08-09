//
// Created by pbx on 09/08/19.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fsb.h>
#include "log.h"

off_t filesz(int fd ) {
    off_t oldpos, size;
    oldpos = lseek( fd, 0, SEEK_CUR );
    size = lseek( fd, 0, SEEK_END );
    lseek( fd, oldpos, SEEK_SET );
    return size;
}

void *read_full_file(const char *path, uint16_t *psize) {
    int fd;
    size_t size;
    void *map, *buf;
    fd = open(path, O_RDONLY);
    logassert( fd >= 0, "loader", "Could not open file: %s\n", strerror( errno ) );

    size = filesz( fd );

    map = mmap( NULL, size, PROT_READ, MAP_PRIVATE, fd, 0 );
    logassert( map != MAP_FAILED, "loader", "Could not map file: %s\n", strerror( errno ) );

    buf = malloc( size );
    logassert( buf != NULL, "loader", "Could not allocate file buffer\n");

    memcpy( buf, map, size );

    munmap( map, size );

    if ( psize )
        *psize = size;

    return buf;
}