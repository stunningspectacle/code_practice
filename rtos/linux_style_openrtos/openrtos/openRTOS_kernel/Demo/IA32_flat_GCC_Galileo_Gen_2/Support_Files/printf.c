/*----------------------------------------------------------------------------
Stripped-down printf()
Chris Giese <geezer@execpc.com> http://my.execpc.com/~geezer
Release date: Feb 3, 2008
This code is public domain (no copyright).
You can do whatever you want with it.

Revised Feb 3, 2008
- sprintf() now works with NULL buffer; returns size of output
- changed va_start() macro to make it compatible with ANSI C

Revised Dec 12, 2003
- fixed vsprintf() and sprintf() in test code

Revised Jan 28, 2002
- changes to make characters 0x80-0xFF display properly

Revised June 10, 2001
- changes to make vsprintf() terminate string with '\0'

Revised May 12, 2000
- math in DO_NUM is now unsigned, as it should be
- %0 flag (pad left with zeroes) now works
- actually did some TESTING, maybe fixed some other bugs

%[flag][width][.prec][mod][conv]
flag:   -   left justify, pad right w/ blanks   DONE
    0   pad left w/ 0 for numerics      DONE
    +   always print sign, + or -       no
    ' ' (blank)                 no
    #   (???)                   no

width:      (field width)               DONE

prec:       (precision)             no

conv:   d,i decimal int             DONE
    u   decimal unsigned            DONE
    o   octal                   DONE
    x,X hex                 DONE
    f,e,g,E,G float                 no
    c   char                    DONE
    s   string                  DONE
    p   ptr                 DONE

mod:    N   near ptr                DONE
    F   far ptr                 no
    h   short (16-bit) int          DONE
    l   long (32-bit) int           DONE
    L   long long (64-bit) int          no
----------------------------------------------------------------------------*/

#include <stdarg.h>
#include "galileo_support.h"


/* flags used in processing format string */
#define PR_LJ   0x0001  /* left justify */
#define PR_CA   0x0002  /* use A-F instead of a-f for hex */
#define PR_SG   0x0004  /* signed numeric conversion (%d vs. %u) */
#define PR_32   0x0008  /* long (32-bit) numeric conversion */
#define PR_16   0x0010  /* short (16-bit) numeric conversion */
#define PR_WS   0x0020  /* PR_SG set and num was < 0 */
#define PR_LZ   0x0040  /* pad left with '0' instead of ' ' */
#define PR_FP   0x0080  /* pointers are far */
#define PR_PR   0x0100  /* precision specifier */
/* largest number handled is 2^32-1, lowest radix handled is 8.
2^32-1 in base 8 has 11 digits (add 5 for trailing NUL and for slop) */
#define PR_BUFLEN   16


typedef int (*fnptr_t)(unsigned c, void **helper);

static int print_char_to_screen(unsigned c, void **ptr_UNUSED)
{
    vGalileoPrintc( c );
    (void)ptr_UNUSED;
    return 0 ;
}

