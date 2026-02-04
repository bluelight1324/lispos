/*
 * codegen.c - MASM x64 Code Generator Implementation
 *
 * Generates x64 MASM assembly from Lisp expressions.
 * Uses Windows x64 calling convention (RCX, RDX, R8, R9 for first 4 args).
 *
 * Memory layout for Lisp objects:
 *   [type:8][gc:8][padding:16][data:varies]
 *
 * Result values are returned in RAX as pointers to Lisp objects.
 */

#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Helper to emit a line of assembly */
static void emit(CodegenContext *ctx, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(ctx->output, format, args);
    fprintf(ctx->output, "\n");
    va_end(args);
}

/* Generate a unique label */
static char *gen_label(CodegenContext *ctx, const char *prefix) {
    char *label = (char *)malloc(64);
    snprintf(label, 64, "%s_%d", prefix, ctx->label_counter++);
    return label;
}

/* Add a string literal to the table */
static char *add_string_literal(CodegenContext *ctx, const char *text) {
    /* Check if already exists */
    for (struct StringLiteral *s = ctx->strings; s; s = s->next) {
        if (strcmp(s->text, text) == 0) {
            return s->label;
        }
    }

    /* Add new string */
    struct StringLiteral *s = (struct StringLiteral *)malloc(sizeof(struct StringLiteral));
    s->text = strdup(text);
    s->label = (char *)malloc(32);
    snprintf(s->label, 32, "str_%d", ctx->string_counter++);
    s->next = ctx->strings;
    ctx->strings = s;
    return s->label;
}

/* Add a symbol reference to the table */
static void add_symbol_ref(CodegenContext *ctx, const char *name) {
    /* Check if already exists */
    for (struct SymbolRef *s = ctx->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return;  /* Already tracked */
        }
    }

    /* Add new symbol */
    struct SymbolRef *s = (struct SymbolRef *)malloc(sizeof(struct SymbolRef));
    s->name = strdup(name);
    s->next = ctx->symbols;
    ctx->symbols = s;
}

/* Initialize code generator */
void codegen_init(CodegenContext *ctx, FILE *output) {
    ctx->output = output;
    ctx->label_counter = 0;
    ctx->string_counter = 0;
    ctx->lambda_counter = 0;
    ctx->env = NULL;
    ctx->strings = NULL;
    ctx->symbols = NULL;
    ctx->lambdas = NULL;
}

/* Free code generator resources */
void codegen_free(CodegenContext *ctx) {
    /* Free string literals */
    struct StringLiteral *s = ctx->strings;
    while (s) {
        struct StringLiteral *next = s->next;
        free(s->text);
        free(s->label);
        free(s);
        s = next;
    }

    /* Free symbol references */
    struct SymbolRef *sym = ctx->symbols;
    while (sym) {
        struct SymbolRef *next = sym->next;
        free(sym->name);
        free(sym);
        sym = next;
    }

    /* Free lambda entries */
    struct LambdaEntry *l = ctx->lambdas;
    while (l) {
        struct LambdaEntry *next = l->next;
        free(l->label);
        free(l);
        l = next;
    }
}

/* Forward declaration */
static void compile_expr(CodegenContext *ctx, LispObject *expr, int tail_position);

/* Compile a number literal */
static void compile_number(CodegenContext *ctx, double value) {
    /* Call runtime to allocate number object */
    emit(ctx, "        ; Load number %g", value);

    /* Check if it's a small integer that fits in a fixnum */
    long long int_val = (long long)value;
    if ((double)int_val == value && int_val >= -2147483647LL - 1 && int_val <= 2147483647LL) {
        emit(ctx, "        mov     rcx, %lld", int_val);
        emit(ctx, "        call    rt_make_fixnum");
    } else {
        /* For floating point, store in data section and load via SSE */
        char *lbl = gen_label(ctx, "flt");
        /* We'll need to add this to a float literal table */
        emit(ctx, "        lea     rcx, [%s]", lbl);
        emit(ctx, "        movsd   xmm0, qword ptr [rcx]");
        emit(ctx, "        call    rt_make_float");
        /* TODO: Actually add the float to data section */
    }
}

