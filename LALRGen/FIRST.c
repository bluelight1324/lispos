/* first.c --------------- Find FIRST Sets     Page 305                       */

//#include <windows.h>
#pragma warning(disable:4115)
#pragma warning(disable:4211)
#pragma warning(disable:4018)
#pragma warning(disable:4715)

//#include <standard.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "debug.h"
#include "set.h"
#include "hash.h"
#include <virtualmem.h>    /* memory-management functions */


 //extern _TINT       ptab( HASH_TAB *, _TINT(*prnt)(SYMBOL *), void *, _TINT );
//extern _TINT       ptab( HASH_TAB *, _TINT(*prnt)(), void *, _TINT );

/* FIRST.C      Compute FIRST  sets for all productions in a symbol table.
 * -----------------------------------------------------------------------
 */

 _TINT   first_closure    P(( SYMBOL *lhs ));                     /* local   */
 void    first            P(( void ));                            /* public  */
 _TINT   first_rhs        P(( SET *dest, SYMBOL  **rhs, _TINT len ));


 static  _TINT Did_something;

 /*----------------------------------------------------------------------*/

   void first()
 {
    /* Construct FIRST sets for all nonterminal symbols in the symbol table   */

    D( _tprintf( _T("\nFinding FIRST sets\n") ); )

    do
    {
      Did_something = 0;
      ptab( Symtab, first_closure, NULL, 0 );

    } while( Did_something );
 }

/*-------------------------------------------------------------------------------*/

static _TINT first_closure(SYMBOL *lhs )
/*SYMBOL  *lhs;                                 Current left-hand side  */
{

      /* Called for every element in the FIRST sets. Adds elements to the first
       * sets. The following rules are used:
       *
       * 1) given lhs->...Y... where Y is a terminal symbol preceded by any number
       *   (including 0) of nullable nonterminal symbols or actions, add Y to
       *   FIRST(x).
       *
       * 2) given lhs->...y... where y is a nonterminal symbol preceded by any
       *    number (including 0) of nullable nonterminal symbols or actions, add
       *    FIRST(y) to FIRST(lhs).
       */

       PRODUCTION  *prod;             /* Pointer to one production side        */
       SYMBOL      **y;               /* Pointer to one element of production  */
       static  SET *set = NULL;       /* Scratch-space set.                    */
       _TINT         i;

       if( !ISNONTERM(lhs) )          /* Ignore entries for terminal symbols   */
       {
             return(1);               /* CC 1 added to return  */
       }

       if( !set )                     /* Do this once. The set isn't free()d */
           set = newset();

       ASSIGN( set, lhs->first );

       for( prod = lhs->productions; prod ; prod = prod->next )
       {
          if( prod->non_acts <= 0 )    /* No non-action symbols */
          {                            /* add epsilon to first set */
             ADD( set, EPSILON );
             continue;
          }

          for( y = prod->rhs, i = prod->rhs_len; --i >= 0; y++ )
          {
               if( ISACT( *y ) )      /* pretend acts don't exist */
                   continue;

               if( ISTERM( *y ) )
               {
                   ADD( set, (*y)->val );
               }
                                      /*  it's a nonterminal */
               else if( *y )
               {
			       UNION ( set, (*y)->first );
               }

			   if( !NULLABLE(*y ))    /* it's not a nullable nonterminal */
			       break;
		  }
  }

  if( !IS_EQUIVALENT(set,lhs->first) )
  {
      ASSIGN( lhs->first, set );
	  Did_something = 1;
  }
  return 1;  /* Added explicit return */
}

/*----------------------------------------------------------------------*/
  _TINT  first_rhs(SET *dest,SYMBOL **rhs,_TINT len )
/* SET          *dest;    Target set              */
/* SYMBOL       **rhs;    A right-hand side       */
/* _TINT          len;      # of objects in rhs     */
{
    /* Fill the destination set with FIRST(rhs) where rhs is the
	 * right-hand side of a production represented as an array of
	 * pointers to symbol-table elements. Return 1 if the entire
	 * right-hand side is nullable, otherwise return 0.
	 */


 if( len <= 0 )
 {
    ADD( dest, EPSILON );
	return 1;
 }

 for(; --len >= 0; ++rhs )
 {
     if( ISACT( rhs[0] ) )
	     continue;

	 if( ISTERM( rhs[0] ) )
	     ADD( dest, rhs[0]->val );
	 else
	     UNION( dest, rhs[0]->first );

	 if( !NULLABLE( rhs[0] ) )
	     break;
 }

 return( len < 0 );
}
