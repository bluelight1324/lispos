#include <windows.h>
//#include <standard.h>                                                                    
#include <tchar.h>
#include <stdio.h>
#include "debug.h"
#include "set.h"
#include "hash.h"               

#ifdef SPACE
#undef SPACE
#endif

#include "synyyout.h"


/* See comment on page 271  */

static _TINT Ignore = 0;
static _TINT Start_line;   /* starting line number */

/*----------------------------------------------------------------------
 * Prototypes for functions at the bottom of this file
 */

 void stripcr   P(( _TCHAR *src ));  /* Remove carriage returns (but not  */
								   /* linefeeds) from src.              */
 void nows      P(( void ));       /* Ignore white space, etc.          */
 void ws        P(( void ));       /* Don't ignore white space, etc.    */




#ifdef  __NEVER__
/*------------------------------------------------------
 * DFA (start state is 0) is:
 *
 * State 0  [nonaccepting]
  *    goto  8 on \x000\x001\x002\x003\x004\x005\x006\x007\b\t
  *    goto 26 on \n
  *    goto  8 on \x00b\f
  *    goto  9 on \r
  *    goto  8 on \x00e\x00f\x010\x011\x012\x013\x014\x015\x016\x017\x018\x019
 *                \x01a\x01b\x01c\x01d\x01e\x01f 
  *    goto  1 on !"#$
  *    goto 27 on %
  *    goto  1 on &\'+-.
  *    goto 24 on /
  *    goto  1 on 0123456789
  *    goto  7 on :
  *    goto  6 on ;
  *    goto 23 on <
  *    goto  1 on =?@ABCDEFGHIJKLMNOPQRSTUVWXYZ
  *    goto  5 on [
  *    goto  1 on \\
  *    goto  4 on ]
  *    goto  1 on ^_`abcdefghijklmnopqrstuvwxyz
  *    goto  3 on {
  *    goto  2 on |
  *    goto  1 on ~\x07f
 *State 1 [accepting, line 205 <return  NAME;>]
  *    goto  1 on !"#$&\'+-./0123456789=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\\^_`ab
 *                cdefghijklmnopqrstuvwxyz~\x07f
 *State 2 [accepting, line 198 <return OR;>]
 *State 3 [accepting, line 63 </* Suck up an entire>]
 *State 4 [accepting, line 202 <return END_OPT;\n\n>]
  *    goto 25 on *
 *State 5 [accepting, line 200 <return START_OPT;>]
 *State 6 [accepting, line 199 <return SEMI;>]
 *State 7 [accepting, line 197 <return COLON;>]
 *State 8 [accepting, line 207 <if( !Ignore) return >]
 *State 9 [accepting, line 206 <; /* discard carriag>]
 *State 10 [accepting, line 31 <{  /* Absorb a comme>]
 *State 11 [accepting, line 153 <{\n\t\t\t\t /* Copy >]
  *    goto 11 on \t 
 *State 12 [accepting, line 186 <return FIELD;       >]
 *State 13 [accepting, line 145 <{  /* The code below>]Anchor: start 
 *State 14 [accepting, line 190 <return TYPE;        >]
 *State 15 [accepting, line 189 <return TERM_SPEC;>]
 *State 16 [accepting, line 195 <return PREC;        >]
 *State 17 [accepting, line 192 <return LEFT;        >]
 *State 18 [accepting, line 187 <return PERCENT_UNION>]
 *State 19 [accepting, line 191 <return SYNCH;       >]
 *State 20 [accepting, line 196 <return START;       >]
 *State 21 [accepting, line 193 <return RIGHT;       >]
 *State 22 [accepting, line 194 <return NONASSOC;    >]
 * State 23  [nonaccepting]
  *    goto 28 on ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz
 *State 24 [accepting, line 205 <return  NAME;>]
  *    goto  1 on !"#$&\'
  *    goto 10 on *
  *    goto  1 on +-./0123456789=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\\^_`abcdefghi
 *                jklmnopqrstuvwxyz~\x07f
 *State 25 [accepting, line 202 <return END_OPT;\n\n>]
 *State 26 [accepting, line 207 <if( !Ignore) return >]
  *    goto 36 on %
 * State 27  [nonaccepting]
  *    goto 35 on l
  *    goto 34 on n
  *    goto 33 on p
  *    goto 32 on r
  *    goto 31 on s
  *    goto 30 on t
  *    goto 29 on u
  *    goto 11 on {
 * State 28  [nonaccepting]
  *    goto 28 on 0123456789
  *    goto 12 on >
  *    goto 28 on ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz
 * State 29  [nonaccepting]
  *    goto 61 on n
 * State 30  [nonaccepting]
  *    goto 62 on e
  *    goto 38 on o
  *    goto 37 on y
 * State 31  [nonaccepting]
  *    goto 40 on t
  *    goto 39 on y
 * State 32  [nonaccepting]
  *    goto 41 on i
 * State 33  [nonaccepting]
  *    goto 63 on r
 * State 34  [nonaccepting]
  *    goto 64 on o
 * State 35  [nonaccepting]
  *    goto 42 on e
 * State 36  [nonaccepting]
  *    goto 13 on %
 * State 37  [nonaccepting]
  *    goto 44 on p
 * State 38  [nonaccepting]
  *    goto 45 on k
 * State 39  [nonaccepting]
  *    goto 47 on n
 * State 40  [nonaccepting]
  *    goto 48 on a
 * State 41  [nonaccepting]
  *    goto 49 on g
 * State 42  [nonaccepting]
  *    goto 52 on f
 * State 43  [nonaccepting]
  *    goto 53 on o
 * State 44  [nonaccepting]
  *    goto 14 on e
 * State 45  [nonaccepting]
  *    goto 54 on e
 * State 46  [nonaccepting]
  *    goto 15 on m
 * State 47  [nonaccepting]
  *    goto 55 on c
 * State 48  [nonaccepting]
  *    goto 56 on r
 * State 49  [nonaccepting]
  *    goto 57 on h
 * State 50  [nonaccepting]
  *    goto 16 on c
 * State 51  [nonaccepting]
  *    goto 58 on a
 * State 52  [nonaccepting]
  *    goto 17 on t
 * State 53  [nonaccepting]
  *    goto 18 on n
 * State 54  [nonaccepting]
  *    goto 15 on n
 * State 55  [nonaccepting]
  *    goto 19 on h
 * State 56  [nonaccepting]
  *    goto 20 on t
 * State 57  [nonaccepting]
  *    goto 21 on t
 * State 58  [nonaccepting]
  *    goto 65 on s
 * State 59  [nonaccepting]
  *    goto 60 on o
 * State 60  [nonaccepting]
  *    goto 22 on c
 * State 61  [nonaccepting]
  *    goto 43 on i
 * State 62  [nonaccepting]
  *    goto 46 on r
 * State 63  [nonaccepting]
  *    goto 50 on e
 * State 64  [nonaccepting]
  *    goto 51 on n
 * State 65  [nonaccepting]
  *    goto 59 on s
 */