static int do_printf(const char *fmt, va_list args, fnptr_t fn, void *ptr)
{
    unsigned flags, actual_wd, count, given_wd, precision, actual;
    unsigned char *where, buf[PR_BUFLEN];
    unsigned char state, radix;
    long num;

    state = flags = count = given_wd = precision = 0;
/* begin scanning format specifier list */
    for(; *fmt; fmt++)
    {
        switch(state)
        {
/* STATE 0: AWAITING % */
        case 0:
            if(*fmt != '%') /* not %... */
            {
                fn(*fmt, &ptr); /* ...just echo it */
                count++;
                break;
            }
/* found %, get next char and advance state to check if next char is a flag */
            state++;
            fmt++;
            /* FALL THROUGH */
/* STATE 1: AWAITING FLAGS (%-0) */
        case 1:
            if(*fmt == '%') /* %% */
            {
                fn(*fmt, &ptr);
                count++;
                state = flags = given_wd = precision = 0;
                break;
            }
            if(*fmt == '-')
            {
                if(flags & PR_LJ)/* %-- is illegal */
                    state = flags = given_wd = precision = 0;
                else
                    flags |= PR_LJ;
                break;
            }
            if(*fmt == '0')
            {
                flags |= PR_LZ;
                break;
            }
/* not a flag char: advance state to check if it's field width */
            state++;
            /* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
        case 2:
            if(*fmt >= '0' && *fmt <= '9')
            {
                given_wd = 10 * given_wd +
                    (*fmt - '0');
                break;
            }
/* not field width: advance state to check if it's a precision  */
            state++;
            /* FALL THROUGH */
/* STATE 3: AWAITING PRECISION */
        case 3:
            if (flags & PR_PR)
            {
                if(*fmt >= '0' && *fmt <= '9')
                {
                    precision = (10 * precision) + (*fmt - '0');
                    break;
                }
                /* else - we are finished with precision and advance to next state */
            }
            else if (*fmt == '.')
            {
                flags |= PR_PR;
                break;
            }

/* not precision width: advance state to check if it's a modifier */
            state++;
            /* FALL THROUGH */
/* STATE 4: AWAITING MODIFIER CHARS (FNlh) */
        case 4:
            if(*fmt == 'F')
            {
                flags |= PR_FP;
                break;
            }
            if(*fmt == 'N')
                break;
            if(*fmt == 'z')
                break;
            if(*fmt == 'l')
            {
                flags |= PR_32;
                break;
            }
            if(*fmt == 'h')
            {
                flags |= PR_16;
                break;
            }
/* not modifier: advance state to check if it's a conversion char */
            state++;
            /* FALL THROUGH */
/* STATE 5: AWAITING CONVERSION CHARS (Xxpndiuocs) */
        case 5:
            where = buf + PR_BUFLEN - 1;
            *where = '\0';
            switch(*fmt)
            {
            case 'X':
                flags |= PR_CA;
                /* FALL THROUGH */
/* xxx - far pointers (%Fp, %Fn) not yet supported */
            case 'x':
            case 'p':
            case 'n':
                radix = 16;
                goto DO_NUM;
            case 'd':
            case 'i':
                flags |= PR_SG;
                /* FALL THROUGH */
            case 'u':
                radix = 10;
                goto DO_NUM;
            case 'o':
                radix = 8;
/* load the value to be printed. l=long=32 bits: */
DO_NUM:             if(flags & PR_32)
                    num = va_arg(args, unsigned long);
/* h=short=16 bits (signed or unsigned) */
                else if(flags & PR_16)
                {
                    if(flags & PR_SG)
                        num = (short)va_arg(args, int);
                    else
                        num = (unsigned short)va_arg(args, int);
                }
/* no h nor l: sizeof(int) bits (signed or unsigned) */
                else
                {
                    if(flags & PR_SG)
                        num = va_arg(args, int);
                    else
                        num = va_arg(args, unsigned int);
                }
/* take care of sign */
                if(flags & PR_SG)
                {
                    if(num < 0)
                    {
                        flags |= PR_WS;
                        num = -num;
                    }
                }
/* convert binary to octal/decimal/hex ASCII
OK, I found my mistake. The math here is _always_ unsigned */
                actual_wd = 0;
                do
                {
                    unsigned char temp;

                    actual_wd++;
                    temp = (unsigned char)((unsigned long)num % radix);
                    where--;
                    if(temp < 10)
                        *where = temp + '0';
                    else if(flags & PR_CA)
                        *where = temp - 10 + 'A';
                    else
                        *where = temp - 10 + 'a';
                    num = (unsigned long)num / radix;
                }
                while(num != 0);
                goto EMIT;
            case 'c':
/* disallow pad-left-with-zeroes for %c */
                flags &= ~PR_LZ;
                where--;
                *where = (unsigned char)va_arg(args,
                    int);
                actual_wd = 1;
                goto EMIT2;
            case 's':
/* disallow pad-left-with-zeroes for %s */
                flags &= ~PR_LZ;
                where = va_arg(args, unsigned char *);
                actual_wd = 0;

                if (flags & PR_PR) /* if precision was set */
                {
                    /* find actual_wd */
                    unsigned tmp_prec = precision;
                    unsigned char* tmp_where = where;
                    while(tmp_prec && *tmp_where)
                    {
                        --tmp_prec;
                        ++tmp_where;
                        ++actual_wd;
                    }
                }
                else
                {
                    /* find actual_wd without percision */
                    unsigned char* tmp_where = where;
                    while(*tmp_where)
                    {
                        ++tmp_where;
                        ++actual_wd;
                    }
                }
                goto EMIT2; /* to avoid sign checks */
EMIT:
/* if signed number, then increment the actual_wd so sign will be calculated in width */
                if(flags & PR_WS)
                    actual_wd++;
/* if we pad left with ZEROES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) ==
                    (PR_WS | PR_LZ))
                {
                    fn('-', &ptr);
                    count++;
                }
/* pad on left with spaces or zeroes (for right justify) */
EMIT2:              if((flags & PR_LJ) == 0)
                {
                    while(given_wd > actual_wd)
                    {
                        fn(flags & PR_LZ ?
                            '0' : ' ', &ptr);
                        count++;
                        given_wd--;
                    }
                }
/* if we pad left with SPACES, do the sign now */
                if((flags & (PR_WS | PR_LZ)) == PR_WS)
                {
                    fn('-', &ptr);
                    count++;
                }
/* emit string/char/converted number (ignoring already-printed sign) */
                actual = (flags & PR_WS) ? actual_wd-1 : actual_wd;
                for (; actual; actual--)
                {
                    fn(*where++, &ptr);
                    count++;
                }
/* pad on right with spaces (for left justify) */
                if(given_wd < actual_wd)
                    given_wd = 0;
                else given_wd -= actual_wd;
                for(; given_wd; given_wd--)
                {
                    fn(' ', &ptr);
                    count++;
                }
                break;
            default:
                break;
            }
        default:
            state = flags = given_wd = precision = 0;
            break;
        }
    }
    return count;
}

int print( char **ptr_UNUSED, const char *fmt, va_list args )
{
    (void)ptr_UNUSED;
    int rv = do_printf(fmt, args, print_char_to_screen, NULL);
    va_end( args );
    return rv;
}

int printf(const char *fmt, ...)
{
    int rv;
    va_list args;

    va_start(args, fmt);
    rv = do_printf(fmt, args, print_char_to_screen, NULL);
    va_end(args);
    return rv;
}
