#include <stdlib.h>
#include "manifest.h"

/**
 * Looks up an extension from a manifest/metadata file
 * @param man Pointer to the start of the file buffer
 * @param size Size of the file buffer
 * @param type The tyoe tag if the extension to look up
 * @return Pointer to the extension that was looked up
 */
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