#endif


  /*-------------------------------------------------------------------- 
   * lex.par-- State-Machine Driver Copied to lexyy.c    page 75
   */


 
 /**********************************************************************        
 *          extern declarations 
 **********************************************************************
 */

 extern _TINT       ii_advance      ( void );
 extern _TINT       ii_pushback     ( _TINT  );
 extern void      ii_unterm       ( void ) ;
 extern _TCHAR      *ii_mark_start  ( void );
 extern _TINT       ii_look         ( _TINT  );
 extern _TINT       ii_flush        ( _TINT  );
 extern _TCHAR      *ii_mark_end    ( void );
 extern _TCHAR      *ii_to_mark     ( void );
 extern _TCHAR      *ii_move_start  ( void );
 extern void      ii_term         ( void );
 extern _TINT       ii_length       ( void );
 extern _TINT       ii_input        ( void );
 extern _TCHAR      *ii_text        ( void );
 extern _TINT       ii_lineno       ( void );
 extern void      lerror          (_TINT fatal,_TCHAR *fmt, ... );
 extern void      output          (_TCHAR *fmt, ...);
 extern _TINT       ii_lookahead    (_TINT  n );
 extern _TINT       ii_flushbuf     (void);
/**********************************************************************/

 _TINT yy_init_lex( void )
{
return 1;
}

