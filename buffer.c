
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

UCHAR *pBuffer = NULL;
UINT buf_size = 0;
UINT used = 0;
UINT ALLOC_SIZE = 100;

bool ensure_size( UINT needed ) {
    if( used + needed > buf_size ) {
        int new_size = ( needed > ALLOC_SIZE ) ? needed + used : ALLOC_SIZE + used;
        pBuffer = realloc( pBuffer, new_size * sizeof( UCHAR ) );
        buf_size = new_size;
        memset( pBuffer + used, 0x00, buf_size - used );
    }

    return ( pBuffer != NULL );
}


const UCHAR* init_buffer( UINT size ) {
    //we handle only one buffer
    if( pBuffer ) return NULL;

    ensure_size( ( size > ALLOC_SIZE ) ? size : ALLOC_SIZE );

    return pBuffer;
}

const UCHAR* free_and_init_buffer( UINT size ) {
    free_buffer();
    init_buffer( size );
    return pBuffer;
}

const UCHAR* get_buffer()  {
    return pBuffer;
}

int append_char( const char* pFormat, UCHAR uchar ) {
    ensure_size( strlen( pFormat ) - 1 + sizeof( UCHAR ) );
    UCHAR* p = pBuffer + used;
    int n = sprintf( (char*)p, pFormat, uchar );
    used += n;

    return n;
}

int append_string( const char* pFormat, const char* pString ) {
    int len = pString ? strlen( pString ) : 0;
    ensure_size( strlen( pFormat ) - 1 + len );
    UCHAR* p = pBuffer + used;
    int n = sprintf( (char*)p, pFormat, pString );
    used += n;

    return n;
}

void free_buffer() {
    free( pBuffer );
    pBuffer = NULL;
    buf_size = 0;
    used = 0;
}

