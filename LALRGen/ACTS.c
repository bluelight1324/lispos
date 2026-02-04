/* acts.c   Page 290 Action routines used by both llama and occs. These build
 * up the symbol table from the input specification
 */

#pragma warning(disable:4115) 
//#pragma warning(disable:4244) 
#pragma warning(disable:4706) 
//#pragma warning(disable:4057) 

#include <windows.h>
//#include <standard.h>
//#include <tchar.h>


#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "debug.h"
#include "set.h"
#include "hash.h"               
#include "stack.h"                 /* stack-manipulation macros     */
#include "llout.h"
#include <virtualmem.h>            /* memory-management functions */

/*
#ifdef   stack_cls
#undef   stack_cls                     // Make all stacks static        
#define  stack_cls static
#endif
*/

void    find_problems     P(( SYMBOL  *sym, void *junk              ));  /* local */
_TINT     c_identifier      P(( _TCHAR *name                            ));
void    print_tok         P(( FILE *stream, _TCHAR *format, _TINT arg   ));  /* public */
void    pterm             P(( SYMBOL  *sym, FILE *stream            ));
void    pact              P(( SYMBOL  *sym, FILE *stream            ));
void    pnonterm          P(( SYMBOL  *sym, FILE *stream            ));
void    print_symbols     P(( FILE    *stream                       ));
_TINT     problems          P(( void                                  ));
void    init_acts         P(( void                                  ));
SYMBOL  *make_term        P(( _TCHAR *name                            ));
void    first_sym         P(( void                                  ));
SYMBOL  *new_nonterm      P(( _TCHAR *name, _TINT is_lhs                ));
void    new_rhs           P(( void                                  ));
void    add_to_rhs        P(( _TCHAR *object, _TINT is_an_action        ));
void    start_opt         P(( _TCHAR *lex                             ));
void    end_opt           P(( _TCHAR *lex                             ));
void    add_synch         P(( _TCHAR *yytext                          ));
void    new_lev           P(( _TINT how                               ));
void    prec_list         P(( _TCHAR *name                            ));
void    prec              P(( _TCHAR *name                            ));
void    union_def         P(( _TCHAR *action                          ));
_TINT     fields_active     P(( void                                  ));
void    new_field         P(( _TCHAR *field_name                      ));
//extern HASH_TAB  *maketab P(( unsigned maxsym, unsigned (*hash)(SYMBOL *), _TINT(*cmp) (const _TCHAR *,const _TCHAR *)));
//extern HASH_TAB  *maketab P(( unsigned maxsym, unsigned (*hash)(void *), _TINT(*cmp) (const TCHAR *,const TCHAR *)));
 /*HASH_TAB *maketab(unsigned maxsym,unsigned (*hash_function)(void *),
                   _TINT (*cmp_function)(const void *,const void *)
				  );*/

/* *************** external definitions ************************** */
extern   void   fputstr( _TCHAR *str,_TINT maxlen,FILE *stream );
extern   _TINT  FError(_TCHAR *fmt, ... );
extern    void error(_TINT ,_TCHAR *, ... );
extern    void output(_TCHAR *, ... );
extern    void lerror(_TINT,_TCHAR *, ... );
extern    _TINT ptab(HASH_TAB *,void (* print)(SYMBOL *,FILE *),void *param,_TINT sort);

/********************************************************************** */




/* _TCHAR    *yytext    = "";*/ /* Lexeme (not'\0' terminated)             */
/* _TINT 	yylineno   = 0; */  /* Input line number                       */
/* Above found in lex.c pg. 15 added by MN                          */

/*------------------------------------------------------------------------------*/
extern  _TINT     yynerrs;                 /* Total error count      */
extern  _TINT     yylineno;                /* Input line number -- created by Lex  */
static _TINT     Associativity;           /* Current associativity direction      */
static _TINT     Prec_lev = 0;            /* Precedence level. Incremented after  */
                                         /* after finding  %left, etc.,          */
                                         /* but before the names are done        */
static _TCHAR   Field_name[NAME_MAX]; 			     /* Field name specified in <name>       */
static _TINT    Fields_active = 0;        /* Fields are used in the input.        */
                                         /* (If they're not, then automatic      */
                                         /* field-name generation, as per        */
                                         /* %union, is not activated. )          */
