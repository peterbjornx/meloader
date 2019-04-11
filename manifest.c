#include <stdlib.h>
#include "manifest.h"

void *man_ext_find( void *man, int size, int type )
{
    man_ext *ext;
    int pos = 0;
    for ( pos = 0; pos < size; pos+=ext->length ) {
        ext = (man_ext*) (man + pos);
        if ( ext->type == type )
            return (void *) ext;
    }

    return NULL;
}
