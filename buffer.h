#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#ifndef UCHAR
#define UCHAR unsigned char
#endif // UCHAR



#ifndef UINT
#define UINT unsigned int
#endif // UINT


#ifndef NULL
#define NULL 0
#endif


/**
    initialize the one and only buffer
    @param size: if 0 the default allocation size will be used
*/
const UCHAR* init_buffer( UINT size );

/**
    free used buffer and initialize a new one
    @param size: if 0 the default allocation size will be used
*/
const UCHAR* free_and_init_buffer( UINT size );

/**
    get a constant pointer to the one and only buffer
*/
const UCHAR* get_buffer();

/**
    append a char to the buffer.
    @param pFormat: a format string as in printf. Only 1 %c is allowed,
           but an arbitrary number of constant chars, e.g.
           "this is my favourite char: %c\n"
    @param uchar: the char to store
*/
int append_char( const char* pFormat, UCHAR uchar ) ;

/**
    append a c string to the buffer.
    @param pFormat: a format string as in printf. Only 1 %s is allowed,
           but an arbitrary number of constant chars, e.g.
           "the brown fox %s over the red fence\n"
    @param pString: the string to replace "%s", e.g. "jumps"
*/
int append_string( const char* pFormat, const char* pString );

/**
    free used buffer
*/
void free_buffer();


#endif // BUFFER_H_INCLUDED