_TINT yywrap()
{
 return(1 );
}

#ifndef  YYPRIVATE
#        define  YYPRIVATE  
#endif

#ifndef  NULL
#        include <stdio.h>
#endif

#ifdef  YYDEBUG
		_TINT    yydebug = 0;
#       define YY_D(x) if( yydebug ){ x; }else
#else
#       define YY_D(x)
#endif

#ifndef YY_TTYPE_DEFINED
typedef _TINT    YY_TTYPE;
#define YY_TTYPE_DEFINED
#endif
#define YYF   (( YY_TTYPE )(-1))


/*-----------------------------------------------------------------------      
 * Global variables used by the parser
 *-----------------------------------------------------------------------
 */


 _TCHAR   *yytext;             /* Pointer to lexeme    */
 _TINT    yyleng;              /* Length of lexeme     */
 _TINT    yylineno;            /* Input line number    */
 _TINT separator_count = 0;    /* Counts the number of "%%" found */
                             /*   in the input file             */


 //FILE   *yyout = stderr;

 /*----------------------------------------------------------------------
  * Macros that duplicate functions in UNIX lex:
  */
  #define  output(c)    _puttc(c,stdout)
  #define  ECHO         fprintf(yyout, "%s", yytext )

  #ifndef  YYERROR
  #    define  YYERROR(t)    fprintf(stderr,"%s", t )
  #endif

  #define  yymore()    yymoreflg = 1

  #define unput(c)  (ii_unput(c), --yyleng )
  #define yyless(n) (    ii_unterm(),(yyleng -= ii_pushback(n) ? n : yyleng ),ii_term() )
					
  _TINT   input()
  {
	  _TINT  c;
	  
	  if( (c = ii_input())  && (c != -1) )
	  {
		 yytext    = ii_text();
		 yylineno  = ii_lineno();
		 ++yyleng;
	  }
	  return c;
  }
  

/*---------------------------------------------------------
 *  The Yy_cmap[] and Yy_rmap arrays are used as follows:
 * 
 *  next_state = Yydtran[ Yy_rmap[current_state] ][ Yy_cmap[input_char] ];
 * 
 *  Character positions in the Yy_cmap array are:
 * 
 *    ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N  ^O
 *    ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\ ^]  ^^  ^ 
 *         !   "  #  $   %   &   '   (   )   *   +   ,   -   .  / 
 *    0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >  ? 
 *    @   A   B   C   D   E   F   G   H   I   J   K   L   M   N  O 
 *    P   Q   R   S   T   U   V   W   X   Y   Z   [   \  ]   ^  _ 
 *    `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
 *    p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  DEL
 */

YYPRIVATE YY_TTYPE  Yy_cmap[128] =
{
       0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  0,  0,  3,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   1,  4,  4,  4,  4,  5,  4,  4,  6,  6,  7,  4,  6,  4,  4,  8,
   9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10, 11, 12,  4, 13,  4,
   4, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15,  4, 16,  4, 14,
   4, 17, 14, 18, 14, 19, 20, 21, 22, 23, 14, 24, 25, 26, 27, 28,
  29, 14, 30, 31, 32, 33, 14, 14, 14, 34, 14, 35, 36,  6,  4,  4
};

YYPRIVATE YY_TTYPE Yy_rmap[66] =
{
     0,  1,  2,  2,  3,  2,  2,  2,  2,  2,  2,  4,  2,  2,  2,  2,
   2,  2,  2,  2,  2,  2,  2,  5,  6,  2,  7,  8,  9, 10, 11, 12,
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
  29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
  45, 46
};

