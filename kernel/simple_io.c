//
// Created by pbx on 03/04/19.
//
#include <unistd.h>
#include <string.h>

char hexlookup [] = "0123456789ABCDEF";
void write_hexn( int v ) {
    v &= 0xf;
    write( 2, hexlookup + v, 1 );
}

void write_hex(int n,int i){
    for ( int p = n-1; p >= 0; p-- ) {
        write_hexn(i >> (p*4));
    }
}

void write_str(char *s) {
    write(2, s , strlen(s));
}