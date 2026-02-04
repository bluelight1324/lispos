/* has h.c -- BUCKET Allocation Page 716              */

//#include <windows.h>
#include <standard.h>
#include <stdio.h>
#include <signal.h>
#include "debug.h"
#include "set.h"
#include "hash.h"
#include "yystate.h"
#include <virtualmem.h>    /* memory-management functions */


extern  void  assort(void  **base,int  nel,int  elsize,int (*cmp)(BUCKET **,BUCKET **) );
static int internal_cmp(BUCKET **,BUCKET **);


  void *newsym(int size )
{
   /* Allocate space for a new symbol; return a pointer to the user space  */

   BUCKET    *sym;

   if( !(sym = (BUCKET *) AllocMem( size + sizeof(BUCKET) + 1)) )
   /* AllocMem is used above instead of calloc originally */
   {
       fprintf( stderr, "Can't get memory for BUCKET\n" );
       raise( SIGABRT );
       return NULL;
    }
    return (void *)( sym + 1 );      /* return pointer to user space    */
}

/*------------------------------------------------------------------------------*/

 void freesym(void *sym)
{
   FreeMem( (BUCKET *)sym - 1 );
}


 HASH_TAB *maketab(unsigned maxsym,unsigned (*hash_function)(SYMBOL *),
                   int (*cmp_function)(const char *,const char *)
				  )
{

  /* Make a hash table of the indicated size     */

  HASH_TAB    *p;

  if( !maxsym )
      maxsym = 127;
                        /* |<--- space for table --->|<- and header -->| */
  if( p=(HASH_TAB*)  AllocMem(maxsym * sizeof(BUCKET*) + sizeof(HASH_TAB)) )

  /* AllocMem is used above instead of calloc originally */
  {	  unsigned int count;

	  for (count = 0; count <= maxsym; count++)
	  p->table[count] = (BUCKET *)AllocMem(sizeof(BUCKET*));

      p->size     = maxsym        ;
      p->numsyms  = 0             ;
      p->hash     = (unsigned int (*)(void*))hash_function ;
      p->cmp      = (int (__cdecl *)(const void *,const void *))cmp_function;
  }
  else
  {
      fprintf(stderr, "Insufficent memory for symbol table\n");
      raise( SIGABRT );
      return (HASH_TAB *)NULL;
  }
  return p;
}

//#if TT
  HASH_TAB *maketab(unsigned maxsym,unsigned (*hash_function)(STATE *),
                   int (*cmp_function)(STATE *,STATE *)
				  )
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
      p->hash     = (unsigned int (*)(void*))hash_function ;
      p->cmp      = (int (__cdecl *)(const void *,const void *))cmp_function;
  }
  else
  {
      fprintf(stderr, "Insufficent memory for symbol table\n");
      raise( SIGABRT );
      return (HASH_TAB *)NULL;
  }
  return p;
}

//#endif

