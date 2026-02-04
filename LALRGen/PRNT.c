 /* prnt.c-- General Purpose printf() Driver     Page 736                 */

#include <windows.h>
//#include <standard.h>
#include <stdio.h>
#include <tchar.h>
#include "debug.h"

/*---------------------------------------------------------------------------------
 * Glue formatting workhorse functions to various environments. One of three
 * versions of the workhorse function is used, depending on various #defines:
 *
 * if ANSI is defined          vsprintf()       Standard ANSI function
 * if ANSI is not defined      _doprnt()        Standard UNIX function
 *
 * The default with Microsoft C is MSDOS defined and ANSI not defined,
 * so idoprnt() will be used unless you change things with explicit macro 
 * definitions
 */

 #ifdef ANSI /*-------------------------------------------------------------------*/
 #include <stdarg.h>

 void prnt(int (*ofunct)(int,FILE*),FILE *funct_arg,_TCHAR *format,va_list args )
 {
     _TCHAR  buf[256]; /* Original :  char  buf[256] */
     _TCHAR  *p ;
//     int   vsprintf( _TCHAR* buf,const _TCHAR* fmt, va_list args );
	 
	 //buf = (char *)AllocMem(256 * sizeof(char));
     _vstprintf(buf, format, args);

     for( p = buf; *p ; p++ )
          ( *ofunct)( *p, funct_arg );
}

  
 
  void  stop_prnt(){ }

#else /* UNIX -------------------------------------------------------------------*/
#include <varargs.h>


static FILE     *Tmp_file = NULL ;
static char     *Tmp_name;

  void prnt( ofunct, funct_arg, fmt, argp )
int      (*ofunct)();
void     *funct_arg;
char     *fmt;
int      *argp;
{
    int    c;
    char   *mktemp();

    if( !Tmp_file)
        if( !(Tmp_file = fopen( Tmp_name = mktemp("yyXXXXXX"), "w+") ))
        {
            fprintf(stderr,"Can't open temorary file %s\n", Tmp_name );
            exit(1);
        }

        _doprnt( fmt, argp, Tmp_file );
        putc   ( 0,         Tmp_file );
        rewind (            Tmp_file );

        while( (c= getc(Tmp_file)) != EOF && c )
              (*ofunct)( c, funct_arg );
        rewind( Tmp_file );
 }


  void  stop_prnt()
 {
   fclose( Tmp_file );             /* Remove prnt temporary file   */
   unlink( Tmp_name );
   Tmp_file = NULL;
 }

 /*------------------------------------------------------------------------------*/

   void _vfprintf__( stream, fmt, argp )
 FILE   *stream;
 char   *fmt, *argp;
 {
     _doprnt( fmt, argp, stream );
 }

  void _vprintf__( fmt, argp )
 char *fmt, *argp;
 {
     _doprnt( fmt, argp, stdout );
 }


 static  void putstr( c, p)
 int    c;
 char   **p;
 {
    *(*p)++ = c;
 }

   void _vsprintf__( str, fmt, argp )
 char  *str, *fmt, *argp;
 {
   prnt( putstr, &str, fmt, argp );
   *str = '\0';
 }
#endif
