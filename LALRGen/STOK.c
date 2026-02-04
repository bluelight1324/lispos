/* stok.c -- Routines to Make yyout.h and yy_stok[] Pg. 321                */

#include <windows.h>
//#include "standard.h"
#include <STDPAR.h>
#include <stdio.h>
#include <signal.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h"


/*-------------------------------------------------------------------------------*/
extern   void error(_TINT fatal,_TCHAR *fmt, ... );
extern   void output(_TCHAR *fmt, ...);
extern void    comment(FILE *fp,_TCHAR **argv );
extern void make_yy_stok          P(( void ));               /* public    */
extern void make_token_file       P(( void ));
extern movefile(_TCHAR *,_TCHAR *,_TCHAR *);
extern long NcNum;
/*-------------------------------------------------------------------------------*/

//#ifdef _DEBUG
  void   make_yy_stok()
{
    /* This subroutine generates the Yy_stok[] arrray  that's indexed by token
     * value and evaluates to a string representing the token name. Token values
     * are adjusted so that the smallest token value is 1(0 is reserved for end
     * of input)
     */

    register  _TINT  i;

 /*   static char *the_comment[] = 
    {
        "Yy_stok[] is used for debugging and error messages. It is indexed",
        "by the internal value used for a token (as used for a column index in",
        "the transition matrix) and evaluates to a string naming that token",
        NULL
    };

    MESG(comment( Output, the_comment );)*/
	
    output(  _T("const __declspec(dllexport) char *__SynYy_stok%d[] =\n{\n"),NcNum );
    output(  _T("\t/*    0 */  \"__SOT__ \",\n") );
    for( i = MINTERM; i <= Cur_term; i++ )
    {

     output( _T("\t/* %3d */   \"%s\""),   (i-MINTERM)+1, Terms[i]->name );
	 //output( _T("  TernaryInsert((unsigned char *)\"%s\",(long *)%d,(struct tst *)TersPtr%d,0,0);"),Terms[i]->name,(i-MINTERM)+1,NcNum);
//       if( i != Cur_term )
//           outc( ',' );
       if( i <= Cur_term )
           outc( ',' );

       /* if( (i & 0x1) == 0 || i == Cur_term )  */   
	   /* Newline for every other  element            */
        outc( '\n' );                             
    }

	//outc( ',' );
	output(_T("\t\t   "));
	output(_T("     \"%s\" "),ENDOF_TABLE);
	outc(_T( '\n') );                             

    output( _T( "};\n\n"));
	//output( _T( "}\n\n"));


}

//#endif

/*------------------------------------------------------------------------------*/
#ifdef NECESSARY
 void   make_token_file()
{
  /* This subroutine generates the yytokens.h file. Tokens have the same value
   * as in make_yy_stok(). A special token named _EOI_ (with a value of 0 ) is
   * also generated
   */

   FILE   *tokfile;//,*casefile;
   _TCHAR   token_fname[MAX_PATH];
   _TINT    i;

   _stprintf(token_fname,_T("%s%d.%s"),_T(TOKEN_FILE),NcNum,_T(TOKEN_FILE_EXT)); 
   if( !(tokfile = _tfopen( token_fname , _T("w")) ))
     error( FATAL,  _T("can't open %s\n"), _T(TOKEN_FILE) );

   /*if( !(casefile = fopen( CASEOUT_FILE , "w+") ))
     error( FATAL,  "can't open %s\n", CASEOUT_FILE );*/

   D( else if ( Verbose )                         )
   D(    _tprintf(_T("Generating %s\n"),  TOKEN_FILE ); )

   _ftprintf( tokfile, _T("#define _EOI_       0\n"));
   

   for( i= MINTERM; i <= Cur_term; i++ )
		/* Dont print the rdam numbers */
		if ( (i-MINTERM)+1 <= NUM_OF_DEFINED_TERM_SYMBOLS )  
		{							/* as #define in synout.h       */
        fprintf( tokfile, "#define %-10s %d\n", Terms[i]->name, (i-MINTERM)+1 );
		}
		else    /* print them as case statements in the case file */
		{
		  
		  //fprintf(casefile,"\n\t\tstrcpy(RdamTokenNumberTable[%d],%c%s%c);",
		  //        						  (i-MINTERM)+1,'"',Terms[i]->name,'"');
	    }

        
        /* put the ending brackets for the case statements*/
        //fprintf(casefile,"\n\t\t\n}");  
                    		               
	                    		            

   //fclose( casefile);
 
   fclose( tokfile );

}

#endif
                                                     