/* Compile a string literal */
static void compile_string_literal(CodegenContext *ctx, const char *text) {
    char *label = add_string_literal(ctx, text);
    emit(ctx, "        ; Load string \"%s\"", text);
    emit(ctx, "        lea     rcx, [%s]", label);
    emit(ctx, "        call    rt_make_string");
}

/* Compile a symbol reference */
static void compile_symbol_ref(CodegenContext *ctx, const char *name) {
    add_symbol_ref(ctx, name);
    emit(ctx, "        ; Look up symbol '%s'", name);
    emit(ctx, "        lea     rcx, [sym_%s]", name);
    emit(ctx, "        mov     rdx, [rbp-8]        ; Current environment");
    emit(ctx, "        call    rt_env_lookup");
    emit(ctx, "        test    rax, rax");
    emit(ctx, "        jz      error_unbound");
}

/* Compile a quoted expression */
static void compile_quote(CodegenContext *ctx, LispObject *expr) {
    if (is_nil(expr)) {
        emit(ctx, "        mov     rax, [rt_nil]");
    } else if (is_number(expr)) {
        compile_number(ctx, expr->number);
    } else if (is_string(expr)) {
        compile_string_literal(ctx, expr->string.data);
    } else if (is_symbol(expr)) {
        char *label = add_string_literal(ctx, expr->symbol.name);
        emit(ctx, "        lea     rcx, [%s]", label);
        emit(ctx, "        call    rt_make_symbol");
    } else if (is_cons(expr)) {
        /* Recursively build the list at runtime */
        compile_quote(ctx, cdr(expr));
        emit(ctx, "        push    rax");
        compile_quote(ctx, car(expr));
        emit(ctx, "        mov     rcx, rax");
        emit(ctx, "        pop     rdx");
        emit(ctx, "        call    rt_cons");
    } else if (is_boolean(expr)) {
        if (is_true(expr)) {
            emit(ctx, "        mov     rax, [rt_true]");
        } else {
            emit(ctx, "        mov     rax, [rt_false]");
        }
    }
}

/* Compile an if expression */
static void compile_if(CodegenContext *ctx, LispObject *args, int tail_position) {
    char *else_label = gen_label(ctx, "else");
    char *end_label = gen_label(ctx, "endif");

    /* Compile condition */
    compile_expr(ctx, car(args), 0);

    /* Test for false */
    emit(ctx, "        cmp     rax, [rt_false]");
    emit(ctx, "        je      %s", else_label);

    /* Then branch */
    compile_expr(ctx, cadr(args), tail_position);
    emit(ctx, "        jmp     %s", end_label);

    /* Else branch */
    emit(ctx, "%s:", else_label);
    if (is_cons(cddr(args))) {
        compile_expr(ctx, caddr(args), tail_position);
    } else {
        emit(ctx, "        mov     rax, [rt_nil]");
    }

    emit(ctx, "%s:", end_label);

    free(else_label);
    free(end_label);
}

/* Compile a begin expression */
static void compile_begin(CodegenContext *ctx, LispObject *exprs, int tail_position) {
    while (is_cons(exprs)) {
        int is_last = !is_cons(cdr(exprs));
        compile_expr(ctx, car(exprs), is_last && tail_position);
        exprs = cdr(exprs);
    }
}

/* Compile a define form */
static void compile_define(CodegenContext *ctx, LispObject *args) {
    LispObject *first = car(args);

    if (is_cons(first)) {
        /* (define (name params...) body...) */
        LispObject *name = car(first);
        emit(ctx, "        ; Define function %s", name->symbol.name);

        /* Compile as lambda and bind */
        LispObject *params = cdr(first);
        LispObject *body = cdr(args);
        LispObject *lambda = make_cons(make_symbol("lambda"),
                                       make_cons(params, body));
        compile_expr(ctx, lambda, 0);

        /* Store in environment */
        add_symbol_ref(ctx, name->symbol.name);
        emit(ctx, "        mov     rdx, rax");
        emit(ctx, "        lea     rcx, [sym_%s]", name->symbol.name);
        emit(ctx, "        mov     r8, [rbp-8]");
        emit(ctx, "        call    rt_env_define");
    } else {
        /* (define var value) */
        add_symbol_ref(ctx, first->symbol.name);
        emit(ctx, "        ; Define variable %s", first->symbol.name);
        compile_expr(ctx, cadr(args), 0);
        emit(ctx, "        mov     rdx, rax");
        emit(ctx, "        lea     rcx, [sym_%s]", first->symbol.name);
        emit(ctx, "        mov     r8, [rbp-8]");
        emit(ctx, "        call    rt_env_define");
    }
}