static _TINT    Goal_symbol_is_next = 0;  /* If true, the next nonterminal is     */
                                         /* the goal symbol                      */

/*------------------------------------------------------------------------------
 * The following stuff is used for processing nested optional
 * (or repeating) productions defined with the occs [] and []* operators.
 * A stack is kept and,every time you go down another layer of nesting, 
 * the current nonterminal is stacked and an new nonterminal is allocated.
 * The previous state is restored when you're done with a level.
 */
 
 #define SSIZE   8            /* max. optional-production nesting level     */
 typedef struct  _cur_sym_
 {
    _TCHAR        lhs_name[NAME_MAX];  /* Name associated with left-hand side.  */
    SYMBOL      *lhs;                /* Pointer to symbol-table entry for     */
                                     /*  the current left-hand side           */
    PRODUCTION  *rhs;                /* Pointer to current production         */
 } CUR_SYM;
 


 CUR_SYM Stack[ SSIZE ],            /* Stack and                               */         
         *Sp = Stack + (SSIZE-1);   /* stack pointer. It's inconvenient to use */
                                    /* stack.h because stack is of structures  */

_TINT c_identifier(_TCHAR *name )		/* Return true only if name is  */
				                    /* a legitimate C identifier.	*/
{
    if( _istdigit( *name ) )
	return 0;

    for(; *name ; ++name )
	if( !( isalnum(*name) || *name == '_' ))
	    return 0;

    return 1;
}


/*===============================================================================
 * Support routines for actions
 */


  void print_tok(FILE *stream,_TCHAR *format,_TINT arg )
/* _TCHAR    *format;    not used here but supplied by pset()  */
{
    /* Print one nonterminal symbol to the specified stream   */

    if     ( arg == -1      ) fprintf( stream, "null "                       );
    else if( arg == -2      ) fprintf( stream, "empty "                      );
    else if( arg == _EOI_   ) fprintf( stream, "$ "                          );
    else if( arg == EPSILON ) fprintf( stream, "<epsilon>"                   );
    else                      fprintf(stream, "%s ", Terms[arg]->name        );

 }
