/*
 * lexer.c - Lisp Lexical Analyzer Implementation
 */

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Initialize lexer */
void lexer_init(Lexer *lex, const char *source) {
    lex->source = source;
    lex->start = source;
    lex->current = source;
    lex->line = 1;
    lex->column = 1;
    lex->start_line = 1;
    lex->start_column = 1;
}

/* Character utilities */
static int is_at_end(Lexer *lex) {
    return *lex->current == '\0';
}

static char advance(Lexer *lex) {
    char c = *lex->current++;
    if (c == '\n') {
        lex->line++;
        lex->column = 1;
    } else {
        lex->column++;
    }
    return c;
}

char lexer_peek(Lexer *lex) {
    return *lex->current;
}

char lexer_peek_next(Lexer *lex) {
    if (is_at_end(lex)) return '\0';
    return lex->current[1];
}

static int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

/* Symbol character rules - very permissive in Lisp */
static int is_symbol_char(char c) {
    if (c == '\0' || is_whitespace(c)) return 0;
    if (c == '(' || c == ')') return 0;
    if (c == '"' || c == ';') return 0;
    if (c == '\'' || c == '`' || c == ',') return 0;
    return 1;
}

static int is_symbol_start(char c) {
    return is_symbol_char(c) && c != '#';
}

/* Create a token */
static Token make_token(Lexer *lex, TokenType type) {
    Token tok;
    tok.type = type;
    int len = (int)(lex->current - lex->start);
    tok.text = (char *)malloc(len + 1);
    memcpy(tok.text, lex->start, len);
    tok.text[len] = '\0';
    tok.line = lex->start_line;
    tok.column = lex->start_column;
    tok.value.number = 0;
    return tok;
}

static Token make_error_token(Lexer *lex, const char *message) {
    Token tok;
    tok.type = TOK_ERROR;
    tok.text = strdup(message);
    tok.line = lex->line;
    tok.column = lex->column;
    return tok;
}

/* Skip whitespace and comments */
static void skip_whitespace(Lexer *lex) {
    while (1) {
        char c = lexer_peek(lex);

        /* Whitespace */
        if (is_whitespace(c)) {
            advance(lex);
            continue;
        }

        /* Line comment */
        if (c == ';') {
            while (lexer_peek(lex) != '\n' && !is_at_end(lex)) {
                advance(lex);
            }
            continue;
        }

        break;
    }
}

/* Read a number */
static Token read_number(Lexer *lex) {
    /* Handle negative sign */
    if (lexer_peek(lex) == '-') {
        advance(lex);
    }

    /* Integer part */
    while (is_digit(lexer_peek(lex))) {
        advance(lex);
    }

    /* Fractional part */
    if (lexer_peek(lex) == '.' && is_digit(lexer_peek_next(lex))) {
        advance(lex);  /* consume '.' */
        while (is_digit(lexer_peek(lex))) {
            advance(lex);
        }
    }

    /* Exponent */
    if (lexer_peek(lex) == 'e' || lexer_peek(lex) == 'E') {
        advance(lex);
        if (lexer_peek(lex) == '+' || lexer_peek(lex) == '-') {
            advance(lex);
        }
        while (is_digit(lexer_peek(lex))) {
            advance(lex);
        }
    }

    Token tok = make_token(lex, TOK_NUMBER);
    tok.value.number = strtod(tok.text, NULL);
    return tok;
}

/* Read a string */
static Token read_string(Lexer *lex) {
    advance(lex);  /* consume opening quote */

    /* Accumulate string content */
    size_t capacity = 64;
    size_t length = 0;
    char *buffer = (char *)malloc(capacity);

    while (!is_at_end(lex) && lexer_peek(lex) != '"') {
        char c = advance(lex);

        if (c == '\\') {
            /* Escape sequence */
            if (is_at_end(lex)) {
                free(buffer);
                return make_error_token(lex, "Unterminated string");
            }
            c = advance(lex);
            switch (c) {
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case 'r':  c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"':  c = '"';  break;
                default:
                    /* Keep the character as-is */
                    break;
            }
        }

        /* Grow buffer if needed */
        if (length + 1 >= capacity) {
            capacity *= 2;
            buffer = (char *)realloc(buffer, capacity);
        }
        buffer[length++] = c;
    }

    if (is_at_end(lex)) {
        free(buffer);
        return make_error_token(lex, "Unterminated string");
    }

    advance(lex);  /* consume closing quote */
    buffer[length] = '\0';

    Token tok = make_token(lex, TOK_STRING);
    free(tok.text);
    tok.text = buffer;
    tok.value.string = buffer;
    return tok;
}

/* Read a symbol */
static Token read_symbol(Lexer *lex) {
    while (is_symbol_char(lexer_peek(lex))) {
        advance(lex);
    }
    return make_token(lex, TOK_SYMBOL);
}

