//
// Created by pbx on 19/04/19.
//

#ifndef MELOADER_GRANT_H
#define MELOADER_GRANT_H

#include <stdint.h>

#define MG_EXISTS       (1)
#define MG_FIELD1_USED  (2)
#define MG_BY_PID       (4)
#define MG_FLAG1        (8)
#define MG_FLAG2        (16)

typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t buffer;
    uint32_t size;
    uint32_t segment;
    uint32_t obj;
} mg_desc_t;

extern mg_desc_t *mg_list;
extern int        mg_count;

#endif //MELOADER_GRANT_H
