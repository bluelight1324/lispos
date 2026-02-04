/* ferr.c -- Fatal Error Processing    Page 732             */

#pragma warning(disable:4115) 
//#include <standard.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>     /* errno is declared here       */
#include <stdarg.h>            
#include "debug.h"      /* VA_LIST definition is here   */


//extern  void prnt( _TINT (*ofunct)(_TINT,FILE*),FILE *funct_arg,_TCHAR *format,va_list args );
extern  void prnt( _TINT (*ofunct)(),FILE *funct_arg,_TCHAR *format,va_list args );
extern  _TINT     on_ferr(); 


/* Note that ferr() is typed as _TINT, even though it usually doesn't return,
 * because it's occasionally useful to use it inside a conditional expression
 * whare a type will be required. VA_list expands to ... if ANSI is #defined,
 * otherwise it expands to _a_r_g_s.
 */


 _TINT      FError(_TCHAR  *fmt,...)
 {
           va_list           args; 
          va_start( args, fmt );

          if( fmt) 
			  prnt  ( _fputtc, stderr, fmt, args );
          else     _tperror( va_arg(args, _TCHAR * )    );

          va_end( args );
          exit( on_ferr() );
		  return(1);
		  
 }