/* Read a hash literal (#t, #f, #\char) */
static Token read_hash_literal(Lexer *lex) {
    advance(lex);  /* consume '#' */

    if (is_at_end(lex)) {
        return make_error_token(lex, "Unexpected end after #");
    }

    char c = lexer_peek(lex);

    /* Boolean */
    if (c == 't' || c == 'T') {
        advance(lex);
        Token tok = make_token(lex, TOK_BOOLEAN);
        tok.value.boolean = 1;
        return tok;
    }

    if (c == 'f' || c == 'F') {
        advance(lex);
        Token tok = make_token(lex, TOK_BOOLEAN);
        tok.value.boolean = 0;
        return tok;
    }

    /* Character literal */
    if (c == '\\') {
        advance(lex);  /* consume backslash */

        if (is_at_end(lex)) {
            return make_error_token(lex, "Unexpected end in character literal");
        }

        /* Check for named characters */
        const char *start = lex->current;
        while (!is_at_end(lex) && !is_whitespace(lexer_peek(lex)) &&
               lexer_peek(lex) != '(' && lexer_peek(lex) != ')') {
            advance(lex);
        }

        int len = (int)(lex->current - start);
        Token tok = make_token(lex, TOK_CHARACTER);

        if (len == 1) {
            tok.value.character = start[0];
        } else if (len == 7 && strncmp(start, "newline", 7) == 0) {
            tok.value.character = '\n';
        } else if (len == 5 && strncmp(start, "space", 5) == 0) {
            tok.value.character = ' ';
        } else if (len == 3 && strncmp(start, "tab", 3) == 0) {
            tok.value.character = '\t';
        } else if (len == 6 && strncmp(start, "return", 6) == 0) {
            tok.value.character = '\r';
        } else {
            free(tok.text);
            return make_error_token(lex, "Unknown character name");
        }

        return tok;
    }

    return make_error_token(lex, "Unknown # literal");
}

/* Main tokenizer function */
Token lexer_next_token(Lexer *lex) {
    skip_whitespace(lex);

    lex->start = lex->current;
    lex->start_line = lex->line;
    lex->start_column = lex->column;

    if (is_at_end(lex)) {
        return make_token(lex, TOK_EOF);
    }

    char c = advance(lex);

    /* Single-character tokens */
    switch (c) {
        case '(':
            return make_token(lex, TOK_LPAREN);
        case ')':
            return make_token(lex, TOK_RPAREN);
        case '\'':
            return make_token(lex, TOK_QUOTE);
        case '`':
            return make_token(lex, TOK_QUASIQUOTE);
        case ',':
            if (lexer_peek(lex) == '@') {
                advance(lex);
                return make_token(lex, TOK_UNQUOTE_SPLICE);
            }
            return make_token(lex, TOK_UNQUOTE);
    }

    /* Dot - check if it's not part of a number */
    if (c == '.') {
        if (!is_digit(lexer_peek(lex)) && !is_symbol_char(lexer_peek(lex))) {
            return make_token(lex, TOK_DOT);
        }
        /* Otherwise fall through to symbol handling */
        lex->current--;  /* Back up */
        lex->column--;
    }

    /* Number (including negative) */
    if (is_digit(c) || (c == '-' && is_digit(lexer_peek(lex)))) {
        lex->current--;  /* Back up */
        lex->column--;
        return read_number(lex);
    }

    /* String */
    if (c == '"') {
        lex->current--;  /* Back up */
        lex->column--;
        return read_string(lex);
    }

    /* Hash literals */
    if (c == '#') {
        lex->current--;  /* Back up */
        lex->column--;
        return read_hash_literal(lex);
    }

    /* Symbol */
    if (is_symbol_start(c)) {
        lex->current--;  /* Back up */
        lex->column--;
        return read_symbol(lex);
    }

    return make_error_token(lex, "Unexpected character");
}

/* Free token resources */
void token_free(Token *tok) {
    if (tok->text) {
        free(tok->text);
        tok->text = NULL;
    }
}

/* Token type name for debugging */
const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_LPAREN:         return "LPAREN";
        case TOK_RPAREN:         return "RPAREN";
        case TOK_DOT:            return "DOT";
        case TOK_QUOTE:          return "QUOTE";
        case TOK_QUASIQUOTE:     return "QUASIQUOTE";
        case TOK_UNQUOTE:        return "UNQUOTE";
        case TOK_UNQUOTE_SPLICE: return "UNQUOTE_SPLICE";
        case TOK_SYMBOL:         return "SYMBOL";
        case TOK_NUMBER:         return "NUMBER";
        case TOK_STRING:         return "STRING";
        case TOK_BOOLEAN:        return "BOOLEAN";
        case TOK_CHARACTER:      return "CHARACTER";
        case TOK_EOF:            return "EOF";
        case TOK_ERROR:          return "ERROR";
        default:                 return "UNKNOWN";
    }
}
