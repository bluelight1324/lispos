/*
 * lexer.h - Lisp Lexical Analyzer
 *
 * Tokenizes Lisp source code into a stream of tokens.
 */

#ifndef LEXER_H
#define LEXER_H

#include "lisp.h"

/* Token types */
typedef enum {
    /* Delimiters */
    TOK_LPAREN,         /* ( */
    TOK_RPAREN,         /* ) */
    TOK_DOT,            /* . */

    /* Quote syntax */
    TOK_QUOTE,          /* ' */
    TOK_QUASIQUOTE,     /* ` */
    TOK_UNQUOTE,        /* , */
    TOK_UNQUOTE_SPLICE, /* ,@ */

    /* Atoms */
    TOK_SYMBOL,         /* foo, +, list?, set! */
    TOK_NUMBER,         /* 42, 3.14, -17 */
    TOK_STRING,         /* "hello" */
    TOK_BOOLEAN,        /* #t, #f */
    TOK_CHARACTER,      /* #\a, #\newline */

    /* Special */
    TOK_EOF,
    TOK_ERROR
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;
    char *text;         /* Token text (owned, must be freed) */
    int line;
    int column;
    union {
        double number;
        char *string;   /* For STRING tokens (owned) */
        char character;
        int boolean;
    } value;
} Token;

/* Lexer state */
typedef struct {
    const char *source;     /* Source code */
    const char *start;      /* Start of current token */
    const char *current;    /* Current position */
    int line;
    int column;
    int start_line;
    int start_column;
} Lexer;

/* Initialize lexer with source code */
void lexer_init(Lexer *lex, const char *source);

/* Get the next token */
Token lexer_next_token(Lexer *lex);

/* Free token resources */
void token_free(Token *tok);

/* Token type name for debugging */
const char *token_type_name(TokenType type);

/* Peek at current character without consuming */
char lexer_peek(Lexer *lex);

/* Peek at next character without consuming */
char lexer_peek_next(Lexer *lex);

#endif /* LEXER_H */
