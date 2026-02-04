/* yydollar.c  -- Process $ Attributes                                  */

#include <windows.h>
//#include <standard.h>
#include <stdio.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include "hash.h" 



/*--------------------------------------------------------------------*/
 extern    fields_active();
 extern    void error(_TINT fatal,_TCHAR *fmt, ... );

/*---------------------------------------------------------------------*/

#define   DEF_FIELD  "yy_def"      /* Field name for default field in a %union */

  _TCHAR *do_dollar(_TINT num,_TINT rhs_size,_TINT lineno,PRODUCTION *prod,_TCHAR *fname )
/* int          num      The N in $N, DOLLAR_DOLLAR for $$ (DOLLAR_DOLLAR)    */
/*                       is defined in parser.h,                              */
/* int          rhs_size;  Number of symbols on right-hand side, 0 for tail     */
/* int          lineno;    Input line number for error messages                 */
/* PRODUCTION   *prod;     Only used if rhs_size is >= 0                        */
/* char         *fname;    name in $<name>N                                     */
{
    static   _TCHAR  buf[ 128 ]; /* Original : static   char  buf[ 128 ];*/
    _TINT i, len;

	if( num == DOLLAR_DOLLAR )
    {                                                 /* Do $$  */
        _tcscpy( buf, _T("Yy_val") );

        if( *fname )                                 /* $<name>N */
            _stprintf( buf+6, _T(".%s"), fname );
        
        else if( fields_active() )
        {
             if( *prod->lhs->field )
                 _stprintf( buf+6, _T(".%s"), prod->lhs->field );
             else
             {
                 error( WARNING, _T("Line %d: No <field> assigned to $$, ")
                                 _T("using default int field\n"),      lineno);

                 _stprintf( buf+6, _T(".%s"), _T(DEF_FIELD) );
             }
         }
    }
    else
    {
       if( num < 0 )
           ++num;

        if( rhs_size < 0 )                                   /* $N is in tail  */
            _stprintf( buf, _T("Yy_vsp[ Yy_rhslen-%d ]"), num );
        else
        {
            if( (i = rhs_size - num) < 0 )
               error( WARNING, _T("Line %d: Illegal $%d in production\n"),lineno, num );
            else
            {
                len = _stprintf( buf, _T("yyvsp[%d]"), i );

                if( *fname )                                         /* $<name>N */
                    _stprintf( buf + len, _T(".%s"), fname );

                else if( fields_active() )
                {
                if( num <= 0 )
                {
                   error(NONFATAL, _T("Can't use %%union field with negative")
                                   _T(" attributes. Use $<field>-N\n") );
                 }
                 else if( * (prod->rhs)[num-1]->field )
                 {
                     _stprintf( buf + len, _T(".%s"), (prod->rhs)[num-1]->field );
                 }
                 else
                 {
                   error( WARNING, _T("Line %d: No <field> assigned to $%d, ")
                                   _T("using default int field\n"),lineno, num );
                   _stprintf( buf + len, _T(".%s"), _T(DEF_FIELD) );
                 }
              }
           }
        }
    }
    
    return buf;
 }                                                       
