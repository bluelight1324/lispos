//#include <standard.h>
/* printv.c  Print argv like Vector Array    Page 734                  */
#include <stdio.h>
#include <tchar.h>

void    printv(FILE *fp,_TCHAR **argv )
{
     /* Print an argv-like array of pointers to strings, one string per line.
      * The array must be NULL terminated.
      */

      while( *argv )
          fprintf( fp, "%s\n", *argv++ );
}

#ifdef _DEBUG
void    comment(FILE *fp,_TCHAR **argv )
{
    /* Works like printv except that the array is printed as a C comment       */

    fprintf(fp, "\n/*---------------------------------------------------------\n");
    while( *argv )
           fprintf(fp, " * %s\n", *argv++ );
    fprintf(fp, " */\n\n");
}

#endif
