
/* main.c -- Command  Line Parsing and main() Pg. 322                         */

#include <windows.h>
//#include <standard.h>
#include <stdpar.h>
#include <stdio.h>
#include <signal.h>
#include <sys/timeb.h>
#include <errno.h>
#include "debug.h"
#include <tchar.h>
#include "set.h"
#include <virtualmem.h>    /* memory-management functions */

/* Removed COMPANY_NAME redefinition and invalid exestr pragma (LNK4229) */




#ifdef  LLAMA
#       define  ALLOCATE
#       include "haspars.h"
#       undef   ALLOCATE
#else
#       define  ALLOCATE
#       include "hash.h"
#       undef   ALLOCATE
#endif  


//TCHAR  *GlobalCompanyName = _T(COMPANY_NAME),
//		*GlobalCopyRight   = _T(COPYRIGHT),
//		*GlobalRights      = _T(RIGHTS),
//		*GlobalProgramName = _T(PARSER_NAME),
//		*GlobalProgramVer  = _T(PARSER_VERSION) ;


long    NcNum;					            /* Neuronal Cluster number */
static   _TINT     Warn_exit     = 0;         /* Set to 1 if -W on command line    */
static   _TINT     Num_warnings  = 0;         /* Total warnings printed            */
static   _TCHAR    Output_fname[MAX_PATH];   /* Name of the output file           */
static   FILE    *Doc_file     = NULL;      /* Error log & machine desc.         */



/**************************** Extern Declarations **********************/
extern void signon(void);             
extern _TINT  FError(_TCHAR *fmt, ... );
extern _TINT  ii_newfile(_TCHAR *name ) ;
extern   void  init_acts(void);
extern   void  file_header(void);
extern   void  nows(void);
extern _TINT    yyparse(void);
extern   _TINT problems(void);
extern   void first(void);
extern   void  code_header(void) ;
extern   void patch(void);
extern void   tables(void);
extern  void  driver(void);
extern  void print_symbols(FILE *stream );
extern  void lr_stats(FILE *fp );
extern _TINT lr_conflicts(FILE *fp );
extern void   ii_unterm(void);
extern _TINT    ii_advance(void);
extern _TINT    ii_flush(_TINT force );
extern _TCHAR *ii_mark_start(void);
extern _TINT   ii_pushback(_TINT n );
extern  _TINT ii_plineno(void);
extern  _TINT ii_plineno (void);
extern  _TINT ii_lineno (void);
extern void    printv(FILE *,_TCHAR **);
extern _TINT     yylex();
extern void    pact(SYMBOL *sym,FILE *stream );
extern void    output_driver(void);
/**********************************************************************/



#define   VERBOSE(str)     if(Verbose){ printf( str ":\n" );   }else
#define E(x)   fprintf(stdout,"%s\n", x )

/*------------------------------------------------------------------------------*/

void onintr       P(( _TINT                              ));        /* local    */
void parse_args   P(( int argc,    _TCHAR  **argv        ));
_TINT  do_file      P(( void                             ));
void symbols      P(( void                             ));
void statistics   P(( FILE  *fp                        ));
void tail         P(( void                             ));

void  main        P(( int argc,  _TCHAR  **argv          ));       /* public    */
void output       P(( _TCHAR *fmt, ...                   ));
void lerror       P(( _TINT fatal,  _TCHAR  *fmt, ...      ));
void error        P(( _TINT fatal,  _TCHAR  *fmt, ...      ));
_TCHAR *open_errmsg P(( void                             ));
_TCHAR *do_dollar   P(( _TINT num,_TINT rhs_size,_TINT lineno,PRODUCTION *prod,_TCHAR *fname));

/*---------------------------------------------------------------------------------
 * There are two versions of the following subroutines-- used  in do_file(),
 * depending on whether this is llama or occs.
 * 
 * subroutine:           llama version in:                 occs version in:
 *
 * file_header()         lldriver.c                        yydriver.c
 * code_header()         lldriver.c                        yydriver.c
 * driver()              lldriver.c                        yydriver.c
 * tables()              llcode.c                          yycode.c
 * patch()               --------                          yypatch.c
 * select()              llselect.c                        ---------
 * do_dollar()           lldollar.c                        yydollar.c
 *
 * Also, several part of this file are compiled only for llama, others only for
 * occs. The llama-specific parts are arguments to LL() macros, the occs-
 * specific parts are in OX() macros. We'll look at what the occs-specific
 * parts actually do in the next chapter.
 *--------------------------------------------------------------------------------
 */



   extern  _TINT yynerrs;             /* Total error count          */
  
 /* The above originally defined in llpar.c page 286            */
