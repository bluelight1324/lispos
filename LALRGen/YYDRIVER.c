/* yydriver.c -- Routines to Create Occs output File 
 * page 409 of "Compiler Design in C" 
 */

#include <windows.h>
//#include <standard.h>
#include <stdio.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h"


/*---------------------------------------------------------------------*/
 extern    void output(_TCHAR *fmt, ...);
 extern   FILE *driver_1(FILE *output,_TINT lines,_TCHAR *file_name );
 extern    void error(_TINT fatal,_TCHAR *fmt, ... );
 extern    _TINT driver_2(FILE *output,_TINT lines );
 extern long NcNum;
/* ------------------------------------------------------------------------------ */

void file_header                  P(( void ));           /* public */
void code_header                  P(( void ));
void driver                       P(( void ));

/*--------------------------------------------------------------------------------*/

static FILE      *Driver_file; //= stderr ;

/*--------------------------------------------------------------------------------
 * Routines in this file are occs specific. There's a different version of all 
 * these routines in lldriver.c .They MUST be called in the following order:
 *  file_header()
 *  code_header()
 *  driver()
 *-------------------------------------------------------------------------------
 */

   void  file_header()
 {
    /*  This header is printed at the top of the output file, before the
	 *  definitions section is processed. Various #defines that you might want
	 * to modify are put here.
	 */

	#ifdef _DEBUG
	if( Public )
	    output( _T("#define  static\n") );

    if( Debug )
	    output( _T("#define  YYDEBUG\n") );
	#endif


    /* The below code has been added      */

    output(_T("/**************************************************************************\n"));
	output(_T(" * COPYRIGHT. %s	  						      *\n"),"AST");
	output(_T(" *																		  *\n"));
	output(_T(" *																		  *\n"));
	output(_T(" *																		  *\n"));
	output(_T(" *																		  *\n"));
	output(_T(" **************************************************************************/\n"));
    //output(_T(" #define NULL    ((void *)0)\n\n")                        );
	//output(_T(" #define NULL    0\n\n")                        );
	output(_T(" extern \"C\"\n{    \n    \n")                  );
//	output(_T(" __declspec(dllexport) char *__NSyn%d(int);\n"),NcNum             ); 		           
//  output(_T(" __declspec(dllexport) int  *__SynAction%d[];\n"),NcNum           ); 		 
//  output(_T(" __declspec(dllexport) int  *__SynGoto%d[];\n"),NcNum             ); 		 
//  output(_T(" __declspec(dllexport) int  __SynReduce%d[];\n"),NcNum            ); 		 
//	output(_T(" __declspec(dllexport) char *__SynYy_stok%d[];\n"),NcNum          ); 		 
//	output(_T("}    \n")                                        );
}

#ifdef NECESSARY

void output_driver(void)
{
  if( !( Driver_file = driver_1(Output, !No_lines, Template)  ))
	    error( NONFATAL, "%s not found--output file won't compile\n", Template);
}


/* ---------------------------------------------------------------------------*/


  void  code_header()
{
 /* This stuff is output after the definitions section is processed, but
  * before any  tables or the driver is processed.
  */

  driver_2( Output, !No_lines );
}

/* ----------------------------------------------------------------------------- */


  void   driver()
{

  /* Print out the actual parser by copying llama.par to the output file.
   */

   if( Make_parser)
       driver_2( Output, !No_lines );

   fclose( Driver_file );
}
#endif