YYPRIVATE YY_TTYPE Yy_nxt[ 47 ][ 37 ]=
{
/* 00 */ {   8,   8,  26,   9,   1,  27,  -1,  -1,  24,   1, 
     7,   6,  23,  -1,   1,   5,   4,   1,   1,   1, 
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1, 
     1,   1,   1,   1,   1,   3,   2
   },
/* 01 */ {  -1,  -1,  -1,  -1,   1,  -1,  -1,  -1,   1,   1, 
    -1,  -1,  -1,  -1,   1,  -1,  -1,   1,   1,   1, 
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1, 
     1,   1,   1,   1,   1,  -1,  -1
   },
/* 02 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 03 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  25,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 04 */ {  -1,  11,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 05 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  28,  -1,  -1,  28,  28,  28, 
    28,  28,  28,  28,  28,  28,  28,  28,  28,  28, 
    28,  28,  28,  28,  28,  -1,  -1
   },
/* 06 */ {  -1,  -1,  -1,  -1,   1,  -1,  -1,  10,   1,   1, 
    -1,  -1,  -1,  -1,   1,  -1,  -1,   1,   1,   1, 
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1, 
     1,   1,   1,   1,   1,  -1,  -1
   },
/* 07 */ {  -1,  -1,  -1,  -1,  -1,  36,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 08 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  35,  -1,  34,  -1,  33, 
    32,  31,  30,  29,  -1,  11,  -1
   },
/* 09 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  28, 
    -1,  -1,  -1,  12,  28,  -1,  -1,  28,  28,  28, 
    28,  28,  28,  28,  28,  28,  28,  28,  28,  28, 
    28,  28,  28,  28,  28,  -1,  -1
   },
/* 10 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  61,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 11 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  62, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  38,  -1, 
    -1,  -1,  -1,  -1,  37,  -1,  -1
   },
/* 12 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  40,  -1,  39,  -1,  -1
   },
/* 13 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  41,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 14 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    63,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 15 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  64,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 16 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  42, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 17 */ {  -1,  -1,  -1,  -1,  -1,  13,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 18 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  44, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 19 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  45,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 20 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  47,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 21 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  48,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 22 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  49,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 23 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    52,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 24 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  53,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 25 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  14, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 26 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  54, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 27 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  15,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 28 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  55,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 29 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    56,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 30 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  57,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 31 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  16,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 32 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  58,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 33 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  17,  -1,  -1,  -1,  -1
   },
/* 34 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  18,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 35 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  15,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 36 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  19,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 37 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  20,  -1,  -1,  -1,  -1
   },
/* 38 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  21,  -1,  -1,  -1,  -1
   },
/* 39 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  65,  -1,  -1,  -1,  -1,  -1
   },
/* 40 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  60,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 41 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  22,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 42 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  43,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 43 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    46,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 44 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  50, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 45 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  51,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1
   },
/* 46 */ {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 
    -1,  59,  -1,  -1,  -1,  -1,  -1
   } 
};

/*---------------------------------------------------------
 * yy_next(state,c) is given the current state number and input
 * character  and evaluates to the next state.
 */

#define yy_next(state,c) (Yy_nxt[Yy_rmap[state] ][ Yy_cmap[c] ] )

/*---------------------------------------------------------
 * The Yyaccept array has two purposes. If Yyaccept[i] is 0 the state
 * i is nonaccepting. If it's nonzero then the number determines whether
 * the string is anchored, 1=anchored at start of line, 2=at end of
 * line, 3=both, 4=line not anchored
 */