//#include <sys/timeb.h>
  void main(int argc,_TCHAR **argv )
 {
  DWORD   Time;
  TCHAR ErrorMsg[64];
  struct _timeb StartTime,EndTime;

/*__try
   {*/
	signon();                        /* Print sign on message   */
	parse_args( argc, argv );

    if( TimeIt )
	{
     _ftime( &StartTime ); 
     EndTime = StartTime ;  
	}


    if( Debug  && !Symbols )
        Symbols = 1;

    OX( if( Make_parser )                                                     )
    OX( {                                                                     )
    OX(      if( Verbose == 1 )                                               )
    OX(      {                                                                )
    OX(          if( !(Doc_file = fopen( DOC_FILE, "w") ) )                   )
    OX(              FError( _T("Can't open log file %s\n"), DOC_FILE );            )
    OX(      }                                                                )
    OX(      else if( Verbose > 1 )                                           )
    OX(           Doc_file = stderr;                                          )
    OX( }                                                                     )


    if( Use_stdout )
    {	
      #ifdef USE_STDOUT
         Output_fname = "/dev/tty" ;
         Output       = stdout;
      #endif
    }
    else
    {
        //OX( Output_fname = !Make_parser ? ACT_FILE : PARSE_FILE ; )
        LL( Output_fname = PARSE_FILE;                            )
		OX( _stprintf(Output_fname,_T("%s%d.%s"),_T(PARSE_FILE),NcNum,_T(OUT_FILE_EXT)); )

        if( (Output = _tfopen( Output_fname, _T("w"))) == NULL )
                error( FATAL, _T("Can't open output file %s: %s\n"),
                                                      Output_fname, open_errmsg() );
    }

		do_file(); /* Do all the work  and dont bother with errors    */
        
		#ifdef _DEBUG
         if( Symbols )                            /* Print the symbol table  */
          symbols();
		       
         statistics( stdout );
		

         if( Verbose && Doc_file )
         {
           OX( statistics( Doc_file );     )
         }
     
		#endif
      /* Exit with the number of hard errors (or, if -W was specified, the sum
       * of the hard errors and warnings) as an exit status. Doc_file and Output
       * are closed implicitly by exit().
       */

       /*exit( yynerrs + (Warn_exit ? Num_warnings : 0) );*/
	   _fcloseall();
       if( TimeIt )
	   {
        _ftime ( &EndTime        );
        Time   = (DWORD)((  EndTime.time * 1000) + EndTime.millitm) ;
        Time  -= (DWORD)((StartTime.time * 1000) + StartTime.millitm) ;
        printf("\nTime required: %1d.%-03d seconds\n",\
              (Time/1000), (Time%1000)
	          );
	   }

	   exit(SYN_SUCCESS);
   //} // end of try
/*   __except(EXCEPTION_EXECUTE_HANDLER) 
   {
	_tcscpy(ErrorMsg,"An internal error occured");
	printf("\n%s", ErrorMsg);
	exit(FAILURE);
   }*/
 }

 /*-----------------------------------------------------------------------------*/
#ifdef NECESSARY
static void  onintr(_TINT i)                /* SIGABRT (Ctrl-Break, ^C) Handler */
 {
    if( Output != stdout )                /* Destroy parse file so that a     */
    {                                     /* subsequent compile will fail     */
        fclose( Output );
        _unlink( Output_fname );
     }
     exit( EXIT_USR_ABRT );
     return;
 }
#endif

 /*-----------------------------------------------------------------------------*/

