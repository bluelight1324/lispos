/* yypatch.c -- Modify Grammar for use in Occs Page 402                     */

#include <windows.h>
//#include <standard.h>
#include <stdio.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h"
#include <virtualmem.h>    /* memory-management functions */

#pragma warning(disable:4018)

/*---------------------------------------------------------------------*/
extern void    printv(FILE *fp,_TCHAR **argv );
//extern   _TINT ptab(HASH_TAB *tabp,void (*),void *param,_TINT sort);
extern _TINT   ptab( HASH_TAB *, void(*)(), void *, _TINT );
extern   void error(_TINT fatal,_TCHAR *fmt, ... );
extern   void output(_TCHAR *fmt, ...);
extern _TCHAR *remove_junk(_TCHAR *);
extern void pact(SYMBOL *sym,FILE *stream );
extern long NcNum;
/*------------------------------------------------------------------------------*/

void patch               P(( void                 ));   /* public   */

_TINT dopatch             P(( SYMBOL  *sym         ));   /* local    */
void print_one_case      P(( _TINT case_val,_TCHAR *action, \
                           _TINT rhs_size, _TINT lineno, struct _prod_ *prod ));
                             

/* -----------------------------------------------------------------------------*/

static  _TINT         Last_real_nonterm ;   
 /* This is the number of the last nonterminal to appear in the input grammar
  * ( as compared to the ones that patch() creates)
  */

/*-----------------------------------------------------------------------------*/

  void patch()
{
    /* This subroutine does several things:
     *
     *     * It modifies the symbol table as described in the text
     *     * It prints the action subroutine and deletes the memory associated
     *       with the actions
     *
     * This is not a particularly good subroutine from a structured prog. 
     * perspective because it does two very different things at the same time.
     * You save a lot of code by combining operations however.
     */


     //void dopatch();

    //output("_TCHAR *__NSyn%d(_TINT _SynNum)",NcNum);

     static char *top[] =
     {
	   _T(""),
       _T("{"),
       _T(""),
       _T("    switch( _SynNum )"),
       _T("    {"),
         NULL
      };

      static char  *bot[] =
      {
           //_T("\n"),
           _T(" "),
           _T("     default:  \n\n") 
           _T("     return  0;\n")
		   //_T("	               \n")
           _T(" "),
           _T("    }"),
           _T(""),
           _T("}\n\n\n"),
           NULL
       };

       Last_real_nonterm = Cur_nonterm;

       if( Make_actions )
       {
		output(_T("const __declspec(dllexport) char *__NSyn%d(const int _SynNum)"),NcNum);
		printv( Output, top );
	   }
         ptab( Symtab,dopatch, NULL, 0 );

       if( Make_actions )
           printv( Output, bot );
    }
      
  /*----------------------------------------------------------------------------*/

  //static  void  dopatch(SYMBOL *sym)
  static  _TINT  dopatch(SYMBOL *sym)
  {
      PRODUCTION    *prod ;       /* Current right-hand side of sym               */
      SYMBOL        **pp  ;       /* Pointer to one symbol on rhs                 */
      SYMBOL        *cur  ;       /* Current element of right-hand side           */
	  static unsigned debugcounter = 0;


      if( !ISNONTERM(sym)  || sym->val > Last_real_nonterm )
      {
         /* If the current symbol isn't a nonterminal, or if it is a nonterminal
          * that used to be an action (one that we just transformed), ignore it.
          */
          return 1;
      }

      
      for( prod = sym->productions; prod; prod = prod->next )
      {
         if( prod->rhs_len == 0 )
             continue;

         pp  = prod->rhs + (prod->rhs_len -1 );
         cur = *pp;

         if( ISACT(cur) )                        /* Check rightmost symbol  */
         {	 //strcpy(string,cur->string);
             cur->string = remove_junk(cur->string);
             print_one_case( prod->num, cur->string, --(prod->rhs_len),
                                        cur->lineno, prod );
			 ++debugcounter;
			            
             delsym   ( (HASH_TAB*) Symtab,  (BUCKET*) cur );
             cur->string = NULL;
             *pp-- = NULL;
          }

          /* cur is no longer valid because of the --pp shown above
           * Count the number of nonactions in the right-hand
           * side
           */

           for(; pp >= prod->rhs; --pp )
           {
               cur = *pp;

               if( !ISACT(cur) )
                   continue;

               if( Cur_nonterm >= MAXNONTERM )
                   error(1,_T("Too many nonterminals & actions (%d max)\n"), MAXTERM);
               else
               {
                  /* Transform the action into a nonterminal */

                  Terms[ cur->val = ++Cur_nonterm ] = cur;

                  cur->productions = 
				  (PRODUCTION*)AllocMem( sizeof(PRODUCTION)
				                              );
                  if( !cur->productions )
                      error(1, _T("INTERNAL [dopatch]: Out of memory\n"));

                  cur->string = remove_junk(cur->string);
                  print_one_case( Num_productions,   /* Case value to use         */
                                  cur->string,       /* Source code               */
                                  (_TINT)(pp - prod->rhs),    /* # symbols to left of act  */
                                  cur->lineno,       /* Input line # of code      */
                                  prod
                                );

                  /* Once the case is printed, the string argument can be freed. */

                  /* free( cur->string );*/
                  /* cur->string is not allocated  memory by malloc()
                   * so there is no need to free it                */
                  cur->string               = NULL;
                  cur->productions->num     = Num_productions++ ;
                  cur->productions->lhs     = cur ;
                  cur->productions->rhs_len = 0 ;
                  cur->productions->rhs[0]  = NULL;
                  cur->productions->next    = NULL;
                  cur->productions->prec    =  0;

                  /* Since the new production goes to epsilon and nothing else,
                   * FIRST(new) == { epsilon }. Don't bother to refigure the
                   * follow sets because they won't be used in the LALR(1) state
                   * machine routines [ If you really want them, call follow()
                   * again. ]
                   */

                   cur->first = newset();
                   ADD( cur->first, EPSILON );
               }
            }
        }
	  return 1;
  }


 
 static void print_one_case(_TINT case_val,_TCHAR *action,_TINT rhs_size,_TINT lineno,PRODUCTION *prod )
 /* _TINT             case_val;      Numeric value attached to case itself     */
 /* unsigned _TCHAR   *action;       Source code to execute in case            */
 /* _TINT             rhs_size;      Number of symbols on right-hand side      */
 /* _TINT             lineno;        input line number( for #lines).           */
 /* PRODUCTION      *prod;         Pointer to right-hand side.               */
 {

    /* Print out one action as a case statement. All $-specifiers are mapped
     * to refrences to the value stack: $$ becomes Yy_vsp[0], $1 becomes
     * Yy_vsp[-1], etc. The rhs_size argument is used for this purpose.
     *[see do_dollar() in yydollar.c for details]
     */

     _TINT         num, i;
     //_TCHAR        *do_dollar();            /* source found in yydollar.c          */
	 _TCHAR *do_dollar ( _TINT,_TINT ,_TINT,PRODUCTION *,_TCHAR *);
     extern _TCHAR *production_str(PRODUCTION *);       /* source found in acts.c              */
     _TCHAR        fname[40], *fp;          /* place to assemble $<fname>1         */
     #ifdef _DEBUG
      _TCHAR *tbuf;
	 #endif

	 if( !Make_actions )
         return;

     #ifdef _DEBUG
      output(_T("\n    case %d: /* %s  */\n\n\t"), case_val,tbuf = production_str(prod) );
	 #else
	 /* Dont print out the comments in a  release version */
	  output(_T("\n    case %d: \n\n\t"), case_val);
	 #endif

     if( !No_lines )
          output(_T("#line %d \"%s\"\n\t"), lineno, Input_file_name );

     while( *action )
     {
         if( *action != '$' )
          {  output( _T("%c"), *action++ );
          }
         else
         {
            /* Skip the attribute reference. The if statement handles $$ the
             * else clause handles the two forms: $N and $-N, where N is a
             * decimal number. When we hit the do_dollar call ( in the output()
             * call), "num" holds the number associated with N, or DOLLAR_DOLLAR
             * in case of $$.
             */

             if( *++action != '<' )
                 *fname  ='\0';
             else
             {
                 ++action;           /* skip the < */
                 fp = fname;

                 for(i=sizeof(fname); --i>0 && *action && *action != '>'; )
                     *fp++ = *action++;

                 *fp = '\0';
                 if( *action == '>' )
                     ++action;
              }

              if( *action == '$' )
              {
                  num = DOLLAR_DOLLAR;
                  ++action;
              }
              else
              {
                 num = _ttoi (( _TCHAR *) action );
                 if( *action == '-' )
                         ++action;
                 while( isdigit(*action) )
                        ++action;
               }

               output( _T("%s"), do_dollar( num, rhs_size, lineno, prod, fname ));
            }
        }
        //output(_T("\n              break;\n")                     );
		output(_T("\n")                     );
   }



