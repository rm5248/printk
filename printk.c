/*
    printk kernel formatting 
    Copyright (C) 2018 Robert Middleton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define FORMAT_BUFFER_SIZE 20
#define JUSTIFY_RIGHT 0
#define JUSTIFY_LEFT 1

enum current_specifier{
    NONE,
    SIGNED_INT,
    UNSINGED_INT,
    UNSIGNED_HEX
};

enum Flags{
    FLAGS_NONE,
    FLAGS_LEFT_JUSTIFY, /* '-' */
    FLAGS_FORCE_SIGN,   /* '+' */
    FLAGS_SPACE,        /* ' ' */
    FLAGS_DECIMAL,      /* '#' */
    FLAGS_LEFT_PAD      /* '0' */
};

struct Specifier{
    int width;
    int precision;
    enum Flags flags;
    char type;
};

static void print( const char* data ){
    printf( data );
}

static void printchar( const char data ){
    printf( "%c", data );
}

static void reverse_notnull( char* string, int len ){
    int location = 0;
    char tmp;

    len -= 1;

    /* Walk through the array backwards, putting the characters in the front */
    for( ; len > 0; len-- ){
        if( string[ len ] == '\0' ) continue;

        if( len < location ) break;

        tmp = string[ len ];
        string[ len ] = string[ location ];
        string[ location ] = tmp;

        location++;
    }
}

static void format_integer( char* location, int len, int number, int base, int capitalize, struct Specifier* specifier ){
    int x = 0;
    const char* characters = "0123456789abcdefghijklmnopqrstuvwxyz";
    int char_idx;
    int negative = number < 0;

    if( negative ) number *= -1;

    if( number == 0 ){
        location[ x++ ] = characters[ 0 ];
    }

    /* Put the numbers in the array backwards */
    while( number > 0 ){
        char_idx = number % base;

        location[ x ] = characters[ char_idx ];
        if( location[ x ] >= 'a' 
              && location[ x ] <= 'z' 
              && capitalize ){
            location[ x ] = location[ x ] - 32;
        }
        x++;

        number = number / base;

        if( x >= len ){
            fprintf( stderr, "buffer not big enough\n" );
            return;
        }
    }

    if( negative && base == 10 ){
        location[ x++ ] = '-';
    }else if( !negative 
        && base == 10 
        && specifier->flags == FLAGS_FORCE_SIGN ){
        location[ x++ ] = '+';
    }

    if( specifier->flags == FLAGS_LEFT_PAD &&
        specifier->width > x ){
        while( x < specifier->width &&
               x < len ){
            location[ x++ ] = '0';
        }
    }

    /* Null out the remainter characters in the array */
    for( ; x < len; x++ ){
        location[ x ] = '\0';
    }

    reverse_notnull( location, len );
}

static enum Flags parse_flag( char c ){
    switch( c ){
    case '-':
        return FLAGS_LEFT_JUSTIFY;
    case '+':
        return FLAGS_FORCE_SIGN;
    case ' ':
        return FLAGS_SPACE;
    case '#':
        return FLAGS_DECIMAL;
    case '0':
        return FLAGS_LEFT_PAD;
    default:
        return FLAGS_NONE;
   }
}

static int ascii_to_int( char c ){
    switch( c ){
    case '9':  return 9;
    case '8':  return 8;
    case '7':  return 7;
    case '6':  return 6;
    case '5':  return 5;
    case '4':  return 4;
    case '3':  return 3;
    case '2':  return 2;
    case '1':  return 1;
    case '0':  return 0;
    default:   return -1;
    }
}

static int printk_parse_width( const char* string, int* chars_consumed ){
    int width = 0;

    *chars_consumed = 0;

    if( *string == '\0' ) return 0;

    if( *string == '*' ){
        *chars_consumed = 1;
        return -1;
    }

    while( *string >= '0' && 
           *string <= '9' &&
           *string != '.' ){
        width *= 10;
        width += ascii_to_int( *string );
        string++;
        *chars_consumed += 1;
    }

    return width;
}

static int printk_parse_precision( const char* string, int* chars_consumed ){
    int precision = 0;

    *chars_consumed = 0;

    if( *string == '\0' || *string != '.' ) return 0;

    string++;
    while( *string >= '0' && 
           *string <= '9' ){
        precision *= 10;
        precision += ascii_to_int( *string );
        string++;
        *chars_consumed += 1;
    }
    
    return precision;
}