static void parse_args(int argc,_TCHAR **argv )
 {
    /* Parse the command line, setting global variables as appropriate         */

    _TCHAR         *p;
    static _TCHAR  name_buf[80]; /* Original : static _TCHAR  name_buf[80]
                                  Use to assemble default file name */
	#ifdef USAGE_MSG
	static  _TCHAR *usage_msg[] =
    {
  #ifdef LLAMA
          "Usage is: llama [-switch] file",
          "Create an LL(1) parser from the specifications in the",
          "input file. Legal command-line switches are:",
          "",
          "-cN      Use N as the pairs threshold when (C)ompressing",
          "-D       enable (D)ebug mode in yyparse.c (implies -s)",
          "-f       (F)ast, uncompressed tables",
  #else
          "Usage is: syn [-switch] file ",
          "",
          "\tCreate an LALR(1) parser from the specifications in the",
          "\tinput file. Legal command-line switches are:",
          "",
          "-a       Output actions only ( see -p)",
          "-D       enable (Debug) mode in yyparse.c (implies -s)",
  #endif
          "-g       make static symbols (G)lobal in yyparse.c",
          "-l       suppress #(L)ine directives",
          "-p       output parser only (can be used with -T also)",
          "-s       make (s)ymbol table",
          "-S       make more-complete (S)ymbol table",
          "-t       print all (T)ables (and the parser) to standard output",
          "-T       move large tables from yyout.c to yyoutab.c",
          "-v       print (V)erbose diagnostics (including symbol table)",
          "-V       more verbose than -v. Implies -t & yyout.doc goes to stderr",
          "-w       suppress all warning messages",
          "-W       warnings (as well as errors) generate nonzero exit status",
          NULL

	};	
	 #endif

	  if (argc == 1) 
      {
	      #ifdef USAGE_MSG
	       printv(stdout,usage_msg);			 
		  #endif
          
          exit(1);
      }
     /* Note that all global variables set by command-line swicthes are declared
      * in parser.h. Space is allocated because a #define ALLOC is present at 
      * the top of the current file
      */

      for( ++argv,--argc; argc && *(p =  *argv) == '-'; ++argv, --argc )
      {
          while( *++p )
          {
              switch( *p )
              {
            OX( case 'a':  Make_parser = 0;                      )
            OX(            break;                                )

                case  'D':  Debug            = 1;            break;
                case  'g':  Public           = 1;            break;  
            LL( case  'f':  Uncompressed     = 1;            break;     )
                case  'l':  No_lines         = 1;            break;
            OX( case  'p':  Make_actions     = 0;            break;     )
                case  's':  Symbols          = 1;            break;
                case  'S':  Symbols          = 2;            break;
                case  't':  TimeIt           = 1;            break;
                case  'T':  Make_yyoutab     = 1;            break;
                case  'v':  Verbose          = 1;            break;
                case  'V':  Verbose          = 2;            break;
                case  'w':  No_warnings      = 1;            break;
                case  'W':  Warn_exit        = 1;            break;
				//case  'N':  NcNum			 = ++p;          
			    case  'N':  NcNum   = _ttol( ++p );                       
                            while( *p && isdigit( p[1] ) )              
                             ++p;                                   
                           break;                                      

			LL( case  'c':  Threshold   = atoi( ++p );                  )     
            LL(             while( *p && isdigit( p[1] ) )              )
            LL(                  ++p;                                   )
            LL(             break;                                      )
                default :   
					        fprintf(stderr, "<-%c>: illegal argument\n", *p);
                            #ifdef USAGE_MSG
					        printv(stderr, usage_msg );
							#endif	
                            exit( EXIT_ILLEGAL_ARG );
                }
            }

      }
      	   

	  if (NcNum >= MAXLONG)
	  {
	   exit( EXIT_ILLEGAL_ARG );
	  }
      
	  if( Verbose > 1)
          Use_stdout = 1;

          
      if( argc <= 0 )                     /* Input from standard input     */
          No_lines = 1;
          
      else if( argc > 1 )
      {
           _ftprintf( stderr, _T("Too many arguments.\n") );
           #ifdef USAGE_MSG
            printv ( stderr, usage_msg );
		   #endif
           exit   ( EXIT_TOO_MANY );
      }
      else                                /* argc == 1, input from file     */
      {
          if( ii_newfile( Input_file_name = *argv ) < 0 )
          {
              _stprintf( name_buf, _T("%0.70s.%s"), *argv, _T(DEF_EXT) );
              
              if( ii_newfile( Input_file_name = name_buf ) < 0 )
                  error( FATAL, _T("Can't open input file %s or %s: %s\n"),
                                                  *argv, name_buf, open_errmsg());
          }
      }
  }
  
  /*----------------------------------------------------------------------------*/
  
  static  _TINT       do_file()
  {
      /* Process the input file . Return the number of errors                   */
      
      struct _timeb start_time, end_time;
      long         time;
      
      _ftime( &start_time );   /* Initialize time now so that the difference    */
      end_time = start_time ; /* between times will be 0 if we don't build the */
                              /* tables. Note that I'm using structure assign- */
                              /* ment here.                                    */

      init_acts  ();          /* Initialize the action code                    */
      file_header();          /* Output #defines that you might want to change */


      VERBOSE( "parsing" );

      /* nows     ();*/       /* Make lex ignore white space until ws() is called */
      /* The above line is commented because  with "nows" enabled Lex is
       * not working properly
       */
      nows();
      yyparse();            /* Parse the entire input file                      */

      /* Print symbol table here */
      /* ptab( Symtab, pact, stdout, 1 ); */ /* CC */
       /* The code below has been changed because OCCS is sometimes
	    * stopping when there is no real problem. Therefore the OCCS
		* output file  is being created by force, even when errors are
		* present
	    */
		/* The below line has been commented out */
       //problems();

       VERBOSE( "analyzing grammar" );

       first();             /* Find first sets,                         */
       LL( follow(); )          /* FOLLOW sets,                             */
       LL( select(); )          /* and ll(1) select sets if this is llama   */

	   OX( patch(); )           /* Patch up the grammar (if this is occs)   */
                                    /* and output the actions                   */

       _ftime( &start_time );

      if( Make_parser )
      {
        VERBOSE( "making tables " );
        tables();                       /* generate the tables      */
      }

      _ftime ( &end_time        );
     
      if( Make_actions )
        tail();             /*and the tail end of the source file */
      
     

     if( Verbose )
     {
         time   = (long)((  end_time.time * 1000) + end_time.millitm) ;
         time  -= (long)((start_time.time * 1000) + start_time.millitm) ;
         printf( "time required to make tables: %1d.%-031d seconds\n",
                                                  (time/1000), (time%1000));
      }

      return yynerrs;
  }

