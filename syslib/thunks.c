//
// Created by pbx on 03/04/19.
//

#include "meloader.h"
#include "syslib.h"

void syslib_install_thunks() {
    insert_thunk((void *) 0xCDC3, kernelcall  );
    insert_thunk_rec( (void *) 0x978A, get_tls_ptr);
}