YYPRIVATE YY_TTYPE Yyaccept[] =
{
	0 ,   /* State 0   */
	4  ,   /* State 1   */
	4  ,   /* State 2   */
	4  ,   /* State 3   */
	4  ,   /* State 4   */
	4  ,   /* State 5   */
	4  ,   /* State 6   */
	4  ,   /* State 7   */
	4  ,   /* State 8   */
	4  ,   /* State 9   */
	4  ,   /* State 10  */
	4  ,   /* State 11  */
	4  ,   /* State 12  */
	1  ,   /* State 13  */
	4  ,   /* State 14  */
	4  ,   /* State 15  */
	4  ,   /* State 16  */
	4  ,   /* State 17  */
	4  ,   /* State 18  */
	4  ,   /* State 19  */
	4  ,   /* State 20  */
	4  ,   /* State 21  */
	4  ,   /* State 22  */
	0 ,   /* State 23  */
	4  ,   /* State 24  */
	4  ,   /* State 25  */
	4  ,   /* State 26  */
	0 ,   /* State 27  */
	0 ,   /* State 28  */
	0 ,   /* State 29  */
	0 ,   /* State 30  */
	0 ,   /* State 31  */
	0 ,   /* State 32  */
	0 ,   /* State 33  */
	0 ,   /* State 34  */
	0 ,   /* State 35  */
	0 ,   /* State 36  */
	0 ,   /* State 37  */
	0 ,   /* State 38  */
	0 ,   /* State 39  */
	0 ,   /* State 40  */
	0 ,   /* State 41  */
	0 ,   /* State 42  */
	0 ,   /* State 43  */
	0 ,   /* State 44  */
	0 ,   /* State 45  */
	0 ,   /* State 46  */
	0 ,   /* State 47  */
	0 ,   /* State 48  */
	0 ,   /* State 49  */
	0 ,   /* State 50  */
	0 ,   /* State 51  */
	0 ,   /* State 52  */
	0 ,   /* State 53  */
	0 ,   /* State 54  */
	0 ,   /* State 55  */
	0 ,   /* State 56  */
	0 ,   /* State 57  */
	0 ,   /* State 58  */
	0 ,   /* State 59  */
	0 ,   /* State 60  */
	0 ,   /* State 61  */
	0 ,   /* State 62  */
	0 ,   /* State 63  */
	0 ,   /* State 64  */
	0     /* State 65  */
};

/*---------------------------------------------------------------------*/

  void nows() { Ignore = 1; }   /* Ignore white space, etc.     */
  void ws()   { Ignore = 0; }   /* Don't ignore white space,etc.*/

  _TINT start_action()          /* Return starting line number of most */
{                                   /* recently read ACTION block      */
   return Start_line;
}

/*---------------------------------------------------------------------*/

