
/* bintoasc.c-- Convert Binary to Human readable form  Pg.731             */

#include <standard.h>
#include <stdio.h>
#include "debug.h"
#include <virtualmem.h>                   /* memory-management functions */

_TCHAR  *bin_to_ascii(_TINT c,_TINT use_hex )
{
    /* Return a pointer to a string that represents c. This will be the
     * character itself for normal characters and an escape sequence (\n, \t,
     * \x00, etc., for most others). A ' is represented as \'.The string will 
     * be destroyed the next time bin_to_ascii() is called. If "use_hex" is true
     * then \xDD escape sequemnces are used. Otherwise, octal sequences (\DDD)
     * are used.( see also: pchar.c )
     */

     static _TUCHAR buf[8]; /* Original : static unsigned char buf[8];*/

     c &= 0xff ;
     if( _T(' ') <= c && c < 0x7f  &&  c != _T('\'')  && c!= _T('\\'))
     {
        buf[0] = c;
        buf[1] = _T('\0');
     }
     else
     {
        buf[0] = _T('\\') ;
        buf[2] = _T('\0') ;

        switch(c)
        {
        case _T('\\'): buf[1] = _T('\\') ; break;
        case _T('\''): buf[1] = _T('\'') ; break;
        case _T('\b'): buf[1] = _T('b' ) ; break;
        case _T('\f'): buf[1] = _T('f' ) ; break;
        case _T('\t'): buf[1] = _T('t' ) ; break;
        case _T('\r'): buf[1] = _T('r' ) ; break;
        case _T('\n'): buf[1] = _T('n' ) ; break;
        default  : _stprintf((_TCHAR *)&buf[1], use_hex ? _T("x%03x") : _T("%03o"), c );
                   break;
        }
    }
    return (_TCHAR *)buf;
}
