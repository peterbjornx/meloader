//
// Created by pbx on 03/04/19.
//

#include "user/meloader.h"
#include "user/loader.h"
#include "user/romlib.h"
#include "cfg_file.h"

static void install_thunk( const cfg_section *section, const char *name, void *thunk ) {
    uint32_t targ;
    int s;
    s = cfg_find_int32( section, name, &targ );
    if ( s >= 0 )
        insert_thunk( (void *) targ, thunk );
}

void romlib_install_thunks( const cfg_section *section ) {
    install_thunk( section, "tstamp_read" , tstamp_read  );
    install_thunk( section, "write_seg_32", write_seg_32 );
    install_thunk( section, "write_seg_16", write_seg_16 );
    install_thunk( section, "write_seg_8" , write_seg_8  );
    install_thunk( section, "read_seg_32" , read_seg_32  );
    install_thunk( section, "read_seg_16" , read_seg_16  );
    install_thunk( section, "read_seg_8"  , read_seg_8   );
    install_thunk( section, "write_seg"   , write_seg    );
    install_thunk( section, "read_seg"    , read_seg     );
}
