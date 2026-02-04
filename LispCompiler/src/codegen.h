/*
 * codegen.h - MASM x64 Code Generator
 *
 * Compiles Lisp expressions to x64 MASM assembly.
 * The generated code uses a runtime library for Lisp operations.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "lisp.h"
#include "env.h"
#include <stdio.h>

/* Compilation context */
typedef struct {
    FILE *output;           /* Output file */
    int label_counter;      /* For generating unique labels */
    int string_counter;     /* For string literals */
    int lambda_counter;     /* For lambda functions */
    Environment *env;       /* Current environment (for variable tracking) */

    /* String literal table */
    struct StringLiteral {
        char *text;
        char *label;
        struct StringLiteral *next;
    } *strings;

    /* Symbol reference table (for symbols used in code) */
    struct SymbolRef {
        char *name;
        struct SymbolRef *next;
    } *symbols;

    /* Lambda function table */
    struct LambdaEntry {
        LispObject *lambda;
        char *label;
        struct LambdaEntry *next;
    } *lambdas;
} CodegenContext;

/* Initialize code generator */
void codegen_init(CodegenContext *ctx, FILE *output);

/* Free code generator resources */
void codegen_free(CodegenContext *ctx);

/* Compile a Lisp program to MASM */
void codegen_program(CodegenContext *ctx, LispObject *program);

/* Compile a single expression */
void codegen_expr(CodegenContext *ctx, LispObject *expr);

/* Generate the runtime support code */
void codegen_runtime(CodegenContext *ctx);

/* Generate the data section */
void codegen_data_section(CodegenContext *ctx);

/* Compile a file */
int compile_file(const char *input_path, const char *output_path);

/* Compile a string */
int compile_string(const char *source, const char *output_path);

#endif /* CODEGEN_H */
