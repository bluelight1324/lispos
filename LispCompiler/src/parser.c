/*
 * parser.c - Lisp Parser Implementation
 *
 * A recursive descent parser for Lisp S-expressions.
 * While LALRGen can generate parsers for Lisp, recursive descent
 * is actually ideal for the simple S-expression grammar.
 */

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Error message buffer */
static char error_message[256];

/* Advance to next token */
static void advance(Parser *parser) {
    parser->previous = parser->current;

    while (1) {
        parser->current = lexer_next_token(parser->lexer);

        if (parser->current.type != TOK_ERROR) break;

        /* Report lexer error */
        parser->had_error = 1;
        snprintf(error_message, sizeof(error_message),
                 "Lexer error at line %d: %s",
                 parser->current.line, parser->current.text);
        token_free(&parser->current);
    }
}

/* Check current token type */
static int check(Parser *parser, TokenType type) {
    return parser->current.type == type;
}

/* Consume token if it matches */
static int match(Parser *parser, TokenType type) {
    if (!check(parser, type)) return 0;
    advance(parser);
    return 1;
}

/* Report error */
static void error_at(Parser *parser, Token *token, const char *message) {
    if (parser->panic_mode) return;
    parser->panic_mode = 1;
    parser->had_error = 1;

    snprintf(error_message, sizeof(error_message),
             "Error at line %d, column %d: %s (got '%s')",
             token->line, token->column, message,
             token->text ? token->text : "?");
}

static void error_current(Parser *parser, const char *message) {
    error_at(parser, &parser->current, message);
}

/* Expect and consume a specific token */
static void consume(Parser *parser, TokenType type, const char *message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    error_current(parser, message);
}

/* Forward declaration */
static LispObject *parse_datum(Parser *parser);

/* Parse an atom */
static LispObject *parse_atom(Parser *parser) {
    switch (parser->current.type) {
        case TOK_SYMBOL: {
            LispObject *obj = make_symbol(parser->current.text);
            advance(parser);
            return obj;
        }

        case TOK_NUMBER: {
            LispObject *obj = make_number(parser->current.value.number);
            advance(parser);
            return obj;
        }

        case TOK_STRING: {
            LispObject *obj = make_string(parser->current.text);
            advance(parser);
            return obj;
        }

        case TOK_BOOLEAN: {
            LispObject *obj = make_boolean(parser->current.value.boolean);
            advance(parser);
            return obj;
        }

        case TOK_CHARACTER: {
            LispObject *obj = make_character(parser->current.value.character);
            advance(parser);
            return obj;
        }

        default:
            return NULL;
    }
}

/* Parse a list */
static LispObject *parse_list(Parser *parser) {
    consume(parser, TOK_LPAREN, "Expected '('");

    /* Empty list */
    if (match(parser, TOK_RPAREN)) {
        return make_nil();
    }

    /* Build list of datums */
    LispObject *head = NULL;
    LispObject *tail = NULL;

    while (!check(parser, TOK_RPAREN) && !check(parser, TOK_DOT) &&
           !check(parser, TOK_EOF)) {

        LispObject *datum = parse_datum(parser);
        if (!datum) {
            error_current(parser, "Expected expression");
            return make_nil();
        }

        LispObject *new_cell = make_cons(datum, make_nil());

        if (tail) {
            tail->cons.cdr = new_cell;
            tail = new_cell;
        } else {
            head = tail = new_cell;
        }
    }

    /* Check for dotted pair */
    if (match(parser, TOK_DOT)) {
        if (!tail) {
            error_current(parser, "Invalid dotted pair - no elements before dot");
            return make_nil();
        }

        LispObject *datum = parse_datum(parser);
        if (!datum) {
            error_current(parser, "Expected expression after dot");
            return make_nil();
        }

        /* Set the cdr of the last cell to the datum */
        tail->cons.cdr = datum;
    }

    consume(parser, TOK_RPAREN, "Expected ')'");

    return head ? head : make_nil();
}

/* Parse a quoted expression */
static LispObject *parse_quoted(Parser *parser) {
    const char *quote_sym = NULL;

    switch (parser->current.type) {
        case TOK_QUOTE:
            quote_sym = "quote";
            break;
        case TOK_QUASIQUOTE:
            quote_sym = "quasiquote";
            break;
        case TOK_UNQUOTE:
            quote_sym = "unquote";
            break;
        case TOK_UNQUOTE_SPLICE:
            quote_sym = "unquote-splicing";
            break;
        default:
            return NULL;
    }

    advance(parser);  /* consume quote token */

    LispObject *datum = parse_datum(parser);
    if (!datum) {
        error_current(parser, "Expected expression after quote");
        return make_nil();
    }

    /* Build (quote datum) or similar */
    return make_cons(make_symbol(quote_sym),
                     make_cons(datum, make_nil()));
}

/* Parse any datum (expression) */
static LispObject *parse_datum(Parser *parser) {
    /* Quoted expressions */
    if (check(parser, TOK_QUOTE) || check(parser, TOK_QUASIQUOTE) ||
        check(parser, TOK_UNQUOTE) || check(parser, TOK_UNQUOTE_SPLICE)) {
        return parse_quoted(parser);
    }

    /* List */
    if (check(parser, TOK_LPAREN)) {
        return parse_list(parser);
    }

    /* Atom */
    LispObject *atom = parse_atom(parser);
    if (atom) {
        return atom;
    }

    /* Nothing recognized */
    return NULL;
}

/* Initialize parser */
void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->had_error = 0;
    parser->panic_mode = 0;
    error_message[0] = '\0';

    /* Prime the parser with the first token */
    advance(parser);
}

/* Parse a single expression */
LispObject *parse_expression(Parser *parser) {
    parser->had_error = 0;
    parser->panic_mode = 0;

    if (check(parser, TOK_EOF)) {
        return NULL;
    }

    LispObject *expr = parse_datum(parser);

    if (!expr && !parser->had_error) {
        error_current(parser, "Expected expression");
    }

    return expr;
}

/* Parse all expressions (returns a list) */
LispObject *parse_program(Parser *parser) {
    parser->had_error = 0;
    parser->panic_mode = 0;

    LispObject *head = NULL;
    LispObject *tail = NULL;

    while (!check(parser, TOK_EOF)) {
        LispObject *expr = parse_datum(parser);

        if (!expr) {
            if (!parser->had_error) {
                error_current(parser, "Expected expression");
            }
            break;
        }

        LispObject *new_cell = make_cons(expr, make_nil());

        if (tail) {
            tail->cons.cdr = new_cell;
            tail = new_cell;
        } else {
            head = tail = new_cell;
        }
    }

    return head ? head : make_nil();
}

/* Check if parser had errors */
int parser_had_error(Parser *parser) {
    return parser->had_error;
}

/* Get error message */
const char *parser_error_message(Parser *parser) {
    return error_message;
}
