//
// Created by pbx on 09/08/19.
//

#ifndef MELOADER_FILE_H
#define MELOADER_FILE_H

#include <sys/types.h>

off_t filesz(int fd );

void *read_full_file(const char *path, uint16_t *pInt);

#endif //MELOADER_FILE_H
