#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "buffer.h"



const char *USAGE = "\nusage: Hex [-a] [-n] filename\n \
    -a              encoding of file to print is ANSI (default: UTF-8)\n \
    -n MEGABYTE_COUNT write hex output to file after each read MEGABYTE_COUNT bytes \n \
                    (Split) filename(s) will be source file name + '_nnn' \n \
    filename        name of file to print\n\n";

const char *WRONG_NUMBER_MEGABYTES = "\nmegabytes needs to be > 0. Must not be 0\n\n";
const char *FILENAME_TOO_LONG = "\nfile name too long. Max 256 Bytes allowed.\n\n";
const char *FILE_OPEN_ERROR = "\nfile does not exist or is in use\n\n";
const char *SPLITFILE_CREATE_ERROR = "\ncouldn't create split file\n\n";

const int BUFFERSIZE = 16;
enum { MAXLEN_FILENAME = 255 };

FILE *split_file = 0;
int split_file_nr = -1;

struct arguments {
    bool u8; // default encoding is utf-8.
    long split_on_bytes; //create split file each read split_on_bytes; -1 means no splitting.
    char file_name[MAXLEN_FILENAME];
    FILE *fp;
};

void exit_on_failure( int rc, const char* message ) {
    fprintf( stderr, "%s", message );
    exit( rc );
}

/**
 * dumps raw memory in hex byte and printable split format
 */
void dump( const UCHAR* pData_buffer, const unsigned int length ) {

    for( UINT i = 0; i < length; i++ ) {
        //print byte hexadecimal:
        append_char( "%02X ", pData_buffer[i] );

        if( ( ( i % BUFFERSIZE ) == BUFFERSIZE - 1 ) || ( i == length - 1 ) ) {
            //print spaces 'til position of separator:
            for( UINT j = 0; j < ( BUFFERSIZE - 1 ) - ( i % BUFFERSIZE ); j++ ) {
                append_string( "   ", NULL );
            }
            append_string( "| ", NULL );

            for( UINT j = ( i - ( i % BUFFERSIZE ) ); j <= i; j++ ) {
                UCHAR byte = pData_buffer[j];
                if( byte != 0x0A ) {
                    append_char( "%c", byte );
                }

            } //for
            const char* pBuf = get_buffer();
            fprintf( stderr, "%s\n", pBuf );
            if( split_file ) {
                fwrite( get_buffer(), sizeof( UCHAR ), sizeof( pBuf ), split_file );
            }
            free_and_init_buffer( 0 );
        } //if
    } //for
}

void create_split_file( const char* pathnfile ) {
    if( ++split_file_nr > 9999 ) {
        exit_on_failure( 1, "Maximum number of split files exceeded (>9999)\n" );
    }

    char nr[5];
    sprintf( nr, "%d", split_file_nr );
    char *split_file_name = malloc( strlen( pathnfile ) + 5 );

    strcat( strcpy( split_file_name, pathnfile ), nr );

    split_file = fopen( split_file_name, "w" );
    free( split_file_name );
    if( !split_file ) {
        exit_on_failure( 1, SPLITFILE_CREATE_ERROR );
    }
}

/** returns the number of bytes necessary to encode
 *  the given character
 */
int get_number_of_encoding_bytes( int c, bool is_u8 ) {
    if( !is_u8 ) {
        return 1;
    }

    int needed_bytes = 1;  //ASCII char

    if( c >= 0xF0 ) { //4 byte-coded
        needed_bytes = 4;
    } else if( c >= 0xE0 ) {
        needed_bytes = 3;
    } else if( c >= 0xC2 ) {
        needed_bytes = 2;
    }

    return needed_bytes;
}


void check_splitfiles( struct arguments *args, const char *arg2 ) {
    int mega_bytes;
    if( ( mega_bytes = atol( arg2 ) ) == 0 ) {
        //can't convert to number
        exit_on_failure( 1, WRONG_NUMBER_MEGABYTES );
    }
    if( mega_bytes <= 1 ) {
        split_file = NULL;
        args->split_on_bytes = -1;
    } else {
        args->split_on_bytes = mega_bytes * 1000000; //actually 1048576;
    }
}

void check_and_open_file( struct arguments *args, const char *arg3 ) {
    if( strlen( arg3 ) > MAXLEN_FILENAME ) {
        exit_on_failure( 1, FILENAME_TOO_LONG );
    }

    strcpy( args->file_name, arg3 );
    args->fp = fopen( args->file_name, "r" ); // read mode

    if( args->fp == NULL ) {
        exit_on_failure( 1, FILE_OPEN_ERROR );
    }
}

void process_arguments( int argc, char **argv, struct arguments *args ) {
    if( argc < 2 ) {
        exit_on_failure( 1, USAGE );
    }

    args->u8 = true;
    args->split_on_bytes = -1;

    for( int i = 1; i < argc; i++ ) {
        const char *arg = argv[i];
        if( !strcmp( "-a", arg ) ) {
            args->u8 = false;
        } else if( !strcmp( "-n", arg ) ) {
            check_splitfiles( args, argv[++i] );
        } else {
            check_and_open_file( args, argv[i] );
        }
    }
    if( args->split_on_bytes > 0 ) {
        create_split_file( args->file_name );
    }
}

//int main_() {
//    UCHAR* pBuf = init_buffer( 10 );
//    int n;
//    n = append_char( "%czzz", 'a' );
//    n = append_string( "%s!!", "that is what gives me hope" );
//    n = append_string( "--%s:-)", "well, that's what I think " );
//    n = append_string( "...%s: THE END\n", "and that is, you won't believe it " );
//    n = append_string( "\nBUT %s", "a new story is starting NOW!\n" );
//    n = append_char( "it will be ver%c startling\n", 'y' );
//    fprintf( stderr, "%s", get_buffer() );
//    return 0;
//}

int main( int argc, char **argv ) {
    struct arguments args;
    process_arguments( argc, argv, &args );

    int c;
    long n = 0; //number of bytes
    UCHAR chars[BUFFERSIZE];

    memset( chars, 0x00, sizeof( chars ) );
    init_buffer( 0 );

    UCHAR *p = chars;
    int buf_cnt = 0;
    while ( ( c = getc( args.fp ) ) != EOF ) {
        n++;
        //char coded by more than 1 Byte? If so, check forcing dump NOW
        //see https://en.wikipedia.org/wiki/UTF-8
        int needed_bytes = get_number_of_encoding_bytes( c, args.u8 );
        bool force_dump = BUFFERSIZE - buf_cnt < needed_bytes;

        if( !force_dump ) {
            buf_cnt++;
            *p++ = c;
        }

        if( ( buf_cnt % BUFFERSIZE ) == 0 || c == 0x0A || force_dump ) {
            dump( chars, p - chars );
            memset( chars, 0x00, sizeof( chars ) );
            p = chars;
            buf_cnt = 0;
        } //if

        if( force_dump ) {
            *p++ = c;
            buf_cnt++;
        }
        if( args.split_on_bytes > 0 && ( n % args.split_on_bytes ) ) {
            //save_split_file( args.file_name, args.split_file_nr++ );
        }
    } //while

    if( buf_cnt > 0 ) {
        dump( chars, p - chars );
    }

    //fprintf( stderr, "%s\n", get_buffer() );

    fclose( args.fp );
    fprintf( stderr, "\nChars gelesen: %ld\n", n );
    free_buffer();

    getchar();

    return 0;
}
