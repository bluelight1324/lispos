# Building a Lisp Compiler with LALRGen

A comprehensive guide to building a complete Lisp/Scheme compiler using the LALRGen
LALR(1) parser generator.

## Table of Contents

1. [Overview](#overview)
2. [Why Lisp is Perfect for LALR Parsing](#why-lisp-is-perfect-for-lalr-parsing)
3. [Compiler Architecture](#compiler-architecture)
4. [Phase 1: Lexical Analysis](#phase-1-lexical-analysis)
5. [Phase 2: Syntax Analysis with LALRGen](#phase-2-syntax-analysis-with-lalrgen)
6. [Phase 3: Semantic Analysis](#phase-3-semantic-analysis)
7. [Phase 4: Evaluation Strategies](#phase-4-evaluation-strategies)
8. [Phase 5: Compilation to Native Code](#phase-5-compilation-to-native-code)
9. [Complete Build Workflow](#complete-build-workflow)
10. [Example Implementation](#example-implementation)
11. [Extending the Language](#extending-the-language)

---

## Overview

This document describes how to build a complete Lisp/Scheme compiler using
LALRGen as the parser generator. Lisp is unique among programming languages
because its syntax is based on S-expressions (symbolic expressions), making
it one of the easiest languages to parse.

### What We'll Build

- A complete Lisp interpreter/compiler
- Support for core Lisp primitives (car, cdr, cons, etc.)
- Lambda expressions and closures
- Recursion and tail-call optimization
- Macros and quasiquotation
- Optional: compilation to native x86-64 code

### Prerequisites

- LALRGen x64 build
- C/C++ compiler (MSVC, GCC, or Clang)
- Basic understanding of Lisp/Scheme syntax
- Familiarity with compiler concepts

### Why Use LALRGen for Lisp?

While Lisp's simple syntax means you *could* write a recursive descent parser
by hand, using LALRGen provides:

1. **Formal grammar specification** - Self-documenting syntax definition
2. **Automatic error recovery** - Built-in parse error handling
3. **Extensibility** - Easy to add new syntactic forms
4. **Consistency** - Same toolchain as other language implementations

---

## Why Lisp is Perfect for LALR Parsing

### The Simplicity of S-Expressions

Lisp syntax is based on S-expressions, which have an extremely simple grammar:

```
s-expression = atom | list
atom         = symbol | number | string | boolean
list         = '(' s-expression* ')' | '(' s-expression+ '.' s-expression ')'
```

This simplicity translates to a **conflict-free LALR(1) grammar** with fewer
than 20 productions - compared to 150+ for ANSI C.

### No Syntactic Ambiguity

Unlike C (typedef names), C++ (templates), or JavaScript (ASI), Lisp has:

- **No operator precedence issues** - Operators are just function calls
- **No context-sensitive lexing** - Every token is unambiguous
- **No reserved words in identifiers** - `if`, `lambda`, etc. are symbols
- **Uniform syntax** - Everything is a list or atom

### Grammar Comparison

| Language | Productions | Conflicts | Lexer Hacks |
|----------|-------------|-----------|-------------|
| **Lisp/Scheme** | ~15 | 0 | None |
| Pascal | ~80 | 1 | None |
| Go | ~100 | 0 | None |
| C | ~150 | 6 | typedef |
| C++ | ~400+ | Many | Many |

---

## Compiler Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Lisp Source File                         │
│            (+ 1 (* 2 3)) or (define (fact n) ...)               │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 1: LEXER (Scanner)                                       │
│  - Character stream → Token stream                              │
│  - Recognizes: ( ) ' ` , ,@ . symbols numbers strings           │
│  - Handles comments (;) and string escapes                      │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 2: PARSER (LALRGen-generated)                            │
│  - Token stream → S-expression tree (AST)                       │
│  - Validates balanced parentheses                               │
│  - Handles dotted pairs and quote sugar                         │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 3: SEMANTIC ANALYZER / MACRO EXPANDER                    │
│  - Expand macros (defmacro, syntax-rules)                       │
│  - Resolve free variables                                       │
│  - Build environment/closure structures                         │
└─────────────────────────────────────────────────────────────────┘
                                │
                ┌───────────────┴───────────────┐
                ▼                               ▼
┌───────────────────────────┐   ┌───────────────────────────────┐
│  Path A: INTERPRETER      │   │  Path B: COMPILER              │
│  - Direct evaluation      │   │  - IR generation               │
│  - Tree-walking           │   │  - Optimization                │
│  - REPL support           │   │  - Code generation (x86-64)    │
└───────────────────────────┘   └───────────────────────────────┘
                │                               │
                ▼                               ▼
┌───────────────────────────┐   ┌───────────────────────────────┐
│        Result Value       │   │      Executable / Object      │
└───────────────────────────┘   └───────────────────────────────┘
```

---

## Phase 1: Lexical Analysis

### Token Types

Lisp has remarkably few token types:

```c
typedef enum {
    // Delimiters
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_DOT,            // .

    // Quote syntax
    TOK_QUOTE,          // '
    TOK_QUASIQUOTE,     // `
    TOK_UNQUOTE,        // ,
    TOK_UNQUOTE_SPLICE, // ,@

    // Atoms
    TOK_SYMBOL,         // foo, +, list?, set!
    TOK_NUMBER,         // 42, 3.14, -17
    TOK_STRING,         // "hello"
    TOK_BOOLEAN,        // #t, #f
    TOK_CHARACTER,      // #\a, #\newline

    // Special
    TOK_EOF
} TokenType;
```

### Lexer Implementation

```c
// lexer.h
typedef struct {
    TokenType type;
    char *text;
    int line, column;
    union {
        double number;
        char *string;
        char character;
        int boolean;
    } value;
} Token;

typedef struct {
    const char *source;
    const char *current;
    int line, column;
} Lexer;

// lexer.c
Token lexer_next_token(Lexer *lex) {
    skip_whitespace_and_comments(lex);

    if (*lex->current == '\0') {
        return make_token(TOK_EOF, NULL);
    }

    char c = *lex->current;

    // Single-character tokens
    switch (c) {
        case '(': advance(lex); return make_token(TOK_LPAREN, "(");
        case ')': advance(lex); return make_token(TOK_RPAREN, ")");
        case '\'': advance(lex); return make_token(TOK_QUOTE, "'");
        case '`': advance(lex); return make_token(TOK_QUASIQUOTE, "`");
        case ',':
            advance(lex);
            if (*lex->current == '@') {
                advance(lex);
                return make_token(TOK_UNQUOTE_SPLICE, ",@");
            }
            return make_token(TOK_UNQUOTE, ",");
    }

    // Dot (must check it's not part of a number or symbol)
    if (c == '.' && !is_symbol_char(peek_next(lex))) {
        advance(lex);
        return make_token(TOK_DOT, ".");
    }

    // Numbers (including negative)
    if (is_digit(c) || (c == '-' && is_digit(peek_next(lex)))) {
        return read_number(lex);
    }

    // Strings
    if (c == '"') {
        return read_string(lex);
    }

    // Hash literals (#t, #f, #\char)
    if (c == '#') {
        return read_hash_literal(lex);
    }

    // Symbols (including operators like +, -, *, etc.)
    if (is_symbol_start(c)) {
        return read_symbol(lex);
    }

    // Unknown character
    lexer_error(lex, "Unexpected character: %c", c);
}

// Symbol character rules (very permissive in Lisp)
int is_symbol_char(char c) {
    if (c == '\0' || is_whitespace(c)) return 0;
    if (c == '(' || c == ')') return 0;
    if (c == '"' || c == ';') return 0;
    if (c == '\'' || c == '`' || c == ',') return 0;
    return 1;
}

int is_symbol_start(char c) {
    return is_symbol_char(c) && c != '#';
}

Token read_symbol(Lexer *lex) {
    const char *start = lex->current;
    while (is_symbol_char(*lex->current)) {
        advance(lex);
    }
    int len = lex->current - start;
    char *text = strndup(start, len);
    return make_token(TOK_SYMBOL, text);
}

Token read_number(Lexer *lex) {
    const char *start = lex->current;

    if (*lex->current == '-') advance(lex);

    while (is_digit(*lex->current)) advance(lex);

    // Floating point
    if (*lex->current == '.' && is_digit(peek_next(lex))) {
        advance(lex);  // consume '.'
        while (is_digit(*lex->current)) advance(lex);
    }

    // Scientific notation
    if (*lex->current == 'e' || *lex->current == 'E') {
        advance(lex);
        if (*lex->current == '+' || *lex->current == '-') advance(lex);
        while (is_digit(*lex->current)) advance(lex);
    }

    char *text = strndup(start, lex->current - start);
    Token tok = make_token(TOK_NUMBER, text);
    tok.value.number = strtod(text, NULL);
    return tok;
}

void skip_whitespace_and_comments(Lexer *lex) {
    while (1) {
        // Skip whitespace
        while (is_whitespace(*lex->current)) {
            if (*lex->current == '\n') {
                lex->line++;
                lex->column = 1;
            }
            advance(lex);
        }

        // Skip line comments
        if (*lex->current == ';') {
            while (*lex->current && *lex->current != '\n') {
                advance(lex);
            }
            continue;
        }

        break;
    }
}
```

---

## Phase 2: Syntax Analysis with LALRGen

### The Complete Lisp Grammar

Here is a complete, production-ready Lisp grammar for LALRGen:

```yacc
%{
/* lisp_parser.y - Lisp/Scheme Grammar for LALRGen */
#include <stdio.h>
#include "lisp.h"

/* Forward declarations */
LispObject *make_cons(LispObject *car, LispObject *cdr);
LispObject *make_symbol(const char *name);
LispObject *make_number(double value);
LispObject *make_string(const char *str);
LispObject *make_nil(void);

/* The result of parsing */
LispObject *parse_result;
%}

/* Token declarations */
%token LPAREN RPAREN DOT
%token QUOTE QUASIQUOTE UNQUOTE UNQUOTE_SPLICE
%token SYMBOL NUMBER STRING BOOLEAN CHARACTER

/* Start symbol */
%start program

%%

/*
 * A program is a sequence of expressions (for a file)
 * or a single expression (for REPL)
 */
program
    : expression_list
        { parse_result = $1; }
    ;

expression_list
    : expression
        { $$ = make_cons($1, make_nil()); }
    | expression_list expression
        { $$ = append_list($1, make_cons($2, make_nil())); }
    ;

/*
 * An expression is either an atom or a compound form
 */
expression
    : atom
        { $$ = $1; }
    | list
        { $$ = $1; }
    | quoted_expression
        { $$ = $1; }
    ;

/*
 * Atoms: the indivisible elements
 */
atom
    : SYMBOL
        { $$ = make_symbol(yylval.text); }
    | NUMBER
        { $$ = make_number(yylval.number); }
    | STRING
        { $$ = make_string(yylval.text); }
    | BOOLEAN
        { $$ = make_boolean(yylval.boolean); }
    | CHARACTER
        { $$ = make_character(yylval.character); }
    ;

/*
 * Lists: the compound forms
 * Supports both proper lists and improper (dotted) lists
 */
list
    : LPAREN RPAREN
        { $$ = make_nil(); }
    | LPAREN list_contents RPAREN
        { $$ = $2; }
    | LPAREN list_contents DOT expression RPAREN
        { $$ = set_list_tail($2, $4); }
    ;

list_contents
    : expression
        { $$ = make_cons($1, make_nil()); }
    | list_contents expression
        { $$ = append_list($1, make_cons($2, make_nil())); }
    ;

/*
 * Quote syntax sugar
 * 'x      -> (quote x)
 * `x      -> (quasiquote x)
 * ,x      -> (unquote x)
 * ,@x     -> (unquote-splicing x)
 */
quoted_expression
    : QUOTE expression
        { $$ = make_cons(make_symbol("quote"),
                         make_cons($2, make_nil())); }
    | QUASIQUOTE expression
        { $$ = make_cons(make_symbol("quasiquote"),
                         make_cons($2, make_nil())); }
    | UNQUOTE expression
        { $$ = make_cons(make_symbol("unquote"),
                         make_cons($2, make_nil())); }
    | UNQUOTE_SPLICE expression
        { $$ = make_cons(make_symbol("unquote-splicing"),
                         make_cons($2, make_nil())); }
    ;

%%

/* Error handling */
void yyerror(const char *msg) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, msg);
}
```

### Generate Parser with LALRGen

```batch
LALRGen.exe lisp_parser.y
```

This produces `synout0.cpp` containing:
- Token symbol table
- LALR(1) action and goto tables
- Reduction handlers

### Parser Integration

```c
// parser.c
#include "synout0.cpp"  // LALRGen output

typedef struct {
    int state;
    LispObject *value;
} ParseStackEntry;

LispObject *parse(Lexer *lex) {
    ParseStack stack;
    stack_init(&stack);
    stack_push(&stack, 0, NULL);  // Initial state

    Token tok = lexer_next_token(lex);

    while (1) {
        int state = stack_top_state(&stack);
        int action = yy_action[state][token_to_index(tok.type)];

        if (action > 0) {
            // Shift: push state and token value
            stack_push(&stack, action, token_to_lisp_object(tok));
            tok = lexer_next_token(lex);
        }
        else if (action < 0) {
            // Reduce by production (-action)
            int prod = -action;
            int len = yy_production_length[prod];

            // Collect values for semantic action
            LispObject **values = alloca(len * sizeof(LispObject*));
            for (int i = len - 1; i >= 0; i--) {
                values[i] = stack_pop(&stack);
            }

            // Execute semantic action
            LispObject *result = execute_semantic_action(prod, values);

            // Goto
            int goto_state = yy_goto[stack_top_state(&stack)][yy_lhs[prod]];
            stack_push(&stack, goto_state, result);
        }
        else if (action == ACCEPT) {
            return stack_top_value(&stack);
        }
        else {
            parse_error("Unexpected token", tok);
            return NULL;
        }
    }
}
```

---

## Phase 3: Semantic Analysis

### Lisp Object Representation

```c
// lisp.h
typedef enum {
    LISP_NIL,
    LISP_SYMBOL,
    LISP_NUMBER,
    LISP_STRING,
    LISP_CONS,
    LISP_LAMBDA,
    LISP_PRIMITIVE,
    LISP_BOOLEAN,
    LISP_CHARACTER,
    LISP_VECTOR,
    LISP_MACRO
} LispType;

typedef struct LispObject {
    LispType type;
    union {
        // Symbol
        struct {
            char *name;
            unsigned int hash;
        } symbol;

        // Number
        double number;

        // String
        char *string;

        // Cons cell
        struct {
            struct LispObject *car;
            struct LispObject *cdr;
        } cons;

        // Lambda/closure
        struct {
            struct LispObject *params;    // Parameter list
            struct LispObject *body;      // Body expressions
            struct Environment *env;      // Captured environment
        } lambda;

        // Primitive function
        struct {
            const char *name;
            struct LispObject *(*func)(struct LispObject *args);
        } primitive;

        // Boolean
        int boolean;

        // Character
        char character;
    };
} LispObject;
```

### Environment and Scoping

```c
// environment.h
typedef struct Binding {
    LispObject *symbol;
    LispObject *value;
    struct Binding *next;
} Binding;

typedef struct Environment {
    Binding *bindings;
    struct Environment *parent;
} Environment;

// Create a new environment
Environment *env_create(Environment *parent) {
    Environment *env = malloc(sizeof(Environment));
    env->bindings = NULL;
    env->parent = parent;
    return env;
}

// Look up a variable
LispObject *env_lookup(Environment *env, LispObject *symbol) {
    for (Environment *e = env; e != NULL; e = e->parent) {
        for (Binding *b = e->bindings; b != NULL; b = b->next) {
            if (symbol_eq(b->symbol, symbol)) {
                return b->value;
            }
        }
    }
    return NULL;  // Unbound variable
}

// Define a new variable in current scope
void env_define(Environment *env, LispObject *symbol, LispObject *value) {
    Binding *b = malloc(sizeof(Binding));
    b->symbol = symbol;
    b->value = value;
    b->next = env->bindings;
    env->bindings = b;
}

// Set an existing variable (searches parent scopes)
int env_set(Environment *env, LispObject *symbol, LispObject *value) {
    for (Environment *e = env; e != NULL; e = e->parent) {
        for (Binding *b = e->bindings; b != NULL; b = b->next) {
            if (symbol_eq(b->symbol, symbol)) {
                b->value = value;
                return 1;
            }
        }
    }
    return 0;  // Variable not found
}
```

### Macro Expansion

```c
// macros.c

// Expand all macros in an expression
LispObject *expand_macros(LispObject *expr, Environment *env) {
    if (expr->type != LISP_CONS) {
        return expr;  // Atoms don't expand
    }

    LispObject *head = car(expr);

    // Check if head is a macro
    if (head->type == LISP_SYMBOL) {
        LispObject *value = env_lookup(env, head);
        if (value && value->type == LISP_MACRO) {
            // Apply macro transformer
            LispObject *expanded = apply_macro(value, cdr(expr));
            // Recursively expand the result
            return expand_macros(expanded, env);
        }
    }

    // Not a macro call - recursively expand subforms
    // But skip special forms that don't evaluate all arguments
    if (is_symbol(head, "quote")) {
        return expr;  // Don't expand inside quote
    }
    if (is_symbol(head, "quasiquote")) {
        return expand_quasiquote(expr, env);
    }

    // Expand all subforms
    return cons(expand_macros(head, env),
                expand_list(cdr(expr), env));
}

// Define a macro
LispObject *define_macro(LispObject *name, LispObject *params,
                         LispObject *body, Environment *env) {
    LispObject *macro = make_macro(params, body, env);
    env_define(env, name, macro);
    return name;
}
```

### Free Variable Analysis

For compilation, we need to identify free variables in lambda expressions:

```c
// analysis.c

typedef struct VarSet {
    LispObject **vars;
    int count;
    int capacity;
} VarSet;

// Find free variables in an expression
VarSet *find_free_variables(LispObject *expr, VarSet *bound) {
    VarSet *free = varset_create();

    switch (expr->type) {
        case LISP_SYMBOL:
            if (!varset_contains(bound, expr)) {
                varset_add(free, expr);
            }
            break;

        case LISP_CONS: {
            LispObject *head = car(expr);

            if (is_symbol(head, "lambda")) {
                // (lambda (params...) body...)
                LispObject *params = cadr(expr);
                VarSet *new_bound = varset_union(bound, params_to_varset(params));
                LispObject *body = cddr(expr);
                while (body->type == LISP_CONS) {
                    VarSet *body_free = find_free_variables(car(body), new_bound);
                    varset_merge(free, body_free);
                    body = cdr(body);
                }
            }
            else if (is_symbol(head, "let")) {
                // (let ((var val) ...) body...)
                // Handle let bindings
                analyze_let(expr, bound, free);
            }
            else if (is_symbol(head, "quote")) {
                // No free variables in quoted expressions
            }
            else {
                // Regular function application
                LispObject *form = expr;
                while (form->type == LISP_CONS) {
                    VarSet *sub_free = find_free_variables(car(form), bound);
                    varset_merge(free, sub_free);
                    form = cdr(form);
                }
            }
            break;
        }

        default:
            // Numbers, strings, etc. have no free variables
            break;
    }

    return free;
}
```

---

## Phase 4: Evaluation Strategies

### Tree-Walking Interpreter

The simplest approach - directly evaluate the AST:

```c
// eval.c

LispObject *eval(LispObject *expr, Environment *env) {
    switch (expr->type) {
        case LISP_NIL:
        case LISP_NUMBER:
        case LISP_STRING:
        case LISP_BOOLEAN:
        case LISP_CHARACTER:
            // Self-evaluating
            return expr;

        case LISP_SYMBOL:
            // Variable lookup
            LispObject *value = env_lookup(env, expr);
            if (!value) {
                error("Unbound variable: %s", expr->symbol.name);
            }
            return value;

        case LISP_CONS:
            return eval_list(expr, env);

        default:
            error("Cannot evaluate: %s", lisp_type_name(expr->type));
    }
}

LispObject *eval_list(LispObject *expr, Environment *env) {
    LispObject *head = car(expr);
    LispObject *args = cdr(expr);

    // Special forms
    if (head->type == LISP_SYMBOL) {
        const char *name = head->symbol.name;

        if (strcmp(name, "quote") == 0) {
            return car(args);
        }

        if (strcmp(name, "if") == 0) {
            LispObject *cond = eval(car(args), env);
            if (is_true(cond)) {
                return eval(cadr(args), env);
            } else if (cddr(args)->type != LISP_NIL) {
                return eval(caddr(args), env);
            }
            return make_nil();
        }

        if (strcmp(name, "define") == 0) {
            LispObject *var = car(args);
            if (var->type == LISP_CONS) {
                // (define (name params...) body...)
                // Sugar for (define name (lambda (params...) body...))
                LispObject *name = car(var);
                LispObject *params = cdr(var);
                LispObject *body = cdr(args);
                LispObject *lambda = make_lambda(params, body, env);
                env_define(env, name, lambda);
                return name;
            } else {
                // (define var value)
                LispObject *value = eval(cadr(args), env);
                env_define(env, var, value);
                return var;
            }
        }

        if (strcmp(name, "set!") == 0) {
            LispObject *var = car(args);
            LispObject *value = eval(cadr(args), env);
            if (!env_set(env, var, value)) {
                error("Cannot set undefined variable: %s", var->symbol.name);
            }
            return value;
        }

        if (strcmp(name, "lambda") == 0) {
            LispObject *params = car(args);
            LispObject *body = cdr(args);
            return make_lambda(params, body, env);
        }

        if (strcmp(name, "begin") == 0) {
            LispObject *result = make_nil();
            while (args->type == LISP_CONS) {
                result = eval(car(args), env);
                args = cdr(args);
            }
            return result;
        }

        if (strcmp(name, "let") == 0) {
            return eval_let(args, env);
        }

        if (strcmp(name, "cond") == 0) {
            return eval_cond(args, env);
        }

        if (strcmp(name, "and") == 0) {
            return eval_and(args, env);
        }

        if (strcmp(name, "or") == 0) {
            return eval_or(args, env);
        }
    }

    // Function application
    LispObject *func = eval(head, env);
    LispObject *evaluated_args = eval_args(args, env);
    return apply(func, evaluated_args);
}

LispObject *apply(LispObject *func, LispObject *args) {
    switch (func->type) {
        case LISP_PRIMITIVE:
            return func->primitive.func(args);

        case LISP_LAMBDA: {
            // Create new environment extending the closure's environment
            Environment *call_env = env_create(func->lambda.env);

            // Bind parameters to arguments
            LispObject *params = func->lambda.params;
            LispObject *vals = args;

            while (params->type == LISP_CONS && vals->type == LISP_CONS) {
                env_define(call_env, car(params), car(vals));
                params = cdr(params);
                vals = cdr(vals);
            }

            // Handle rest parameter (dotted list)
            if (params->type == LISP_SYMBOL) {
                env_define(call_env, params, vals);
            }

            // Evaluate body
            LispObject *body = func->lambda.body;
            LispObject *result = make_nil();
            while (body->type == LISP_CONS) {
                result = eval(car(body), call_env);
                body = cdr(body);
            }
            return result;
        }

        default:
            error("Not a function: %s", lisp_type_name(func->type));
    }
}
```

### Tail Call Optimization

Proper tail call optimization is essential for Lisp:

```c
// eval_tco.c - Evaluation with tail call optimization

// Instead of returning a value, return either a value or a "trampoline"
typedef struct {
    int is_call;
    union {
        LispObject *value;
        struct {
            LispObject *func;
            LispObject *args;
        } call;
    };
} EvalResult;

LispObject *eval_tco(LispObject *expr, Environment *env) {
    while (1) {
        EvalResult result = eval_step(expr, env);

        if (!result.is_call) {
            return result.value;
        }

        // Tail call - continue the loop instead of recursing
        LispObject *func = result.call.func;
        LispObject *args = result.call.args;

        if (func->type == LISP_LAMBDA) {
            // Set up new environment and continue
            env = env_create(func->lambda.env);
            bind_parameters(env, func->lambda.params, args);
            expr = cons(make_symbol("begin"), func->lambda.body);
        }
        else if (func->type == LISP_PRIMITIVE) {
            return func->primitive.func(args);
        }
        else {
            error("Not a function");
        }
    }
}

EvalResult eval_step(LispObject *expr, Environment *env) {
    // ... similar to eval() but returns EvalResult
    // In tail position, return is_call=true instead of calling apply
}
```

---

## Phase 5: Compilation to Native Code

### Compilation Strategy

For a compiled Lisp, we generate native code for performance-critical paths
while keeping the runtime for dynamic features:

```
┌─────────────────────────────────────────────────────────────────┐
│                    Lisp Source Code                             │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Parse + Macro Expand + Analyze                                 │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  CPS Transformation (Continuation-Passing Style)                │
│  - Makes control flow explicit                                  │
│  - Enables tail call optimization                               │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Closure Conversion                                             │
│  - Convert free variables to explicit closure access            │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Code Generation (x86-64)                                       │
│  - Emit machine code for compiled functions                     │
│  - Link with runtime library                                    │
└─────────────────────────────────────────────────────────────────┘
```

### Intermediate Representation

```c
// ir.h - Intermediate representation for compilation

typedef enum {
    IR_CONST,       // Load constant
    IR_VAR,         // Load variable
    IR_SET,         // Set variable
    IR_CLOSURE,     // Create closure
    IR_CALL,        // Function call
    IR_TAIL_CALL,   // Tail call
    IR_IF,          // Conditional
    IR_SEQ,         // Sequence
    IR_PRIM         // Primitive operation
} IRType;

typedef struct IRNode {
    IRType type;
    union {
        LispObject *constant;

        struct {
            int index;      // Variable index
            int is_free;    // Free variable (from closure)?
        } var;

        struct {
            struct IRNode *func;
            struct IRNode **args;
            int argc;
        } call;

        struct {
            struct IRNode *cond;
            struct IRNode *then_branch;
            struct IRNode *else_branch;
        } if_node;

        struct {
            LispObject *free_vars;  // List of free variables
            struct IRNode *body;
        } closure;
    };
} IRNode;
```

### x86-64 Code Generation

```c
// codegen.c - x86-64 code generation

typedef struct {
    uint8_t *code;
    int size;
    int capacity;
} CodeBuffer;

void emit_byte(CodeBuffer *buf, uint8_t byte) {
    if (buf->size >= buf->capacity) {
        buf->capacity *= 2;
        buf->code = realloc(buf->code, buf->capacity);
    }
    buf->code[buf->size++] = byte;
}

// Generate code for an IR node
void generate(IRNode *node, CodeBuffer *buf, CompileContext *ctx) {
    switch (node->type) {
        case IR_CONST:
            // mov rax, <immediate pointer>
            emit_byte(buf, 0x48);  // REX.W
            emit_byte(buf, 0xB8);  // mov rax, imm64
            emit_qword(buf, (uint64_t)node->constant);
            break;

        case IR_VAR:
            if (node->var.is_free) {
                // Load from closure
                // mov rax, [rbx + offset]
                int offset = node->var.index * 8 + 8;  // Skip closure header
                emit_byte(buf, 0x48);
                emit_byte(buf, 0x8B);
                emit_byte(buf, 0x43);
                emit_byte(buf, offset);
            } else {
                // Load from stack
                int offset = node->var.index * 8;
                emit_byte(buf, 0x48);
                emit_byte(buf, 0x8B);
                emit_byte(buf, 0x45);
                emit_byte(buf, -offset - 8);
            }
            break;

        case IR_CALL:
            // Evaluate function and arguments
            generate(node->call.func, buf, ctx);
            emit_byte(buf, 0x50);  // push rax (save function)

            for (int i = 0; i < node->call.argc; i++) {
                generate(node->call.args[i], buf, ctx);
                emit_byte(buf, 0x50);  // push rax
            }

            // Pop arguments into registers (System V ABI)
            // rdi, rsi, rdx, rcx, r8, r9 for first 6 args
            for (int i = node->call.argc - 1; i >= 0; i--) {
                emit_pop_to_arg_reg(buf, i);
            }

            // Pop function into rax
            emit_byte(buf, 0x58);  // pop rax

            // Call through function pointer
            // call [rax + code_offset]
            emit_byte(buf, 0xFF);
            emit_byte(buf, 0x50);
            emit_byte(buf, CLOSURE_CODE_OFFSET);
            break;

        case IR_TAIL_CALL:
            // Similar to call but jump instead
            // Restore stack frame first
            generate_tail_call(node, buf, ctx);
            break;

        case IR_IF:
            // Generate condition
            generate(node->if_node.cond, buf, ctx);

            // cmp rax, #f (false)
            emit_byte(buf, 0x48);
            emit_byte(buf, 0x3D);
            emit_dword(buf, (uint32_t)(uint64_t)LISP_FALSE);

            // je else_branch
            emit_byte(buf, 0x74);
            int else_offset_pos = buf->size;
            emit_byte(buf, 0);  // Placeholder

            // Then branch
            generate(node->if_node.then_branch, buf, ctx);

            // jmp end
            emit_byte(buf, 0xEB);
            int end_offset_pos = buf->size;
            emit_byte(buf, 0);  // Placeholder

            // Else branch
            buf->code[else_offset_pos] = buf->size - else_offset_pos - 1;
            generate(node->if_node.else_branch, buf, ctx);

            // End
            buf->code[end_offset_pos] = buf->size - end_offset_pos - 1;
            break;

        case IR_CLOSURE:
            // Allocate closure object
            // Store free variables
            // Store code pointer
            generate_closure(node, buf, ctx);
            break;
    }
}
```

---

## Complete Build Workflow

### Directory Structure

```
lisp_compiler/
├── src/
│   ├── main.c           # Driver (REPL and file execution)
│   ├── lexer.c          # Lexical analyzer
│   ├── lexer.h
│   ├── lisp_grammar.y   # LALRGen grammar
│   ├── synout0.cpp      # Generated parser tables
│   ├── parser.c         # Parser driver
│   ├── parser.h
│   ├── lisp.c           # Object representation
│   ├── lisp.h
│   ├── env.c            # Environment/scoping
│   ├── env.h
│   ├── eval.c           # Interpreter
│   ├── eval.h
│   ├── primitives.c     # Built-in functions
│   ├── primitives.h
│   ├── macros.c         # Macro expansion
│   ├── macros.h
│   ├── gc.c             # Garbage collector
│   ├── gc.h
│   ├── codegen.c        # Native code generation (optional)
│   └── codegen.h
├── lib/
│   └── stdlib.scm       # Standard library
├── test/
│   └── *.scm            # Test programs
└── CMakeLists.txt
```

### Build Steps

```batch
REM 1. Generate parser tables from grammar
LALRGen.exe src/lisp_grammar.y
move synout0.cpp src/

REM 2. Build the interpreter/compiler
cmake -B build -S .
cmake --build build --config Release

REM 3. Run REPL
build\Release\lisp.exe

REM 4. Execute a file
build\Release\lisp.exe test/factorial.scm
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(LispCompiler C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(lisp
    src/main.c
    src/lexer.c
    src/parser.c
    src/synout0.cpp
    src/lisp.c
    src/env.c
    src/eval.c
    src/primitives.c
    src/macros.c
    src/gc.c
)

# Optional: add codegen.c for native compilation
# target_sources(lisp PRIVATE src/codegen.c)

target_include_directories(lisp PRIVATE src)
```

---

## Example Implementation

### Minimal REPL

```c
// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "primitives.h"
#include "gc.h"

void repl(Environment *env) {
    char line[4096];

    printf("Lisp REPL (type 'quit' to exit)\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        if (strcmp(line, "quit\n") == 0) {
            break;
        }

        // Parse
        Lexer lexer;
        lexer_init(&lexer, line);
        LispObject *expr = parse(&lexer);

        if (!expr) {
            printf("Parse error\n");
            continue;
        }

        // Evaluate
        LispObject *result = eval(expr, env);

        // Print
        print_lisp_object(result);
        printf("\n");

        // Garbage collect periodically
        gc_maybe_collect();
    }
}

int main(int argc, char **argv) {
    // Initialize GC
    gc_init();

    // Create global environment with primitives
    Environment *global = env_create(NULL);
    register_primitives(global);

    if (argc > 1) {
        // Execute file
        char *source = read_file(argv[1]);
        if (!source) {
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }

        Lexer lexer;
        lexer_init(&lexer, source);
        LispObject *program = parse(&lexer);

        if (program) {
            // Execute each expression
            while (program->type == LISP_CONS) {
                eval(car(program), global);
                program = cdr(program);
            }
        }

        free(source);
    } else {
        // Run REPL
        repl(global);
    }

    gc_shutdown();
    return 0;
}
```

### Primitive Functions

```c
// primitives.c
#include "primitives.h"
#include "lisp.h"

// (car pair)
LispObject *prim_car(LispObject *args) {
    require_args(args, 1);
    LispObject *pair = car(args);
    require_type(pair, LISP_CONS);
    return car(pair);
}

// (cdr pair)
LispObject *prim_cdr(LispObject *args) {
    require_args(args, 1);
    LispObject *pair = car(args);
    require_type(pair, LISP_CONS);
    return cdr(pair);
}

// (cons a b)
LispObject *prim_cons(LispObject *args) {
    require_args(args, 2);
    return make_cons(car(args), cadr(args));
}

// (+ a b ...)
LispObject *prim_add(LispObject *args) {
    double sum = 0;
    while (args->type == LISP_CONS) {
        LispObject *n = car(args);
        require_type(n, LISP_NUMBER);
        sum += n->number;
        args = cdr(args);
    }
    return make_number(sum);
}

// (- a b)
LispObject *prim_sub(LispObject *args) {
    require_args_min(args, 1);
    LispObject *first = car(args);
    require_type(first, LISP_NUMBER);

    if (cdr(args)->type == LISP_NIL) {
        // Unary minus
        return make_number(-first->number);
    }

    double result = first->number;
    args = cdr(args);
    while (args->type == LISP_CONS) {
        LispObject *n = car(args);
        require_type(n, LISP_NUMBER);
        result -= n->number;
        args = cdr(args);
    }
    return make_number(result);
}

// (* a b ...)
LispObject *prim_mul(LispObject *args) {
    double product = 1;
    while (args->type == LISP_CONS) {
        LispObject *n = car(args);
        require_type(n, LISP_NUMBER);
        product *= n->number;
        args = cdr(args);
    }
    return make_number(product);
}

// (/ a b)
LispObject *prim_div(LispObject *args) {
    require_args(args, 2);
    LispObject *a = car(args);
    LispObject *b = cadr(args);
    require_type(a, LISP_NUMBER);
    require_type(b, LISP_NUMBER);
    if (b->number == 0) {
        error("Division by zero");
    }
    return make_number(a->number / b->number);
}

// (= a b)
LispObject *prim_eq_num(LispObject *args) {
    require_args(args, 2);
    LispObject *a = car(args);
    LispObject *b = cadr(args);
    require_type(a, LISP_NUMBER);
    require_type(b, LISP_NUMBER);
    return make_boolean(a->number == b->number);
}

// (< a b)
LispObject *prim_lt(LispObject *args) {
    require_args(args, 2);
    LispObject *a = car(args);
    LispObject *b = cadr(args);
    require_type(a, LISP_NUMBER);
    require_type(b, LISP_NUMBER);
    return make_boolean(a->number < b->number);
}

// (eq? a b)
LispObject *prim_eq(LispObject *args) {
    require_args(args, 2);
    LispObject *a = car(args);
    LispObject *b = cadr(args);
    return make_boolean(a == b);  // Pointer equality
}

// (equal? a b)
LispObject *prim_equal(LispObject *args) {
    require_args(args, 2);
    return make_boolean(lisp_equal(car(args), cadr(args)));
}

// (null? x)
LispObject *prim_null(LispObject *args) {
    require_args(args, 1);
    return make_boolean(car(args)->type == LISP_NIL);
}

// (pair? x)
LispObject *prim_pair(LispObject *args) {
    require_args(args, 1);
    return make_boolean(car(args)->type == LISP_CONS);
}

// (list a b c ...)
LispObject *prim_list(LispObject *args) {
    return args;  // Args are already a list
}

// (display x)
LispObject *prim_display(LispObject *args) {
    require_args(args, 1);
    print_lisp_object_no_quotes(car(args));
    return make_nil();
}

// (newline)
LispObject *prim_newline(LispObject *args) {
    printf("\n");
    return make_nil();
}

// Register all primitives
void register_primitives(Environment *env) {
    struct { const char *name; LispPrimitive func; } prims[] = {
        // List operations
        {"car", prim_car},
        {"cdr", prim_cdr},
        {"cons", prim_cons},
        {"list", prim_list},
        {"null?", prim_null},
        {"pair?", prim_pair},

        // Arithmetic
        {"+", prim_add},
        {"-", prim_sub},
        {"*", prim_mul},
        {"/", prim_div},

        // Comparison
        {"=", prim_eq_num},
        {"<", prim_lt},
        {">", prim_gt},
        {"<=", prim_le},
        {">=", prim_ge},
        {"eq?", prim_eq},
        {"equal?", prim_equal},

        // I/O
        {"display", prim_display},
        {"newline", prim_newline},

        // ... more primitives
        {NULL, NULL}
    };

    for (int i = 0; prims[i].name; i++) {
        LispObject *prim = make_primitive(prims[i].name, prims[i].func);
        env_define(env, make_symbol(prims[i].name), prim);
    }

    // Also define constants
    env_define(env, make_symbol("#t"), make_boolean(1));
    env_define(env, make_symbol("#f"), make_boolean(0));
}
```

### Test Program: Factorial

```scheme
; factorial.scm
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(display "10! = ")
(display (factorial 10))
(newline)

; Tail-recursive version
(define (factorial-tail n acc)
  (if (= n 0)
      acc
      (factorial-tail (- n 1) (* n acc))))

(define (fact n)
  (factorial-tail n 1))

(display "20! = ")
(display (fact 20))
(newline)
```

---

## Extending the Language

### Adding Macros

Macros are a powerful feature of Lisp. Here's how to implement `defmacro`:

```c
// In eval.c, add handling for defmacro
if (strcmp(name, "defmacro") == 0) {
    // (defmacro name (params...) body...)
    LispObject *macro_name = car(args);
    LispObject *params = cadr(args);
    LispObject *body = cddr(args);

    LispObject *macro = make_macro(params, body, env);
    env_define(env, macro_name, macro);
    return macro_name;
}
```

### Example Macros

```scheme
; Standard macros implemented in Lisp

(defmacro when (test . body)
  `(if ,test (begin ,@body)))

(defmacro unless (test . body)
  `(if (not ,test) (begin ,@body)))

(defmacro let* (bindings . body)
  (if (null? bindings)
      `(begin ,@body)
      `(let (,(car bindings))
         (let* ,(cdr bindings) ,@body))))

(defmacro dotimes (var-count . body)
  (let ((var (car var-count))
        (count (cadr var-count)))
    `(let loop ((,var 0))
       (when (< ,var ,count)
         ,@body
         (loop (+ ,var 1))))))
```

### Adding Continuations

For full Scheme compliance, implement `call/cc`:

```c
// continuations.c

typedef struct Continuation {
    LispType type;  // LISP_CONTINUATION
    jmp_buf jump;
    LispObject *value;
} Continuation;

LispObject *prim_call_cc(LispObject *args) {
    require_args(args, 1);
    LispObject *proc = car(args);
    require_type(proc, LISP_LAMBDA);

    Continuation *cont = make_continuation();

    if (setjmp(cont->jump) == 0) {
        // First time: call the procedure with the continuation
        LispObject *cont_obj = (LispObject *)cont;
        return apply(proc, make_cons(cont_obj, make_nil()));
    } else {
        // Returned via continuation
        return cont->value;
    }
}

// When a continuation is called as a function:
LispObject *apply_continuation(Continuation *cont, LispObject *args) {
    require_args(args, 1);
    cont->value = car(args);
    longjmp(cont->jump, 1);
}
```

---

## Further Reading

1. **"Structure and Interpretation of Computer Programs"** by Abelson & Sussman
   - Classic Lisp/Scheme textbook
   - Includes metacircular interpreter implementation

2. **"Lisp in Small Pieces"** by Christian Queinnec
   - Comprehensive coverage of Lisp implementation
   - Multiple interpreters and compilers

3. **"Compiling with Continuations"** by Andrew Appel
   - CPS-based compilation techniques
   - Relevant for advanced optimization

4. **R5RS / R7RS Scheme Standards**
   - Official Scheme language specifications
   - Reference for standard library

5. **"Compiler Design in C"** by Allen Holub
   - Original source of LALRGen
   - Compiler implementation examples

---

## Appendix: Quick Reference

### Lisp Grammar Productions (LALRGen)

| Production | Description |
|------------|-------------|
| `program → expression_list` | Sequence of expressions |
| `expression → atom \| list \| quoted` | Any expression |
| `atom → SYMBOL \| NUMBER \| STRING \| ...` | Atomic values |
| `list → '(' expressions ')' \| '(' exprs '.' expr ')'` | Lists |
| `quoted → '\'' expr \| '\`' expr \| ',' expr` | Quote forms |

### Core Special Forms

| Form | Syntax | Description |
|------|--------|-------------|
| `quote` | `(quote x)` or `'x` | Return unevaluated |
| `if` | `(if test then else)` | Conditional |
| `define` | `(define var val)` | Define variable |
| `set!` | `(set! var val)` | Mutate variable |
| `lambda` | `(lambda (args) body)` | Create function |
| `begin` | `(begin e1 e2 ...)` | Sequence |
| `let` | `(let ((v e) ...) body)` | Local binding |

### Essential Primitives

| Category | Functions |
|----------|-----------|
| **List** | `car`, `cdr`, `cons`, `list`, `null?`, `pair?` |
| **Arithmetic** | `+`, `-`, `*`, `/`, `mod`, `abs` |
| **Comparison** | `=`, `<`, `>`, `<=`, `>=`, `eq?`, `equal?` |
| **Type** | `number?`, `symbol?`, `string?`, `procedure?` |
| **I/O** | `display`, `newline`, `read`, `write` |