/*
 * Parse the format specifier.  format[start] must be pointing at '%' character
 */
static int parse_specifier( const char* format, int start, int len, struct Specifier* specifier ){
    char* location = (char*)format + start + 1;
    const char* end = format + len;
    int chars_consumed;
    int total_consumed = 0;

    if( *location == '%' ){
        specifier->type = '%';
        return 1;
    }

    specifier->flags = parse_flag( *location );
    if( specifier->flags != FLAGS_NONE ){
        location++;
        total_consumed += 1;
    }

    if( location >= end ) return -1;

    specifier->width = printk_parse_width( location, &chars_consumed );
    location += chars_consumed;
    total_consumed += chars_consumed;

    if( location >= end ) return -1;

    specifier->precision = printk_parse_precision( location, &chars_consumed );
    location += chars_consumed;
    total_consumed += chars_consumed;

    if( location >= end ) return -1;

    /* At this point, we need to be pointed at the conversion specifier */
    specifier->type = *location;
    total_consumed += 1;

    /* need to return how many chars we consumed */
    return total_consumed;
}

static void print_string( const char* string, int min_chars, int justify ){
    int stringlen = strlen(string);
    int x;

    if( min_chars == 0 || stringlen == min_chars ){
        print( string );
        return;
    }

    if( stringlen < min_chars && 
        justify == JUSTIFY_RIGHT ){
        /* print until we are the minimum width */
        for( x = 0; x < min_chars - stringlen; x++ ){
            print( " " );
        }
        print( string );
    }else if( stringlen < min_chars &&
        justify == JUSTIFY_LEFT ){
        print( string );
        for( x = stringlen; x < min_chars; x++ ){
            print( " " );
        }
    }
}

int printk( const char* format, ... ){
    va_list args;
    struct Specifier specifier;
    int current_location = 0;
    int len = strlen( format );
    char format_buffer[ FORMAT_BUFFER_SIZE ];
    int base;
    int uppercase;
    char* toprint;

    va_start(args,format);
    while( format[ current_location ] != '\0' ){
        if( format[ current_location ] == '%' ){
            specifier.width = 0;
            specifier.precision = 0;
            specifier.flags = FLAGS_NONE;
            specifier.type = '\0';
            base = 0;

            current_location += parse_specifier( format, current_location, len, &specifier );

            switch( specifier.type ){
            case '%': 
                print( "%" ); 
                break;
            case 'd':
            case 'i': 
                base = 10;
                uppercase = 0;
                break;
            case 'x':
            case 'X':
            case 'p':
                base = 16;
                uppercase = (specifier.type == 'X');
                break;
            case 's':
                toprint = va_arg(args,char*);
                break;
            case 'c':
                format_buffer[ 0 ] = va_arg(args,int);
                format_buffer[ 1 ] = '\0';
                toprint = format_buffer;
                break;
            }

            if( base ){
                format_integer( format_buffer, FORMAT_BUFFER_SIZE, va_arg(args,int), base, uppercase, &specifier );
                toprint = format_buffer;
            }
            print_string( toprint, specifier.width, specifier.flags == FLAGS_LEFT_JUSTIFY );
        }else{
            printchar( format[ current_location ] );
        }

        if( format[ current_location ] != '\0' ){
            current_location++;
        }
    }

    va_end(args);
}

int main( int argc, char** argv ){
    int x;

    printk( "ten is: %d", 10 );
    printf( "\n" );

    printk( "foo %d", -57 );
    printf( "\n" );

    printk( "foo %x", 10 );
    printf( "\n" );

    printk( "foo %d %d %s", 55, 234, "what what in de butt" );
    printf( "\n" );

    printk( "char testing %c", 'j' );
    printf( "\n" );

    printk( "number with plus sign: %+d", 90 );
    printf( "\n" );

    printk( "number in field 4 chars wide: \"%4d\"", 48 );
    printf( "\n" );

    printk( "number in field 8 chars wide with leading 0(decimal: 787234): \"%08x\"", 787234 );
    printf( "\n" );

    printk( "print string %s", "test value" );
    printf( "\n" );

    for( x = 0; x < 20; x++ ){
        printk( "x is %d 0x%x", x, x );
        printf( "\n" );
    }
}