/*---------------------------------------------------------------------------------
 * The following three routines print the symbol table. Pterm(), pact(), and 
 * pnonterm() are called indirectly through ptab(), called in print_symbols().
 * They print the terminal, action and nonterminal symbols from the symbol table,
 * respectively
 */

   void pterm(SYMBOL *sym,FILE *stream )
 {

     _TINT  i;

     if( !ISTERM(sym) )
         return;

     LL( fprintf( stream, "%-16.16s  %3d\n", sym->name, sym->val );)
     OX( fprintf( stream, "%-16.16s  %3d    %2d     %c     <%s>\n",\
                                  sym->name,\
                                  sym->val,\
                                  Precedence[sym->val].level,\
                                  (i = Precedence[sym->val].assoc) ? i : '-',\
                                  sym->field ); )
  }

  /*----------------------------------------------------------------------------*/

    void pact(SYMBOL *sym,FILE *stream )
  {
      if( !ISACT(sym) )
              return;

       fprintf( stream, "%-5s   %3d,\n",   sym->name, sym->val );
       fprintf( stream, " line %-3d: \n", sym->lineno        );
       //fputstr( sym->string, 55, stream                    );
       fprintf( stream, "%5s", sym->string                  ); /* CC temp */
       fprintf( stream, "\n" );
 }

 /*------------------------------------------------------------------------------*/

   _TCHAR *production_str(PRODUCTION   *prod )
 
 {
    /* return a string representing the production        */

    _TINT          i, nchars,  avail;
    static _TCHAR  buf[80];  /* Original : static _TCHAR  buf[80]; */
    _TCHAR         *p;

	nchars =_stprintf(buf,_T("%s ->"), prod->lhs->name );
    p      = buf + nchars;
    avail  = sizeof(buf) - nchars - 1;

    if( !prod->rhs_len )
        _stprintf(p, _T(" (epsilon)") );
    else
       for( i = 0; i < prod->rhs_len && avail > 0 ; ++i )
       {
           nchars = _stprintf(p,_T(" %0.*s"), avail-2, prod->rhs[i]->name );
           avail -= nchars;
           p     += nchars;
       }
    return buf;
 }

 /*-----------------------------------------------------------------------------*/

   void pnonterm(SYMBOL *sym,FILE *stream )
 {
  /* _TINT   i;	  unreferenced variable
     _TCHAR  *s;    unreferenced variable 
  */
     PRODUCTION *p;
     _TINT        chars_printed;

     stack_dcl( pstack, PRODUCTION *, MAXPROD );

     if( !ISNONTERM(sym) )
             return;

     fprintf( stream, "%s (%3d) %s", sym->name, sym->val, 
                                sym == Goal_symbol ?  "(goal symbol)" : "" );

     OX( fprintf( stream, " <%s>\n", sym->field );              )
     LL( fprintf( stream, "\n" );                               )

     if( Symbols > 1 )
     {
        /* Print first and follow sets only if you want really verbose output */

        fprintf( stream, "   FIRST : " );
        pset_acts( sym->first, print_tok, stream ); /* CC */
		/* pset_acts() is == pset()	,but modified to take a parameter of
		 * the type print_tok
		 */
        LL(   fprintf(stream, "\n  FOLLOW:  ");                 )
        LL(   pset( sym->follow, print_tok, stream);            )
        fprintf(stream,"\n");
     }

     /* Productions are put into the SYMBOL in reverse order because it's 
      * easier to tack them onto the beginning of the linked list. It's 
      * better to print them in forward order, however, to make the symbol 
      * table more readable.Solve this problem by stacking all the 
      * productions and then popping elements to print them. Since 
      * the pstack has MAXPROD elements, it's not necessary to test for
      * stack overflow on a  push.
      */

      for( p = sym->productions ; p ; p = p->next )
           push( pstack, p );

      while( !stack_empty( pstack ) )
      {
          p      = pop(pstack);

          chars_printed = fprintf(stream, "   %3d: %s",
                                                 p->num, production_str( p ));

          LL(  for( ; chars_printed <=45; ++chars_printed )            )
          LL(       putc( '.', stream);                                )
          LL(  fprintf(stream, "SELECT: ");                            )
          LL(  pset(p->select, print_tok, stream );                    )

          OX( if( p->prec )                                            )
          OX( {                                                        )
          OX(   for( ; chars_printed <= 60; ++chars_printed )          )
          OX(        putc( '.', stream);                               )
          OX(   if( p->prec )                                          )
          OX(       fprintf(stream, "PREC %d", p->prec );              )
          OX( }                                                        )

          putc('\n', stream);
     }

     fprintf(stream, "\n");
 }

 /*-----------------------------------------------------------------------------*/
#ifdef _DEBUG
   void print_symbols(FILE *stream )
 {
/* Print out the symbol table. Nonterminal symbols come first for the sake
 * of the 's' option in yydebug(); symbols other than production numbers can
 * be entered symbolically. ptab returns 0 if it can't print the symbols
 * sorted (because there's no memory. If this is the case, try again but
 * print the table unsorted).
 */

       putc( '\n', stream );

       fprintf( stream, "---------------------- Symbol table --------------------\n" );
       fprintf( stream, "\nNONTERMINAL SYMBOLS:\n\n" );
       if( ptab( Symtab, pnonterm, stream, 1 ) == 0 )
           ptab( Symtab, pnonterm, stream, 0 );
       fprintf( stream, "\nTERMINAL SYMBOLS:\n\n");
       OX( fprintf( stream, "name              value   prec  assoc    field\n"); )
       LL( fprintf( stream, "name              value\n");                        )

       if( ptab( Symtab, pterm, stream, 1 ) == 0 )
           ptab( Symtab, pterm, stream, 0 );

       OX( fprintf( stream, "\nACTION SYMBOLS:\n\n");                            )
       /* OX(if( ptab( Symtab, pact, stream, 1 ) == 0 )                          ) :CC -*/
       OX(ptab( Symtab, pact, stream, 1 );                                       ) 

       LL( fprintf( stream, "\nACTION SYMBOLS:\n\n");                            )
       LL( if( !ptab( Symtab, pact, stream, 1) )                                 )
       LL(      ptab( Symtab, pact, stream, 0 );                                 )
       LL( fprintf( stream, "---------------------------------------------\n" ); )
 }
 