/* Adding a symbol to the table Pg. 718                                       */

  void *addsym(HASH_TAB *tabp,void *isym )
{
    /* Add a symbol to the hash table                                         */

    BUCKET   **p, *tmp ;
    BUCKET   *sym = (BUCKET *)isym;

    p = &(tabp->table)[ (*tabp->hash)((UCHAR *) sym-- ) % tabp->size ];

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
int strcmpt(const char *arg1,const char *arg2)
{ //this fn. for debugging only
 //cout << arg1,arg2;
 return strcmp(arg1,arg2);
}

  void    *findsym(HASH_TAB *tabp,void *sym)
{
    /* Return a pointer to the hash table element having a particular name
     * or NULL if the name isn't in the table.
     */

     BUCKET     *p,**q ;

     if( !tabp )                 /* Table empty   */
         return NULL;
	 int i,j,k;

	 i = tabp->size;
	 j = ((*tabp->hash)((UCHAR *)sym));
	 k = j % i;
	 p = (tabp->table)[k];
     //p =(BUCKET *) (tabp->table)[ ((*tabp->hash)((UCHAR *)sym)) % tabp->size ];
/*	 while (p)
	 {
		 try{
	  if (p->next != NULL)
		  p = p->next;
		 q = p;}
	   catch(...)
	   {
		   q = q->prev;
		   q =  q->prev;
		   break;}
	 }*/

	 
	  while(1)
	  {	 try{
		  //if ( p && (*tabp->cmp)( sym, p+1 ) )
		  if ( p && strcmpt((const char *)sym,(const char *) p+1 ) )
			  p = p->next;}
	 catch (...)
	 {
	  p = NULL;
	  break;
	 }
	  }
//     while( p && strcmpt( sym, p+1 ) )
//            p = p->next;



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
                     return(char *)(last->next + 1 );
    return NULL;
}


/* Printing the table        Page 721                                           */

static  int      (*User_cmp)(const void*,const void *);

int ptab(HASH_TAB  *tabp,int (* print)(void *,void *),void *param,int sort )
/*HASH_TAB        *tabp;               /* Pointer to the table                   */
/*void            (* print)();         /* Print function used for output         */
/*void            *param;              /* Parameter passed to print function     */
/*int             sort;                /* Sort the table if true                 */
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
     int           internal_cmp(BUCKET **,BUCKET **);
     int           i;

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

          if( !( outtab = (BUCKET **) 
		         AllocMem(tabp->numsyms * sizeof(BUCKET*)) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     fprintf(stderr,"Internal error [ptab], table overflow\n");
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
        assort( (void**)outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*print)( (*outp)+1, param );

        FreeMem( outtab );
   }
   return 1 ;
}



/* overloaded  ptab functions */
int ptab(HASH_TAB *tabp,void (* print)(SYMBOL *,FILE *),void *param,int sort)
/*HASH_TAB        *tabp;               /* Pointer to the table                   */
/*void            (* print)();         /* Print function used for output         */
/*void            *param;              /* Parameter passed to print function     */
/*int             sort;                /* Sort the table if true                 */
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
     int           internal_cmp(BUCKET **,BUCKET **);
     int           i;

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
               (*print)((SYMBOL *) sym+1,(FILE *)param );
              
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

          if( !( outtab = (BUCKET **) 
		         AllocMem(tabp->numsyms * sizeof(BUCKET*)) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     fprintf(stderr,"Internal error [ptab], table overflow\n");
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
        assort( (void**)outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*print)( ((SYMBOL *)*outp)+1,(FILE *) param );

        FreeMem( outtab );
   }
   return 1 ;
}



int   ptab( HASH_TAB *tabp, int(*prnt)(SYMBOL *), void *param, int sort)
/*HASH_TAB        *tabp;               /* Pointer to the table                   */
/*void            (* print)();         /* Print function used for output         */
/*void            *param;              /* Parameter passed to print function     */
/*int             sort;                /* Sort the table if true                 */
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
     int           internal_cmp(BUCKET **,BUCKET **);
     int           i;

     if( !tabp || tabp->size == 0 )          /* Table is empty */
         return 1;

     if ( ! sort )
     {
          for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
          {
     /* Print all symbols in the current chain. The +1 in the print call
      * increments the pointer to the application area of the bucket
      */
/*            for( sym = *symtab ; sym ; sym = sym->next )
               (*print)((SYMBOL *) sym+1,(FILE *)param );*/
               for( sym = *symtab ; sym ; sym = sym->next )
                  (*prnt)((SYMBOL *) sym+1);//,(FILE *)param );
              
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

          if( !( outtab = (BUCKET **) 
		         AllocMem(tabp->numsyms * sizeof(BUCKET*)) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     fprintf(stderr,"Internal error [ptab], table overflow\n");
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
        assort( (void**)outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

/*        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*print)( ((SYMBOL *)*outp)+1,(FILE *) param );*/
        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*prnt)( ((SYMBOL *)*outp)+1);//,(FILE *) param );


        FreeMem( outtab );
   }
   return 1 ;
}



int  ptab(HASH_TAB *tabp,void (*prnt)(SYMBOL *,PRODUCTION **),void *param,int sort)
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
     int           internal_cmp(BUCKET **,BUCKET **);
     int           i;

     if( !tabp || tabp->size == 0 )          /* Table is empty */
         return 1;

     if ( ! sort )
     {
          for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
          {
     /* Print all symbols in the current chain. The +1 in the print call
      * increments the pointer to the application area of the bucket
      */
/*            for( sym = *symtab ; sym ; sym = sym->next )
               (*print)((SYMBOL *) sym+1,(FILE *)param );*/
               for( sym = *symtab ; sym ; sym = sym->next )
                  (*prnt)((SYMBOL *) sym+1,(PRODUCTION **)param); //,(FILE *)param );
              
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

          if( !( outtab = (BUCKET **) 
		         AllocMem(tabp->numsyms * sizeof(BUCKET*)) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     fprintf(stderr,"Internal error [ptab], table overflow\n");
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
        assort( (void**)outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

/*        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*print)( ((SYMBOL *)*outp)+1,(FILE *) param );*/
        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*prnt)( ((SYMBOL *)*outp)+1,(PRODUCTION **)param);//,(FILE *) param );


        FreeMem( outtab );
   }
   return 1 ;
}



int  ptab(HASH_TAB *tabp,void (*prnt )(STATE *,void*),void *param,int sort)
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
     int           internal_cmp(BUCKET **,BUCKET **);
     int           i;

     if( !tabp || tabp->size == 0 )          /* Table is empty */
         return 1;

     if ( ! sort )
     {
          for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
          {
     /* Print all symbols in the current chain. The +1 in the print call
      * increments the pointer to the application area of the bucket
      */
/*            for( sym = *symtab ; sym ; sym = sym->next )
               (*print)((SYMBOL *) sym+1,(FILE *)param );*/
               for( sym = *symtab ; sym ; sym = sym->next )
                  (*prnt)((STATE *) sym+1,(void *) param); //,(FILE *)param );
              
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

          if( !( outtab = (BUCKET **) 
		         AllocMem(tabp->numsyms * sizeof(BUCKET*)) )
			   )
		  /* AllocMem is used above instead of malloc originally */
               return 0;

         outp = outtab;

         for( symtab = tabp->table, i = tabp->size ; --i >= 0 ; symtab++ )
             for( sym = *symtab ; sym ; sym = sym->next )
             {
                 if( outp > outtab + tabp->numsyms )
                 {
                     fprintf(stderr,"Internal error [ptab], table overflow\n");
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
        assort( (void**)outtab, tabp->numsyms, sizeof( BUCKET *), internal_cmp );

/*        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*print)( ((SYMBOL *)*outp)+1,(FILE *) param );*/
        for( outp = outtab, i = tabp->numsyms; --i >= 0; outp++ )
            (*prnt)( ((STATE *)*outp)+1,(void *)param);//,(FILE *) param );


        FreeMem( outtab );
   }
   return 1 ;
}

 
static int internal_cmp(BUCKET **p1,BUCKET **p2 )
//static int internal_cmp(void **p1,void **p2 )
/*BUCKET **p1, **p2;*/
{
 return  (  (*User_cmp)(*p1 + 1, *p2 + 1 ) );
}
