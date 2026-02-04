%{
/*
 * lisp_grammar.y - Lisp Grammar for LALRGen
 *
 * A complete grammar for parsing Lisp/Scheme S-expressions.
 * This grammar is conflict-free and suitable for LALR(1) parsing.
 */

#include <stdio.h>

/* These will be defined by the parser driver */
extern void *yylval;
extern int yylineno;

/* Forward declarations for semantic actions */
void *make_nil_node(void);
void *make_boolean_node(int value);
void *make_number_node(double value);
void *make_string_node(const char *str);
void *make_symbol_node(const char *name);
void *make_character_node(char c);
void *make_cons_node(void *car, void *cdr);
void *make_quote_node(const char *quote_type, void *datum);
void *append_to_list(void *list, void *item);

%}

/* Token declarations */
%token LPAREN RPAREN DOT
%token QUOTE QUASIQUOTE UNQUOTE UNQUOTE_SPLICE
%token SYMBOL NUMBER STRING BOOLEAN CHARACTER

/* Start symbol */
%start program

%%

/*
 * A program is a sequence of datums (S-expressions).
 * This allows parsing entire files with multiple top-level expressions.
 */
program
    : datum_list
        { /* parse_result = $1; */ }
    ;

datum_list
    : /* empty */
        { $$ = make_nil_node(); }
    | datum_list datum
        { $$ = append_to_list($1, $2); }
    ;

/*
 * A datum is any Lisp expression: atom, list, or quoted form.
 */
datum
    : atom
        { $$ = $1; }
    | list
        { $$ = $1; }
    | quoted_datum
        { $$ = $1; }
    ;

/*
 * Atoms: the indivisible elements of Lisp
 */
atom
    : SYMBOL
        { $$ = make_symbol_node((const char *)yylval); }
    | NUMBER
        { $$ = make_number_node(*(double *)&yylval); }
    | STRING
        { $$ = make_string_node((const char *)yylval); }
    | BOOLEAN
        { $$ = make_boolean_node((int)(size_t)yylval); }
    | CHARACTER
        { $$ = make_character_node((char)(size_t)yylval); }
    ;

/*
 * Lists: the compound forms of Lisp
 *
 * Supports:
 * - Empty list: ()
 * - Proper list: (a b c)
 * - Dotted/improper list: (a b . c)
 */
list
    : LPAREN RPAREN
        { $$ = make_nil_node(); }
    | LPAREN list_contents RPAREN
        { $$ = $2; }
    | LPAREN list_contents DOT datum RPAREN
        {
            /* Set the cdr of the last cons to datum instead of nil */
            $$ = $2;
            /* set_list_tail($$, $4); */
        }
    ;

list_contents
    : datum
        { $$ = make_cons_node($1, make_nil_node()); }
    | list_contents datum
        { $$ = append_to_list($1, $2); }
    ;

/*
 * Quoted expressions - syntactic sugar
 *
 * 'x        -> (quote x)
 * `x        -> (quasiquote x)
 * ,x        -> (unquote x)
 * ,@x       -> (unquote-splicing x)
 */
quoted_datum
    : QUOTE datum
        { $$ = make_quote_node("quote", $2); }
    | QUASIQUOTE datum
        { $$ = make_quote_node("quasiquote", $2); }
    | UNQUOTE datum
        { $$ = make_quote_node("unquote", $2); }
    | UNQUOTE_SPLICE datum
        { $$ = make_quote_node("unquote-splicing", $2); }
    ;

%%

/*
 * Error handling - called by parser on syntax errors
 */
void yyerror(const char *msg) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, msg);
}