/* Compile a lambda expression */
static void compile_lambda(CodegenContext *ctx, LispObject *args) {
    char *lambda_label = gen_label(ctx, "lambda");
    char *after_label = gen_label(ctx, "after_lambda");

    emit(ctx, "        ; Create lambda");
    emit(ctx, "        jmp     %s", after_label);

    /* Lambda body */
    emit(ctx, "%s:", lambda_label);
    emit(ctx, "        push    rbp");
    emit(ctx, "        mov     rbp, rsp");
    emit(ctx, "        sub     rsp, 64         ; Local space");
    emit(ctx, "        mov     [rbp-8], rcx    ; Save closure environment");

    /* Bind parameters to arguments */
    LispObject *params = car(args);
    int param_idx = 0;
    while (is_cons(params)) {
        const char *param_name = car(params)->symbol.name;
        /* Arguments come in as a list in RDX */
        emit(ctx, "        ; Bind parameter %s", param_name);
        if (param_idx == 0) {
            emit(ctx, "        mov     rsi, rdx        ; Args list");
        }
        emit(ctx, "        mov     rdi, [rsi]          ; car of args");
        emit(ctx, "        mov     [rbp-%d], rdi       ; Save to local", 16 + param_idx * 8);
        emit(ctx, "        mov     rsi, [rsi+8]        ; cdr of args");
        params = cdr(params);
        param_idx++;
    }

    /* Compile body */
    compile_begin(ctx, cdr(args), 1);

    emit(ctx, "        mov     rsp, rbp");
    emit(ctx, "        pop     rbp");
    emit(ctx, "        ret");

    /* After lambda - create closure object */
    emit(ctx, "%s:", after_label);
    emit(ctx, "        lea     rcx, [%s]", lambda_label);
    emit(ctx, "        mov     rdx, [rbp-8]    ; Current environment");
    emit(ctx, "        call    rt_make_closure");

    free(lambda_label);
    free(after_label);
}

/* Compile a function call */
static void compile_call(CodegenContext *ctx, LispObject *expr, int tail_position) {
    LispObject *func_expr = car(expr);
    LispObject *args = cdr(expr);

    /* Build argument list on stack */
    int argc = list_length(args);
    emit(ctx, "        ; Function call with %d args", argc);

    /* Evaluate and push arguments in reverse order */
    if (argc > 0) {
        emit(ctx, "        mov     rax, [rt_nil]");
        emit(ctx, "        push    rax             ; Start with nil");

        LispObject *arg_list = list_reverse(args);
        while (is_cons(arg_list)) {
            compile_expr(ctx, car(arg_list), 0);
            emit(ctx, "        mov     rcx, rax");
            emit(ctx, "        pop     rdx");
            emit(ctx, "        call    rt_cons");
            emit(ctx, "        push    rax");
            arg_list = cdr(arg_list);
        }
        emit(ctx, "        pop     rdx             ; Args list");
    } else {
        emit(ctx, "        mov     rdx, [rt_nil]   ; Empty args");
    }

    /* Evaluate function */
    emit(ctx, "        push    rdx             ; Save args");
    compile_expr(ctx, func_expr, 0);
    emit(ctx, "        mov     rcx, rax        ; Function");
    emit(ctx, "        pop     rdx             ; Args");

    /* Call the function */
    if (tail_position) {
        emit(ctx, "        ; Tail call");
        emit(ctx, "        mov     rsp, rbp");
        emit(ctx, "        pop     rbp");
        emit(ctx, "        jmp     rt_apply");
    } else {
        emit(ctx, "        call    rt_apply");
    }
}

