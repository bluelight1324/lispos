
/* hashadd.c -- An addition-Based Hash Function Pg. 723   */
#pragma warning(disable:4115) 
#pragma warning(disable:4220)
//#include <standard.h>
#include <tchar.h>
#include "debug.h"

/*--------------------------------------------------------------------------------
 * Hash function for use with the functions in hash.c. Just adds together
 * characters in the name.
 */

 //unsigned hash_add(_TUCHAR *name )
_TINT hash_add(_TUCHAR *name )
 {

     //unsigned  h ;
	 _TINT h;

     for( h = 0;  *name ; h += *name++ ) 
              ;
     return h;
 }										