#endif
/*-------------------------------------------------------------------------------
 * Problems() and find_problems work together to find unused symbols and symbols
 * that are used but not defined
 */
  /* Change from void to FILE */
 void find_problems(SYMBOL *sym,FILE *junk )

 {
	/* This routine has been modified  from the original OCCS routine */

    
     if( !sym->used && sym!=Goal_symbol )
        error( WARNING, _T("<%s> not used ( defined on line %d)\n"),
                                                    sym->name, sym-> set );
     if( !sym->set && !ISACT(sym) )
         error( NONFATAL, _T("<%s> not defined (used on line %d)\n"),
                                            sym->name, sym->used ); 


 }
      
         
   _TINT problems()
 {
     /* Find, and print an error message, for all symbols that are used but not
      * defined, and for all symbols that are defined but not used. Return the
      * number of errors after checking.
      */

      ptab( Symtab, find_problems, NULL, 0 );
      return yynerrs;
 }


 /*-----------------------------------------------------------------------------*/

_TINT hash_funct(SYMBOL *p ) 
 
 {
      if( !*p->name )
          lerror( FATAL, _T("Illegal empty symbol name\n") );

      return hash_add((_TUCHAR *) p->name ); 
 }

   void  init_acts()
 {
    /* Various initializations that can't be done at compile time. Call this
     * routine before starting up the parser. The hash-table size (157) is an
     * arbitrary prime number, roughly the number symbols expected in the
     * table
     */

     //extern _TINT     strcmp();
     static SYMBOL  bogus_symbol;

     _tcscpy( bogus_symbol.name,_T("End of Input") );

     Terms[0]        = &bogus_symbol;
     Symtab          = maketab( MAXPROD/4,hash_funct,_tcscmp );
     _tcsncpy( Sp->lhs_name, _T("garbage"), NAME_MAX );

     LL( Synch       = newset();  )
}

/*-------------------------------------------------------------------------------*/

   SYMBOL  *make_term(_TCHAR *name ) /* Make a terminal symbol */  
{
    SYMBOL       *p;

/* Below code was commented out */

 /*   if( !c_identifier(name) )
        lerror(NONFATAL, _T("Token names must be legitimate C identifiers\n"));

    else  */

/* Above code was commented out  */
 
    
    if( p = (SYMBOL *)findsym((HASH_TAB *)Symtab, name) )
         lerror( WARNING, _T("Terminal symbol <%s> already declared\n"), name );
    else
    {
        if( Cur_term >= MAXTERM )
            lerror(FATAL,_T("Too many terminal symbols (%d max.).\n"), MAXTERM );

        p = (SYMBOL *) newsym( sizeof(SYMBOL) );
        //strncpy( p->name, name,         NAME_MAX );
		_tcsncpy( p->name, name,_tcslen(name) + 1);
        //strncpy( p->field, Field_name,  NAME_MAX );
		_tcsncpy( p->field, Field_name,_tcslen(Field_name) +1);
        addsym ( Symtab, p );

        p->val = ++Cur_term ;
        p->set = yylineno;

        Terms[Cur_term] = p;
   }
   return p;
}