#ifdef _DEBUG
  static  void  symbols()               /* Print the symbol table    */
  {
      FILE *fd;
      
      if( !(fd = fopen( SYM_FILE, "w")) )
          perror( SYM_FILE );
      else
      {
         print_symbols ( fd );
         fclose        ( fd );
      }
  }
  
#endif
  /*-----------------------------------------------------------------------------*/
  
  static void statistics(FILE *fp )
  {
       /* Print various statistics  */
       
       _TINT conflicts;                    /* Number of parse-table conflicts   */
       
       if( Verbose )
       {
          fprintf (fp, "\n"); 
          fprintf (fp, "%4d/%-4d terminals\n",     USED_TERMS,   NUMTERMS     );
          fprintf (fp, "%4d/%-4d nonterminals\n",  USED_NONTERMS, NUMNONTERMS );
          fprintf (fp, "%4d/%-4d productions\n",   Num_productions, MAXPROD   );
 LL(      fprintf (fp, "%4d      actions\n",       (Cur_act - MINACT) +1      ); )        
 OX(      lr_stats(fp);                                                          )
        }


        LL( conflicts = 0;                        )
        OX( conflicts = lr_conflicts(fp);         )

        if( fp == stdout )
            fp = stderr;

        if( Num_warnings - conflicts > 0 )
            fprintf(fp, "%4d      warnings\n", Num_warnings - conflicts);

        if(yynerrs)
           fprintf(fp, "%4d       hard errors\n", yynerrs    );
 }

 /*-----------------------------------------------------------------------------*/

   void output(_TCHAR *fmt, ...)
 {
     /* Works like printf(), but writes to the output file. See also: the
      * outc() macro in parser.h
      */

      va_list   args;
      va_start( args,   fmt );
      _vftprintf( Output, fmt, args );
  }

  /*-----------------------------------------------------------------------------*/

    void document(_TCHAR *fmt, ... )
  {
     /* Works like printf() but writes to yyout.doc (provided that the file
      * is being created
      */

      va_list   args;

      if( Doc_file )
      {
          va_start( args,    fmt );
          _vftprintf( Doc_file, fmt, args );
      }
  }

 void  document_to(FILE *fp )
  {
     /* Change document-file output to the indicated stream, or to previous
      * stream if fp=NULL.
      */

      static FILE  *oldfp;

      if( fp )
      {
          oldfp    = Doc_file;
          Doc_file = fp;
      }
      else
         Doc_file = oldfp;
   }

   /*---------------------------------------------------------------------------*/

     void lerror(_TINT fatal,_TCHAR *fmt, ... )
   {
    /* This error-processing routine automatically genarates a line number for
     * the error. If "fatal" is true, exit() is called
     */

     va_list     args;
     _TCHAR        *type;
     extern _TINT  yylineno;

     if( fatal == WARNING )
     {
         ++Num_warnings;
         if( No_warnings )
             return;
         type = _T("WARNING: ");
     }
     else
     {
         type = _T("ERROR: ");
           ++yynerrs;
     }

     va_start( args,  fmt);
     fprintf( stdout, "%s %s (%s, line %d): ", PROG_NAME, type,
                                              Input_file_name, yylineno );
     _vftprintf( stdout, fmt, args );

     OX( if( Verbose && Doc_file )                                       )
     OX( {                                                               )
     OX(    fprintf ( Doc_file, "%s (line %d) ", type, yylineno );       )
     OX(    _vftprintf( Doc_file, fmt, args );                             )
     OX( }                                                               )



     if( fatal == FATAL )
         exit( EXIT_OTHER );
}


  void error(_TINT fatal,_TCHAR *fmt, ... )
{
   /* This error routine works like lerror() except that no line number is
    * generated. The global error count is still modified, however.
    */

    va_list   args;
    _TCHAR      *type;

    if( fatal == WARNING )
    {
        ++Num_warnings;
//        if( No_warnings )
//            return;
        type = _T("WARNING: ");
    }
    else
    {
//        if( No_warnings && fatal != FATAL)
//            return;
        type = _T("ERROR: ");
        ++yynerrs;
    }

    va_start ( args,  fmt   );
    _ftprintf  ( stdout, type );
    _vftprintf ( stdout, fmt, args );

    OX( if( Verbose && Doc_file )                           )
    OX( {                                                   )
    OX(     _ftprintf  ( Doc_file, type );                    )
    OX(     _vftprintf ( Doc_file, fmt, args );               )
    OX( }                                                   )

    if( fatal == FATAL )
        exit( EXIT_OTHER );
 }


  _TCHAR   *open_errmsg()
 {

 /* Return an error message that makes sense for a bad open            */

    extern  errno;
    switch( errno )
    {
     case EACCES:              return _T("File is read only or a directory");
     case EEXIST:              return _T("File already exists");
     case EMFILE:              return _T("Too many open files");
     case ENOENT:              return _T("File not found");
     default:                  return _T("Reason unknown");
    }
 }



