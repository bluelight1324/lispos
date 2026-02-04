/* hash.c -- BUCKET Allocation Page 716              */

//#include <windows.h>
#pragma warning(disable:4115) 
#pragma warning(disable:4210) 
#pragma warning(disable:4211) 
#pragma warning(disable:4706) 

//#include <standard.h>
#include <windows.h>
#include <stdio.h>
#include <signal.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h"
#include <virtualmem.h>    /* memory-management functions */



extern  void  assort(void  **base,_TINT  nel,_TINT  elsize,_TINT (*cmp)() );
extern   void error(_TINT fatal,_TCHAR *fmt, ... );


  void *newsym(_TINT size )
{
   /* Allocate space for a new symbol; return a pointer to the user space  */

   BUCKET    *sym;
   size_t total_size = (size_t)size + sizeof(BUCKET) + 1;

   if( !(sym = (BUCKET *) AllocMem( (DWORD)total_size)) )
   /* AllocMem is used above instead of calloc originally */
   {
       _ftprintf( stderr, _T("Can't get memory for BUCKET\n") );
       //CallErrorHandler(NOT_ENOUGH_MEMORY);
	   error( FATAL, _T("No memory for action or goto\n") );
       return NULL;
    }
    return (void *)( sym + 1 );      /* return pointer to user space    */
}

/*------------------------------------------------------------------------------*/

 void freesym(void *sym)
{
   FreeMem( (BUCKET *)sym - 1 );
}


 HASH_TAB *maketab(_TINT maxsym, _TINT (*hash_function)(),int (*cmp_function)() )
{

  /* Make a hash table of the indicated size     */

  HASH_TAB    *p;

  if( !maxsym )
      maxsym = 127;
                        /* |<--- space for table --->|<- and header -->| */
  if( p=(HASH_TAB*)  AllocMem(1 * (maxsym * sizeof(BUCKET*)) +
                                     sizeof(HASH_TAB)) 
									)
  /* AllocMem is used above instead of calloc originally */
  {
      p->size     = maxsym        ;
      p->numsyms  = 0             ;
      p->hash     = hash_function ;
      p->cmp      = cmp_function  ;
  }
  else
  {
      _ftprintf(stderr, _T("Insufficent memory for symbol table\n"));
      //CallErrorHandler(NOT_ENOUGH_MEMORY);
	  error( FATAL, _T("No memory for action or goto\n") );
      return (HASH_TAB *)NULL;
  }
  return p;
}

/* Adding a symbol to the table Pg. 718                                       */

  void *addsym(HASH_TAB *tabp,void *isym )
{
    /* Add a symbol to the hash table                                         */

    BUCKET   **p, *tmp ;
    BUCKET   *sym = (BUCKET *)isym;

    p = & (tabp->table)[ (*tabp->hash)( sym-- ) % tabp->size ];

    tmp          = *p;
    *p           = sym;
    sym->prev    = p;
    sym->next    = tmp;

    if( tmp )
        tmp->prev = &sym->next;

    tabp->numsyms++;
    return (void*)(sym + 1);
}


/* Removing a Node from the table                                    */

 void  delsym(HASH_TAB *tabp,void *isym )
{
    /* Remove a symbol from the hash table, "sym" is a pointer  returned from
     * a previous findsym() call. It points initially at the user space, but
     * is decremented to get at the BUCKET header.
     */

     BUCKET   *sym = (BUCKET *)isym;

     if( tabp && sym )
     {
         --tabp->numsyms;
         --sym;

         if( *(sym->prev) = sym->next )
             sym->next->prev = sym->prev;
     }
}

/* Finding a Symbol Pg. 720                                                  */

  void    *findsym(HASH_TAB *tabp,void *sym)
{
    /* Return a pointer to the hash table element having a particular name
     * or NULL if the name isn't in the table.
     */

     BUCKET     *p = NULL; 

     if( !tabp )                 /* Table empty   */
         return NULL;

     p = (tabp->table)[ (*tabp->hash)(sym) % tabp->size ];

     while( p && (*tabp->cmp)( sym, p+1 ) )
            p = p->next;

     return (void *)(p ? p + 1 :  NULL );
}

/*------------------------------------------------------------------------------*/

  void  *nextsym(HASH_TAB *tabp,void *i_last )
{
   /* Return a pointer the next node in the current chain that has the same
    * key as the last node found(or NULL if there is no such node). "last"
    * is a pointer returned from a previous findsym() of nextsym() call.
    */

    BUCKET *last = (BUCKET *)i_last;

    for(--last; last->next ; last = last->next )
             if( (tabp->cmp)(last+1, last->next + 1) == 0 )   /* keys match  */
                     return(_TCHAR *)(last->next + 1 );
    return NULL;
}


/* Printing the table        Page 721                                           */

static  int      (*User_cmp)();

  _TINT ptab(HASH_TAB  *tabp,void (* print)(),void *param,_TINT sort )
/*HASH_TAB        *tabp;               /* Pointer to the table                   */
/*void            (* print)();         /* Print function used for output         */
/*void            *param;              /* Parameter passed to print function     */
/*_TINT             sort;                /* Sort the table if true                 */
{
    
     /* Return 0 if a sorted table can't be printed because of insufficent
      * memory, else return 1 if the table was printed. The print function
      * is called with two arguments:
      *           (*print)( sym, param )
      *
      * Sym is a pointer to a BUCKET user area and param is the third
      * argument to ptab.
      */

     BUCKET        **outtab, **outp, *sym, **symtab ;
     _TINT           internal_cmp();
     int             i;  /* Must be signed for loop termination! */

     if( !tabp || tabp->size == 0 )          /* Table is empty */
         return 1;

     if ( ! sort )
     {
          for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
          {
     /* Print all symbols in the current chain. The +1 in the print call
      * increments the pointer to the application area of the bucket
      */
            for( sym = *symtab ; sym ; sym = sym->next )
               (*print)( sym+1, param );
          }
     }
     else
     {
         /*     Allocate memory for the outtab, an array of pointers to
          *     BUCKETs, and initialize it. The outtab is different from
          *     the actual hash table in that every outtab element points
          *     to a single   BUCKET structure, rather than to a linked list
          *     of them.
          */
          size_t alloc_size = (size_t)tabp->numsyms * sizeof(BUCKET*);

          if( !( outtab = (BUCKET **)
		         AllocMem((DWORD)alloc_size) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     _ftprintf(stderr,_T("Internal error [ptab], table overflow\n"));
                     exit(1);
                 }
                 *outp++ = sym;
            }

       /*    Sort the outtab and then print it. The (*outp) +1 in the
        *    print call increments the pointer past the header part
        *    of the BUCKET structure. During sorting, the increment
        *    is done in internal_cmp.
        */

        User_cmp = tabp->cmp;
        assort( outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
        {
            void *user_ptr = (void*)((*outp)+1);
            /* Use SEH to handle potential access violations in callback */
            __try {
                (*print)( user_ptr, param );
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                /* Exception occurred but we continue processing */
            }
        }

        FreeMem( outtab );
   }
   return 1 ;
}

static _TINT internal_cmp(BUCKET **p1,BUCKET **p2 )
/*BUCKET **p1, **p2;*/
{
 return  (  (*User_cmp)(*p1 + 1, *p2 + 1 ) );
}
