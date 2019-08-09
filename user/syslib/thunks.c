//
// Created by pbx on 03/04/19.
//

#include "user/meloader.h"
#include "user/loader.h"
#include "user/syslib.h"

static void install_thunk( const cfg_section *section, const char *name, void *thunk ,int rec) {
    uint32_t targ;
    int s;
    s = cfg_find_int32( section, name, &targ );
    if ( s >= 0 ) {
        if ( rec )
            insert_thunk_rec((void *) targ, thunk);
        else
            insert_thunk((void *) targ, thunk);
    }
}

void syslib_install_thunks( const cfg_section *section ) {
    install_thunk(section, "kernelcall" , kernelcall, 0  );
    install_thunk(section, "get_tls_ptr" , get_tls_ptr, 1  );
}
