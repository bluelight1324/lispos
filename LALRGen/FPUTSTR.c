/* fputstr.c-- Fatal Error Processing -- Service Routine  Page 733  */

#include <stdio.h>
#include "debug.h"

/*---------------------- Externs------------------------------------- */
 extern char  *bin_to_ascii(int c,int use_hex );

/* FPUTSTR.C: Print a string with control characters mapped to readable strings.
 */

 void     fputstr(unsigned char *str,int maxlen,FILE *stream )
 {
      char *s;

      while( *str  && maxlen >= 0 )
      {
          s = bin_to_ascii( *str++, 1 );
          while( *s && --maxlen >= 0 )
             putc( *s++, stream );
      }
}