/* Compile a single expression */
static void compile_expr(CodegenContext *ctx, LispObject *expr, int tail_position) {
    if (!expr || is_nil(expr)) {
        emit(ctx, "        mov     rax, [rt_nil]");
        return;
    }

    switch (expr->type) {
        case LISP_NUMBER:
            compile_number(ctx, expr->number);
            break;

        case LISP_STRING:
            compile_string_literal(ctx, expr->string.data);
            break;

        case LISP_BOOLEAN:
            if (is_true(expr)) {
                emit(ctx, "        mov     rax, [rt_true]");
            } else {
                emit(ctx, "        mov     rax, [rt_false]");
            }
            break;

        case LISP_SYMBOL:
            compile_symbol_ref(ctx, expr->symbol.name);
            break;

        case LISP_CONS: {
            LispObject *head = car(expr);
            LispObject *args = cdr(expr);

            if (is_symbol(head)) {
                const char *name = head->symbol.name;

                if (strcmp(name, "quote") == 0) {
                    compile_quote(ctx, car(args));
                } else if (strcmp(name, "if") == 0) {
                    compile_if(ctx, args, tail_position);
                } else if (strcmp(name, "begin") == 0) {
                    compile_begin(ctx, args, tail_position);
                } else if (strcmp(name, "define") == 0) {
                    compile_define(ctx, args);
                } else if (strcmp(name, "lambda") == 0) {
                    compile_lambda(ctx, args);
                } else if (strcmp(name, "set!") == 0) {
                    add_symbol_ref(ctx, car(args)->symbol.name);
                    compile_expr(ctx, cadr(args), 0);
                    emit(ctx, "        mov     rdx, rax");
                    emit(ctx, "        lea     rcx, [sym_%s]", car(args)->symbol.name);
                    emit(ctx, "        mov     r8, [rbp-8]");
                    emit(ctx, "        call    rt_env_set");
                } else {
                    /* Regular function call */
                    compile_call(ctx, expr, tail_position);
                }
            } else {
                /* Function in operator position */
                compile_call(ctx, expr, tail_position);
            }
            break;
        }

        default:
            emit(ctx, "        ; Unknown expression type %d", expr->type);
            emit(ctx, "        mov     rax, [rt_nil]");
            break;
    }
}

/* Generate the data section */
void codegen_data_section(CodegenContext *ctx) {
    emit(ctx, "");
    emit(ctx, "; =============================================================================");
    emit(ctx, "; Data Section");
    emit(ctx, "; =============================================================================");
    emit(ctx, ".data");
    emit(ctx, "");

    /* String literals */
    for (struct StringLiteral *s = ctx->strings; s; s = s->next) {
        emit(ctx, "%s    db      \"%s\", 0", s->label, s->text);
    }
    emit(ctx, "");

    /* Symbol name strings */
    emit(ctx, "; Symbol name strings");
    for (struct SymbolRef *sym = ctx->symbols; sym; sym = sym->next) {
        emit(ctx, "sym_%s    db      \"%s\", 0", sym->name, sym->name);
    }
    emit(ctx, "");
}

/* Generate the runtime support code */
void codegen_runtime(CodegenContext *ctx) {
    emit(ctx, "");
    emit(ctx, "; =============================================================================");
    emit(ctx, "; Runtime Support (External References)");
    emit(ctx, "; =============================================================================");
    emit(ctx, "");
    emit(ctx, "extern rt_nil:qword");
    emit(ctx, "extern rt_true:qword");
    emit(ctx, "extern rt_false:qword");
    emit(ctx, "extern rt_make_fixnum:proc");
    emit(ctx, "extern rt_make_float:proc");
    emit(ctx, "extern rt_make_string:proc");
    emit(ctx, "extern rt_make_symbol:proc");
    emit(ctx, "extern rt_make_closure:proc");
    emit(ctx, "extern rt_cons:proc");
    emit(ctx, "extern rt_car:proc");
    emit(ctx, "extern rt_cdr:proc");
    emit(ctx, "extern rt_apply:proc");
    emit(ctx, "extern rt_env_lookup:proc");
    emit(ctx, "extern rt_env_define:proc");
    emit(ctx, "extern rt_env_set:proc");
    emit(ctx, "extern rt_init:proc");
    emit(ctx, "extern rt_shutdown:proc");
    emit(ctx, "extern rt_print:proc");
    emit(ctx, "");
}

