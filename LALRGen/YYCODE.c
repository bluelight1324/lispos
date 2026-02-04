/* yycode.c -- Controller Routine for Table Generation   Pg. 408            */
#include <stdio.h>
#include "debug.h"
/*---------------------------------------------------------------------*/

 void  make_yy_stok();
 void  make_token_file();
 void  make_parse_tables();


/*---------------------------------------------------------------------*/

 void   tables()
 {
     //#ifdef _DEBUG
      make_yy_stok();                                    /*  in stok.c    */
	 //#endif
      // token file is the header file like synout1.h which is not necessary
     //make_token_file();                                 /*  in stok.c    */
     make_parse_tables();                          /*  in yystate.c */
 }