_TCHAR *remove_junk(_TCHAR *action_statement)
{
       /* Remove junk at end of line of action statement    */
			 
				_TCHAR *i,*s;
				_TINT nestlev,j;          /* brace - nesting level      */
				_TCHAR *lb1;     /* previous character         */
				_TCHAR *lb2;     /* character before that      */
 
				lb1 = lb2       = 0;

                s = (_TCHAR *) AllocMem( (DWORD)(_tcslen((_TCHAR *)action_statement) *5) );
                     if(!s)
                      { 
                        printf("\n Internal Error[No Mem][yypatch.c]\n");
                        exit(1);
                      }
                lb1 = (_TCHAR *) malloc( 5 );
                lb2 = (_TCHAR *) malloc( 5 );
                i = action_statement;
				for( nestlev = 0,j = 0 ;; lb2 = lb1,lb1 = i,i++,j++ )
				{
                        s[j] = *i;
						if( *i == '{' && lb1 != (_TCHAR *)"'" )
						++nestlev;
						   

					if( *i == '}'  && lb1 != (_TCHAR *)"'" && --nestlev <= 0 )
					   break;

                   if( j > _tcslen((_TCHAR *)action_statement) ) /* if "}" not found */
                    { 
                      printf("\n Syntax error: ""'}'"" not found at %s\n",action_statement);
                      exit(1);
                    }
				}
               
             s[++j] = '\0';/* Null- terminate string */
             return s;

}



