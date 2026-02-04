/* signon.c -- Print Sign-on Message  Page 333                   */

#include <windows.h>
//#include <standard.h>
#include <stdpar.h>
#include <stdio.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h"



void   signon()
{

    /* Print the sign-on message . Note that since the console is opened
     * explicitly, the message is printed even if both stdout and stderr are
     * redirected. I'm using the ANSI __TIME__ and __DATE__ macros to get the
     * time and date of compilation
     */

     FILE *screen;

     UX( if( !(screen = _tfopen(_T("/dev/tty"), _T("w"))) )            )
     MS( if( !(screen = _tfopen(_T("con:"),     _T("w"))) )            )
             screen = stderr;

     //_ftprintf(stdout,_T("%s  %s\n"),_T(PROG_NAME),_T(PROG_VERSION) ); 
     //_ftprintf(stdout,_T("%s  %s  %s \n"),_T(COPYRIGHT),_T(COMPANY_NAME),_T(RIGHTS));
     if( screen != stderr )
         fclose(screen);
 }
