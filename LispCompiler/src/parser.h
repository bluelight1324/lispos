/*
 * parser.h - Lisp Parser
 *
 * Parses a stream of tokens into Lisp S-expressions (AST).
 * This can work with LALRGen-generated tables or as a
 * recursive descent parser (which is simpler for Lisp).
 */

#ifndef PARSER_H
#define PARSER_H

#include "lisp.h"
#include "lexer.h"

/* Parser state */
typedef struct {
    Lexer *lexer;
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

/* Initialize parser with a lexer */
void parser_init(Parser *parser, Lexer *lexer);

/* Parse a single expression */
LispObject *parse_expression(Parser *parser);

/* Parse all expressions (for file input) */
LispObject *parse_program(Parser *parser);

/* Check if parser had errors */
int parser_had_error(Parser *parser);

/* Get error message */
const char *parser_error_message(Parser *parser);

#endif /* PARSER_H */
