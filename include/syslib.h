//
// Created by pbx on 15/04/19.
//

#ifndef MELOADER_SYSLIB_H
#define MELOADER_SYSLIB_H
#include <stdint.h>

int kernelcall(uint8_t call_id, uint16_t par_sz, void *par);
void *get_tls_ptr(int i);
#endif //MELOADER_SYSLIB_H
