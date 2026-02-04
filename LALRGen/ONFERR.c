/* onferr.c -- Action Function for ferr()    Page 733         */
 #include <stdlib.h>
 #include "debug.h"

/* This is the default routine called by ferr when it exits. It should return
 * the exit status. You can supply  your own version of this routine if you like
 */

 int on_ferr()
 {
     extern int errno;
     return errno;
 }