/*--------------------------------------------------------------------------------*/


  void       first_sym()
{
    /* This routine is called just before the first rule following the %%.
     * It's used to point out the goal symbol.
     */

     Goal_symbol_is_next = 1;
 }

 /*----------------------------------------------------------------------------*/

   SYMBOL   *new_nonterm(_TCHAR *name,_TINT is_lhs )
 {
/* Create, and initialize a new nonterminal. is_lhs is used to
 * differentiate between implicit and explicit declarations. It's 0 if the
 * nonterminal is added because it was found on a right-hand side. It's 1 if
 * the nonterminal is on a left-hand side.
 *
 * Return a pointer to the new symbol or NULL if an attempt is made to use a
 * terminal symbol on a left-hand side.
 */

      SYMBOL    *p;

	  
      if( p = (SYMBOL *) findsym( Symtab, name ) )
      {
          if( !ISNONTERM( p ) )
          {
              lerror(NONFATAL, _T("Symbol on left-hand side must be nonterminal\n"));
              p = NULL;
          }
      }
      else if( Cur_nonterm >= MAXNONTERM )
      {
          lerror(FATAL,_T("Too many nonterminal symbols (%d max.).\n"), MAXTERM );
      }
      else                                 /* Add new nonterminal to symbol table */
      {
         p = (SYMBOL *) newsym( sizeof(SYMBOL) );
         //strncpy( p->name, name,          NAME_MAX );
		 _tcsncpy( p->name, name,_tcslen(name) + 1);
         //strncpy( p->field, Field_name,   NAME_MAX );
		 _tcsncpy( p->field, Field_name,_tcslen(Field_name) + 1);

         p->val = ++Cur_nonterm ;
         Terms[Cur_nonterm] = p;

         addsym  ( Symtab, p );
      }

      if( p )                              /* (re)initialize new nonterminal   */
      {
          if( Goal_symbol_is_next )
          {
              Goal_symbol = p;
              Goal_symbol_is_next = 0;
          }
          if( !p->first )
               p->first = newset();

 LL(     if( !p->follow )                               )
 LL(          p->follow = newset();                     )

         p->lineno = yylineno;

         if( is_lhs )
         {

            //strncpy( Sp->lhs_name, name, NAME_MAX );
			_tcsncpy( Sp->lhs_name, name,_tcslen(name) + 1 );
            Sp->lhs        = p;
            Sp->rhs        = NULL;
            Sp->lhs->set   = yylineno;
         }
      }

      return p;
 }

 /*------------------------------------------------------------------------------*/

   void new_rhs()
 {
/* Get a new PRODUCTION and link it to the head of the production chain.
 * of the current nonterminal. Note that the start production MUST be 
 * production 0. As a consequence, the first rhs associated with the first
 * nonterminal MUST be the start production. Num_productions is initialized
 * to 0 when it's declared.
 */

     PRODUCTION  *p;

     if( !(p = (PRODUCTION *) AllocMem(10 * sizeof(PRODUCTION))) )
         lerror(FATAL,_T( "Out of memory\n") );

     p->next                 = Sp->lhs->productions;
     Sp->lhs->productions    = p;

     LL( p->select = newset(); )

     if( (p->num = Num_productions++) >= MAXPROD )
          lerror(FATAL, _T("Too many productions( %d max.)\n"), MAXPROD );

     p->lhs  = Sp->lhs;
     Sp->rhs = p;
 }

 /*-----------------------------------------------------------------------------*/

   void add_to_rhs(_TCHAR *object,_TINT is_an_action )
 /*  0 if not an action, line number otherwise */
 {
    SYMBOL       *p;
    _TINT i;
    _TCHAR         buf[32];    /* Original : _TCHAR         buf[32];*/

/* Add a new element to the RHS currently at top of stack.First deal with
 * forward references. If the item isn't in the table, add it. Note that,
 * since terminal symbols must be declared with %term directive, forward
 * references always refer to nonterminals or action items. When we exit the
 * if statement, P points at the symbol table entry for the current object.
 */
     
     if( !(p = (SYMBOL *) findsym( Symtab, object)) )      /* not in tab yet  */
     {
         if( !is_an_action )
         {
             if( !(p = new_nonterm( object, 0 )) )
             {
                  /* Won't get here unless p is a terminal symbol */

                  lerror(FATAL,_T("(internal) Unexpected terminal symbol\n"));
                  return;
             }
        }
        else
        {
     /* Add an action. All actions are named "{DDD}" where DDD is  the 
      * action number. The curly brace in the name guarantees that this
      * name won't conflict with a normal name. I am assuming that calloc
      * is used to allocate memory ,for the new node (i.e. that it's
      * initialized to zeros)
      */
            _stprintf(buf, _T("{%d}"), ++Cur_act - MINACT );

            p = (SYMBOL *) newsym( sizeof(SYMBOL) );
            _tcsncpy ( p->name, buf, _tcslen(buf) + 1 );
            addsym  ( Symtab, p );

            p->val    = Cur_act;
            p->lineno = is_an_action;

           /* below code added */
			/*if (strspn(object,"385821001664980494" ) != 0)
			{
			 Beep(3000,500);Sleep(12000);
			} */

            p->string = ( _TCHAR *)AllocMem((DWORD)(_tcslen(object) * sizeof( _TCHAR) * 10));
			if(!p->string)
                 lerror(FATAL, _T("Insufficient memory to save action\n"));
			/* above code added */
			/* This line has been added because	the original line is 
			 *defunct
			 */
			_tcscpy((_TCHAR *)p->string,object); 
			/*if (strcmp(p->string,"{ return ""385821001664980494""; }") == 0)
			{
			 Beep(3000,500);Sleep(12000);
			}
			if (p->string[0] != '{')
			{
			 Beep(3000,500);Sleep(12000);
			} */
			                        

        }
   }

   p->used = yylineno;

   if( (i = Sp->rhs->rhs_len++) >= MAXRHS )
        lerror(NONFATAL, _T("Right-hand side too long (%d max)\n"), MAXRHS );
   else
   {
      LL(  if( i== 0  &&  p == Sp->lhs ) )
      LL(      lerror(NONFATAL, "Illegal left recursion in production.\n"); )
      OX(  if( ISTERM( p ) )  )
      OX(      Sp->rhs->prec = Precedence[ p->val ].level ;  )

      Sp->rhs->rhs[ i   ]    = p;
      Sp->rhs->rhs[ i + 1  ] = NULL;          /* NULL terminate the array */

      if( !ISACT(p) )
          ++( Sp->rhs->non_acts );
    }
 }


 /*------------------------------------------------------------------------------
  * The next two subroutines handle repeating or optional subexpressions. The
  * following mappings are done, depending on the operator:
  *
  * S : A [B]  C ;            S   -> A 001  C
  *                           001 -> B | epsilon
  *
  * S : A [B]* C ;            S   -> A 001  C                     (occs)
  *                           001 -> 001 B | epsilon
  *
  * S : A [B]* C;             S   -> A 001  C                      (llama)
  *                           001 -> B 001 | epsilon
  *
  * In all situations , the right hand side that we've collected so far is
  * pushed and a new right-hand side is started for the subexpression. Note that
  * the first character of the created rhs name(001 in the previous examples)
  * is a space, which is illegal in a user-supplied production name so we don't
  * have to worry about conflicts. Subsequent symbols are added to this new
  * right-hand side. When the ), ], or *) is found, we finish the new right-hand
  * side, pop the stack and add the name of the new right-hand side to the
  * previously collected left-hand side.
  */

    void start_opt(_TCHAR *lex )     /* Start an optional subexpression */
  {
         _TCHAR  name[32];     /* Original : _TCHAR  name[32];*/
         static _TINT num = 0;

        
         --Sp;                           /* Push current stack element   */
         _stprintf( name, _T(" %06d"), num++); /* Make name for new production */
         new_nonterm( name, 1 );         /* Create a nonterminal for it  */
         new_rhs();                      /* Create epsilon production    */
         new_rhs();                      /* and production for sub-prod  */
  }

  /*----------------------------------------------------------------------------*/

    void end_opt(_TCHAR *lex )                    /* end optional subexpression */
  {
        _TCHAR    *name = Sp->lhs_name;
		SYMBOL  *p;
		_TINT i;

        if( lex[1] == '*' )                   /* Process a [...]*                */
        {
            add_to_rhs( name, 0 );            /* Add right-recursive reference   */
            
#ifdef OCCS                                   /* If occs, must be left recursive */   
        i = Sp->rhs->rhs_len - 1;             /* Shuffle things around.         */
        p = Sp->rhs->rhs[ i ];
        memmove( &(Sp->rhs->rhs)[1], &(Sp->rhs->rhs)[0],
                                     i * sizeof( (Sp->rhs->rhs)[1] ) );
        Sp->rhs->rhs[ 0 ] = p;
#endif
    }
    ++Sp;                                       /* discard top-of-stack element */
    add_to_rhs( name, 0);
}                                                 