/*------------------------------------------------------------------------------*/

void tail()
{
   /* Copy the remainder of input file to standard output. Yyparse will have
    * terminated with the input pointer just past the %%. Attribute mapping
    * ($$ to Yyval, $N to a stack reference, etc.) is done by the do_dollar()
    * call
    *
    * On entry, the parser will have read one token too far, so the first
    * thing to do is print the current line number and lexeme.
    */

    extern  _TINT  yylineno ;       /* Lex generated */
    extern  _TCHAR *yytext  ;       /* Lex generated */
    _TINT          c, i, sign;
    _TCHAR         fname[80],*p;   /* field name in $<...>n */

    yylex();                           /* Push past %% */
    output( _T("\n\n%s"), yytext );       /* Output newline following %%   */
    
    if( !No_lines )
        output( _T("\n#line %d \"%s\"\n"), yylineno, Input_file_name );
        
        ii_unterm();              /* Lex will have terminated yytext */
        
        while( (c = ii_advance()) != 0 )
        {
           if( c == -1 )
           {
               ii_flush(1);
               continue;
           }
           else if( c == '$' )
           {
                ii_mark_start();
                
                if( (c = ii_advance()) != '<' )
                     *fname = '\0';
                else                             /* extract name in $<foo>1 */
                {
                    p = fname;
                    for(i=sizeof(fname); --i > 0 && (c=ii_advance()) != '>';)
                        *p++ =c;
                        *p++ = '\0';
                    if(c == '>' )            /* truncate name if necessary  */
                       c = ii_advance();
                }
                
                if( c== '$' )
                    output( do_dollar( DOLLAR_DOLLAR, -1, 0, NULL, fname ) );
                else
                {
                
                    if( c != '-' )
                        sign = 1;
                    else
                    {
                        sign = -1;
                        c = ii_advance();
                    }
                    
                    for( i = 0; isdigit(c); c = ii_advance() )
                        i = (i * 10) + (c - '0');
                        
                    ii_pushback(1);
                    output( do_dollar( i * sign, -1, ii_lineno(), NULL, fname ) );
                }
            }
            else if( c!= '\r' )
                 outc(c );
       }
}                                                               



