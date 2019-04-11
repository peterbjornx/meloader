//
// Created by pbx on 03/04/19.
//

#include "meloader.h"
#include "romlib.h"

void romlib_install_thunks() {
    insert_thunk((void *) 0x1010, tstamp_read  );
    insert_thunk((void *) 0x11B9, write_seg_32 );
    insert_thunk((void *) 0x11BE, write_seg_16 );
    insert_thunk((void *) 0x11C3, write_seg_8  );
    insert_thunk((void *) 0x11C8,  read_seg_32 );
    insert_thunk((void *) 0x11CD,  read_seg_16 );
    insert_thunk((void *) 0x11D2,  read_seg_8  );
    insert_thunk((void *) 0x11D7, write_seg    );
    insert_thunk((void *) 0x11DC,  read_seg    );
}