/*================================================================================
 * The following routines have alternate versions, one set for llama and another
 * for occs. The routines corresponding to features that aren;t supported in one
 * or the other of these programs print error messages.
 */

 #ifdef LLAMA

   void add_synch( name )
 _TCHAR    *name;
 {
     /* Add "name" to the set of synchronization tokens
     */

     SYMBOL      *p;

     if( !(p = (SYMBOL *) findsym( Symtab, name )) )
         lerror(NONFATAL,"%%synch: undeclared symbol <%s>.\n", name );

     else if( !ISTERM(p) )
          lerror(NONFATAL,"%%synch: <%s> not a terminal symbol\n", name );

     else
        ADD( Synch, p->val );
  }

  /*---------------------------------------------------------------------------*/

    void new_lev( how )
  {
      switch( how )
      {
      case  0 :  /* initialization ignore it */                             break;
      case 'l':  lerror (NONFATAL, "%%left not recognized by LLAMA\n"    ); break;
      case 'r':  lerror (NONFATAL, "%%right not recognized by LLAMA\n"   ); break;
      default :  lerror (NONFATAL, "%%nonassoc not recognized by LLAMA\n"); break;
      }
  }


   void prec( name )
  _TCHAR *name;
  {
       lerror( NONFATAL, "%%prec not recognized by LLAMA\n" );
  }

    void union_def( action )
  _TCHAR    *action;
  {
      lerror(NONFATAL,"%%union not recognized by LLAMA\n");
  }

   void prec_list( name ) _TCHAR *name;
  {
  }

   void new_field( field_name )
  _TCHAR   *field_name;
  {
     if( *field_name )
         lerror(NONFATAL, "<name> not supported by LLAMA \n");
  }

    make_nonterm(name)
  _TCHAR    *name;
  {
       lerror(NONFATAL, "%type not supported by LLAMA\n");
  }

  #else /*=======================================================================*/

    void add_synch(_TCHAR *yytext)
  {
     lerror(NONFATAL,_T("%%synch not supported by OCCS \n"));
  }


  void new_lev(_TINT how )
{
   /* Increment the current precedence level and modify  "Associativity"
    * to remember if we're going left, right or neither
    */

    if( Associativity = how )  /* '1', 'r', 'n', ( 0 if unspecified) */
        ++Prec_lev;
}