static  void  stripcr( _TCHAR  *src)      /* Remove all \r's ( but not \n's from src */
{
   _TCHAR     *dest;
   for( dest = src ; *src ; src++ )
		if( *src != '\r' )
			*dest++ = *src;
   *dest = '\0';
}             
  /*---------------------------------------------------------------------*/
  
  yylex()
  {
	 _TINT          yymoreflg;     /* Set when yymore() is executed   */
	 static _TINT   yystate = -1;  /* Current state                   */
	 _TINT          yylastaccept;  /* Most recently seen accept state */
	 _TINT          yyprev;        /* State before yylastaccept       */
	 _TINT          yynstate;      /* Next state, given lookahead     */
	 _TINT          yylook;        /* Lookahead character             */
	 _TINT          yyanchor;      /* Anchor point for most recently seen*/
								 /* accepting state                 */
								 
	 if( yystate == -1 )
	 {
		yy_init_lex();        /* One-time initializations  */
		ii_advance();
		ii_pushback(1);
	 }
	 
	 
	 yystate      = 0;     /* Top-of-loop initializations   */
	 yylastaccept = 0;
	 yymoreflg    = 0;
	 ii_unterm();
	 ii_mark_start();
	 
	 while( 1)
	 {
	   /* Check end of file. If there's an unprocessed accepting state,
		* yylastaccept will be nonzero. In this case, ignore EOF for now
		* so that you can do the accepting action; otherwise, try to
		* open another file and return if you can't.
		*/
		
		while( 1 )
		{
		   if( (yylook = ii_look(1)) !=EOF )
		   {
			 yynstate = yy_next( yystate, yylook );
			 break;
		   }
		   else
		   {
			 if( yylastaccept )             /* still something to do */
			 {
				yynstate = YYF;
				break;
			 }
			 else if( yywrap() )           /* another file? */
			 {                             /*  no           */
				 yytext = _T("");                                                                                  
				 yyleng = 0;
				 return 0;
			 }
			 else
			 {
				ii_advance();             /* load a new buffer  */
				ii_pushback(1);
			 }
		  }
	  }


	  if( yynstate != YYF )
	  {
		 YY_D( printf("    Transition from state %d", yystate )      );
		 YY_D( printf("to state %d on <%c>\n",  yynstate, yylook)    );

		 if( ii_advance() < 0 )
		 {
			 YYERROR( "Lexeme too long, discarding characters\n" );
			 ii_flush(1);
		 }

		 if(yyanchor = Yyaccept[ yynstate ])     /* saw an accept state */
		 {
			yyprev       =  yystate ;
			yylastaccept = yynstate;
			ii_mark_end();              /* Mark input at current charcter */
										/* A subsequent  ii_to_mark()     */
										/* returns us to this position    */
		 }

		 yystate = yynstate;
	 }
	 else
	 {
		if( !yylastaccept )              /* illegal input     */
		{
#ifdef YYBADINP
			   YYERROR( "Ignoring bad input\n" );
#endif
			   ii_advance();       /* Skip _TCHAR that caused failure  */
		}
		else
		{
		  ii_to_mark();         /* Back up to previous accept state */
		  if( yyanchor & 2 )    /* If end anchor is active          */
			  ii_pushback(1);   /* push back the CR or LF          */


		  if( yyanchor & 1 )    /* If start anchor is active        */
			  ii_move_start();  /* skip the leading newline         */
			  
		  ii_term();            /* Null-terminate the string        */
		  
		  yyleng   = ii_length();
		  yytext   = ii_text  ();
		  yylineno = ii_lineno();
		  
		  YY_D( printf("Accepting state %d, ", yylastaccept )     );
		  YY_D( printf("line %d: <%s>\n",    yylineno, yytext )   );
		  
		  
		  switch( yylastaccept )
		  {
			   
			case 1:					/* State 1   */
		        return  NAME;
		        break;
			case 2:					/* State 2   */
		        return OR;
		        break;
			case 3:					/* State 3   */
		        /* Suck up an entire action.Handle nested braces here.
			 * This code won't work if the action is longer than the
			 * buffer length used by the input functions. If this is 
			 * a problem,you have to allocate your own buffer and copy
			 * the lexeme there as it's processed ( in a manner similar 
			 * to  the %{ processing below). If space is really a problem
			 * the code blocks can be copied to a temporary file and the
			 * offset to the start of the text( as returned by ftell() )
			 * can be stored rather than the string itself.
			 */
			 
			{
				_TINT i;
				_TINT nestlev;        /* brace - nesting level      */
				_TINT lb1;            /* previous character         */
				_TINT lb2;            /* charecter before that      */
				_TINT in_string;      /* processing string constant */                                                                                    
				_TINT in_char_const;  /* processing _TCHAR. constant  */
				_TINT in_comment;     /* processing a comment       */
 
				lb1 = lb2       = 0;
				in_string       = 0;
				in_char_const   = 0;
				in_comment      = 0;                    
				Start_line      = yylineno;

				for( nestlev = 1; i = input(); lb2 = lb1,lb1 = i )
				{
					if( lb2 == '\n' && lb1=='%' && i=='%' )
						lerror( FATAL,
							_T("%%%% in code block starting on  line %d\n"),
															Start_line );

					if( i < 0 )  /* input-buffer overflow  */
					{
						ii_unterm();
						ii_flush(1);
						ii_term();
						lerror(FATAL,
									_T("Code block starting on line %d too long.\n"),
																 Start_line);
				}
					
		 /* Take care of \{, "{",'{', \},"}", '}',  */
					
					if( i == '\\' )
					{
						if( !(i = input()) )   /* discard backslash */
							break;
						else
							continue; /* and following _TCHAR */
					}
					
					if( i == '"' && !(in_char_const || in_comment) )
						in_string = !in_string;
						
					else if( i == '\'' && !(in_string  || in_comment) )
						in_char_const = !in_char_const;
						 
					else if( lb1 == '/'  && i == '*' &&  !in_string )
						in_comment = 1;
						 
					else if( lb1 == '*'  && i == '/' && in_comment )
						in_comment = 0;
						 
					if( !(in_string || in_char_const || in_comment) )
					{
						if( i == '{' )
						++nestlev;
						   
					if( i == '}'  && --nestlev <= 0)
					{
						   stripcr( yytext );
						   return ACTION;
                    
					}
				}
			} 
				
			lerror(FATAL, _T("EOF in code block starting on line %d\n"),
														  Start_line );
		}

		        break;
			case 4:					/* State 4   */
		        return END_OPT;


		        break;
			case 5:					/* State 5   */
		        return START_OPT;
		        break;
			case 6:					/* State 6   */
		        return SEMI;
		        break;
			case 7:					/* State 7   */
		        return COLON;
		        break;
			case 8:					/* State 8   */
		        if( !Ignore) return WHITESPACE;  

		        break;
			case 9:					/* State 9   */
		        ; /* discard carriage return (\r) */
		        break;
			case 10:					/* State 10  */
		        {  /* Absorb a comment (treat it as WHITESPACE) */

			_TINT i;
			_TINT start = yylineno;
   
			while( i = input() ) 
			{
				if( i < 0 )
				{
					ii_unterm();
					ii_flush(1);

					 ii_term();
					lerror(NONFATAL,_T("Comment starting on line %d too long,\
					truncating\n"), start );
				}
				else if( i == '*'  && ii_lookahead(1) == '/' )
				{
					input();
					stripcr( yytext );
						  
					if( Ignore ) goto end;
					else         return WHITESPACE;
				}
		  }
				  
		  lerror(FATAL, _T("End of file encountered in comment\n"));
			  
	 end:;
	}
			
		        break;
			case 11:					/* State 11  */
		        {
				 /* Copy a code block to the output file */
				 
				_TINT c, looking_for_brace = 0;
				 
				#undef  output      /* replace macro with function   */
									 /* in main.c                     */
									 
				if( !No_lines )
					output(_T( "\n#line %d \"%s\"\n"), yylineno, Input_file_name );

				while( c= input() )   /* while not at end of file */
				{
					if( c== -1)       /* buffer is full, flush it */
					ii_flushbuf();

					else if( c != '\r' )
					{
						if( looking_for_brace ) /* last _TCHAR was a % */
						{
							if( c== '}' ) break;
							else          output(_T("%%%c"), c );
						}
						else    
						{
							if( c == '%' ) looking_for_brace = 1;
							else           output(_T("%c"),c );
						}
					}
				}
				return CODE_BLOCK;
			}

		        break;
			case 12:					/* State 12  */
		        return FIELD;              /* for occs. only   */
		        break;
			case 13:					/* State 13  */
		        {  /* The code below has been modified                    */  
				  
					  separator_count++;    /* Keep count of no. of %% */
					  return SEPARATOR;     /* Must be anchored because  it can */
											/* appear in a printf statement   */
				  
		}       
								   
		        break;
			case 14:					/* State 14  */
		        return TYPE;               /* for occs. only   */
		        break;
			case 15:					/* State 15  */
		        return TERM_SPEC;
		        break;
			case 16:					/* State 16  */
		        return PREC;               /* for occs. only   */
		        break;
			case 17:					/* State 17  */
		        return LEFT;               /* for occs. only   */
		        break;
			case 18:					/* State 18  */
		        return PERCENT_UNION;      /* for occs. only   */
		        break;
			case 19:					/* State 19  */
		        return SYNCH;              /* for llama only   */
		        break;
			case 20:					/* State 20  */
		        return START;              /* for error messages only */
		        break;
			case 21:					/* State 21  */
		        return RIGHT;              /* for occs. only   */
		        break;
			case 22:					/* State 22  */
		        return NONASSOC;           /* for occs. only   */ 
		        break;
			case 24:					/* State 24  */
		        return  NAME;
		        break;
			case 25:					/* State 25  */
		        return END_OPT;


		        break;
			case 26:					/* State 26  */
		        if( !Ignore) return WHITESPACE;  

		        break;
          default:
                  YYERROR( "INTERNAL ERROR, yylex\n" );
                  break;
          }
       }
       
       ii_unterm();
       yylastaccept = 0;
       
       if( !yymoreflg )
       {
           yystate = 0;
           ii_mark_start();
       }
       else
       {
           yystate   = yyprev;     /* Back up  */
           yymoreflg = 0;
       }
    }
  }
}                                               
