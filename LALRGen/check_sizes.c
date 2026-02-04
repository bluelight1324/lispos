// Quick structure size check - add to LALRGen
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include "debug.h"
#include "set.h"
#include "hash.h"
#include "yystate.h"

void print_structure_sizes(void) {
    fprintf(stderr, "\n=== Structure Size Analysis (x64) ===\n\n");

    fprintf(stderr, "Basic types:\n");
    fprintf(stderr, "  sizeof(_TINT) = %zu\n", sizeof(_TINT));
    fprintf(stderr, "  sizeof(_TCHAR) = %zu\n", sizeof(_TCHAR));
    fprintf(stderr, "  sizeof(_TUCHAR) = %zu\n", sizeof(_TUCHAR));
    fprintf(stderr, "  sizeof(void*) = %zu\n", sizeof(void*));
    fprintf(stderr, "  sizeof(STATENUM) = %zu\n", sizeof(STATENUM));
    fprintf(stderr, "\n");

    fprintf(stderr, "ITEM structure:\n");
    fprintf(stderr, "  sizeof(ITEM) = %zu\n", sizeof(ITEM));
    fprintf(stderr, "  offsetof(ITEM, prod_num) = %zu\n", offsetof(ITEM, prod_num));
    fprintf(stderr, "  offsetof(ITEM, prod) = %zu\n", offsetof(ITEM, prod));
    fprintf(stderr, "  offsetof(ITEM, right_of_dot) = %zu\n", offsetof(ITEM, right_of_dot));
    fprintf(stderr, "  offsetof(ITEM, dot_posn) = %zu\n", offsetof(ITEM, dot_posn));
    fprintf(stderr, "  offsetof(ITEM, lookaheads) = %zu\n", offsetof(ITEM, lookaheads));
    fprintf(stderr, "\n");

    fprintf(stderr, "STATE structure:\n");
    fprintf(stderr, "  sizeof(STATE) = %zu\n", sizeof(STATE));
    fprintf(stderr, "  offsetof(STATE, kernel_items) = %zu\n", offsetof(STATE, kernel_items));
    fprintf(stderr, "  offsetof(STATE, epsilon_items) = %zu\n", offsetof(STATE, epsilon_items));
    /* Can't use offsetof on bitfields - skip nkitems */
    fprintf(stderr, "  offsetof(STATE, num) = %zu\n", offsetof(STATE, num));
    fprintf(stderr, "  (Note: bitfields nkitems/neitems/closed are between epsilon_items and num)\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "PRODUCTION structure:\n");
    fprintf(stderr, "  sizeof(PRODUCTION) = %zu\n", sizeof(PRODUCTION));
    fprintf(stderr, "\n");

    fprintf(stderr, "SET structure:\n");
    fprintf(stderr, "  sizeof(SET) = %zu\n", sizeof(SET));
    fprintf(stderr, "\n");

    fflush(stderr);
}