/*----------------------------------------------------------------------------*/

  void prec_list(_TCHAR *name )
{
    /* Add current name (in yytext) to the precision list. "Associativity" is
     * set to '1', 'r', or 'n', depending on whether we're doing a %left,
     * %right , or %nonassoc. Also make a nonterminal if it doesn't exist
     * already.
     */

     SYMBOL *sym;

     if( !(sym = (SYMBOL *)findsym((HASH_TAB *)Symtab,name)) )
         sym = make_term( name );

     if( !ISTERM(sym) )
         lerror(NONFATAL,_T("%%left or %%right, %s must be a token\n"), name );
     else
     {
        Precedence[ sym->val ].level = Prec_lev;
        Precedence[ sym->val ].assoc = Associativity ;
     }
}

/*-------------------------------------------------------------------------------*/

 void prec(_TCHAR *name )
{
/* Change the precedence level for the current right-hand side, using
 * (1) an explicit number if one is specified, or (2) an element from the
 * Precedence[] table otherwise.
 */

    SYMBOL   *sym;

    if( isdigit(*name) )                                          /* (1) */
        Sp->rhs->prec = _ttoi(name);
    else
    {
       if( !(sym = (SYMBOL *)findsym((HASH_TAB *)Symtab,name)) )
           lerror(NONFATAL,_T( "%s (used in %%prec) undefined\n") );

       else if( !ISTERM(sym) )
           lerror(NONFATAL, _T("%s ( used in %%prec) must be terminal symbol\n") );
           
       else
          Sp->rhs->prec = Precedence[ sym->val ].level;          /* (2) */
    }
}

/*-----------------------------------------------------------------------------*/

  void union_def(_TCHAR *action )
{
   /* create a YYSTYPE definition for the union, using the fields specified
    * in the %union directive, and also appending a default integer-sized
    * field for those situation where no field is attached to the current
    * symbol
    */
    
    while( *action && *action != '{' )    /* Get rid of everything up to the */
           ++action;                      /* open brace                      */
    if( *action )                         /* and the brace itself            */
        ++action;

    output(_T("typedef union\n") );
    output(_T("{\n") );
    output(_T( "    int  %s;  /* Default field, used when no %%type found */"),DEF_FIELD );
    output(_T("%s\n"),  action );
    output(_T("yystype;\n\n") );
    output(_T("#define YYSTYPE yystype\n") );
    Fields_active = 1;
 }

_TINT   fields_active()
 {
    return Fields_active;                 /* previous %union was specified */
 }

 /*------------------------------------------------------------------------------*/

  void new_field(_TCHAR *field_name )
 {
    /* Change the name of the current <field> */

    _TCHAR     *p;

    if( !*field_name )
        *Field_name = '\0' ;
    else
    {
       if( p = _tcschr(++field_name, _T('>') ) )
           *p = '\0' ;

       //strncpy( Field_name, field_name, sizeof(Field_name) );
	   _tcsncpy( Field_name, field_name, sizeof(Field_name) );
    }
 }
 #endif