/* Compile a Lisp program */
void codegen_program(CodegenContext *ctx, LispObject *program) {
    /* File header */
    emit(ctx, "; =============================================================================");
    emit(ctx, "; Lisp Compiled Output - MASM x64");
    emit(ctx, "; Generated by LispCompiler");
    emit(ctx, "; =============================================================================");
    emit(ctx, "");
    emit(ctx, ".code");
    emit(ctx, "");

    /* Runtime references */
    codegen_runtime(ctx);

    /* Error handler */
    emit(ctx, "error_unbound:");
    emit(ctx, "        lea     rcx, [err_unbound_msg]");
    emit(ctx, "        call    rt_error");
    emit(ctx, "        mov     rax, [rt_nil]");
    emit(ctx, "        ret");
    emit(ctx, "");

    /* Main entry point */
    emit(ctx, "; =============================================================================");
    emit(ctx, "; Main Entry Point");
    emit(ctx, "; =============================================================================");
    emit(ctx, "main    proc");
    emit(ctx, "        push    rbp");
    emit(ctx, "        mov     rbp, rsp");
    emit(ctx, "        sub     rsp, 64");
    emit(ctx, "");
    emit(ctx, "        ; Initialize runtime");
    emit(ctx, "        call    rt_init");
    emit(ctx, "        mov     [rbp-8], rax    ; Global environment");
    emit(ctx, "");

    /* Compile each top-level expression */
    while (is_cons(program)) {
        emit(ctx, "        ; Top-level expression");
        compile_expr(ctx, car(program), 0);

        /* Print result */
        emit(ctx, "        mov     rcx, rax");
        emit(ctx, "        call    rt_print");
        emit(ctx, "");

        program = cdr(program);
    }

    emit(ctx, "        ; Shutdown runtime");
    emit(ctx, "        call    rt_shutdown");
    emit(ctx, "");
    emit(ctx, "        xor     eax, eax        ; Return 0");
    emit(ctx, "        mov     rsp, rbp");
    emit(ctx, "        pop     rbp");
    emit(ctx, "        ret");
    emit(ctx, "main    endp");
    emit(ctx, "");

    /* Data section */
    codegen_data_section(ctx);

    /* Error messages */
    emit(ctx, "err_unbound_msg db \"Error: Unbound variable\", 0");
    emit(ctx, "");
    emit(ctx, "end");
}

/* Compile a single expression */
void codegen_expr(CodegenContext *ctx, LispObject *expr) {
    compile_expr(ctx, expr, 0);
}

/* Compile a file */
int compile_file(const char *input_path, const char *output_path) {
    /* Read input file */
    FILE *input = fopen(input_path, "r");
    if (!input) {
        fprintf(stderr, "Cannot open input file: %s\n", input_path);
        return 1;
    }

    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    fseek(input, 0, SEEK_SET);

    char *source = (char *)malloc(size + 1);
    fread(source, 1, size, input);
    source[size] = '\0';
    fclose(input);

    int result = compile_string(source, output_path);
    free(source);
    return result;
}

/* Compile a string */
int compile_string(const char *source, const char *output_path) {
    /* Initialize Lisp system */
    lisp_init();

    /* Parse */
    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    LispObject *program = parse_program(&parser);

    if (parser_had_error(&parser)) {
        fprintf(stderr, "Parse error: %s\n", parser_error_message(&parser));
        lisp_shutdown();
        return 1;
    }

    /* Open output file */
    FILE *output = fopen(output_path, "w");
    if (!output) {
        fprintf(stderr, "Cannot open output file: %s\n", output_path);
        lisp_shutdown();
        return 1;
    }

    /* Generate code */
    CodegenContext ctx;
    codegen_init(&ctx, output);
    codegen_program(&ctx, program);
    codegen_free(&ctx);

    fclose(output);
    lisp_shutdown();

    return 0;
}
