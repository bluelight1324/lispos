# Building an ANSI C Compiler with LALRGen

A comprehensive guide to building a complete ANSI C compiler using the LALRGen
LALR(1) parser generator.

## Table of Contents

1. [Overview](#overview)
2. [Compiler Architecture](#compiler-architecture)
3. [Phase 1: Lexical Analysis](#phase-1-lexical-analysis)
4. [Phase 2: Syntax Analysis with LALRGen](#phase-2-syntax-analysis-with-lalrgen)
5. [Phase 3: Semantic Analysis](#phase-3-semantic-analysis)
6. [Phase 4: Intermediate Representation](#phase-4-intermediate-representation)
7. [Phase 5: Code Generation](#phase-5-code-generation)
8. [Complete Build Workflow](#complete-build-workflow)
9. [Example Implementation](#example-implementation)

---

## Overview

This document describes how to build a complete ANSI C89/C90 compiler using
LALRGen as the parser generator. The compiler will:

- Accept standard C source files as input
- Perform full lexical, syntactic, and semantic analysis
- Generate executable code (via assembly or object files)

### Prerequisites

- LALRGen x64 build (from cleancode-22)
- C/C++ compiler (MSVC, GCC, or Clang)
- Understanding of compiler theory basics

### Reference Materials

- ISO/IEC 9899:1990 (C90 Standard)
- "Compiler Design in C" by Allen Holub
- "The C Programming Language" by Kernighan & Ritchie (2nd Edition)

---

## Compiler Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        C Source File                            │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 1: LEXER (Scanner)                                       │
│  - Character stream → Token stream                              │
│  - Handles preprocessor output                                  │
│  - Recognizes keywords, identifiers, literals, operators        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 2: PARSER (LALRGen-generated)                            │
│  - Token stream → Abstract Syntax Tree (AST)                    │
│  - Validates syntax according to C grammar                      │
│  - Builds parse tree with semantic actions                      │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 3: SEMANTIC ANALYZER                                     │
│  - Symbol table management                                      │
│  - Type checking and coercion                                   │
│  - Scope resolution                                             │
│  - Constant folding                                             │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 4: INTERMEDIATE REPRESENTATION                           │
│  - Three-address code or SSA form                               │
│  - Platform-independent optimization                            │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Phase 5: CODE GENERATOR                                        │
│  - IR → Target assembly/machine code                            │
│  - Register allocation                                          │
│  - Instruction selection                                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Executable / Object File                     │
└─────────────────────────────────────────────────────────────────┘
```

---

## Phase 1: Lexical Analysis

### Token Categories

The lexer must recognize these token types:

```c
// Token type enumeration
typedef enum {
    // Literals
    TOK_INTEGER_CONSTANT,
    TOK_FLOATING_CONSTANT,
    TOK_CHARACTER_CONSTANT,
    TOK_STRING_LITERAL,

    // Identifier (also used for typedef names)
    TOK_IDENTIFIER,

    // Keywords (32 in C89)
    TOK_AUTO, TOK_BREAK, TOK_CASE, TOK_CHAR, TOK_CONST,
    TOK_CONTINUE, TOK_DEFAULT, TOK_DO, TOK_DOUBLE, TOK_ELSE,
    TOK_ENUM, TOK_EXTERN, TOK_FLOAT, TOK_FOR, TOK_GOTO,
    TOK_IF, TOK_INT, TOK_LONG, TOK_REGISTER, TOK_RETURN,
    TOK_SHORT, TOK_SIGNED, TOK_SIZEOF, TOK_STATIC, TOK_STRUCT,
    TOK_SWITCH, TOK_TYPEDEF, TOK_UNION, TOK_UNSIGNED, TOK_VOID,
    TOK_VOLATILE, TOK_WHILE,

    // Multi-character operators
    TOK_ARROW,      // ->
    TOK_INC,        // ++
    TOK_DEC,        // --
    TOK_LEFT_OP,    // <<
    TOK_RIGHT_OP,   // >>
    TOK_LE_OP,      // <=
    TOK_GE_OP,      // >=
    TOK_EQ_OP,      // ==
    TOK_NE_OP,      // !=
    TOK_AND_OP,     // &&
    TOK_OR_OP,      // ||
    TOK_MUL_ASSIGN, // *=
    TOK_DIV_ASSIGN, // /=
    TOK_MOD_ASSIGN, // %=
    TOK_ADD_ASSIGN, // +=
    TOK_SUB_ASSIGN, // -=
    TOK_LEFT_ASSIGN,  // <<=
    TOK_RIGHT_ASSIGN, // >>=
    TOK_AND_ASSIGN, // &=
    TOK_XOR_ASSIGN, // ^=
    TOK_OR_ASSIGN,  // |=
    TOK_ELLIPSIS,   // ...

    // Single-character tokens (returned as themselves)
    // ; { } , : = ( ) [ ] . & ! ~ - + * / % < > ^ | ?

    TOK_EOF
} TokenType;
```

### Lexer Implementation

```c
// lexer.h
typedef struct {
    TokenType type;
    char *text;           // Token text
    int line, column;     // Source location
    union {
        long long int_val;
        double float_val;
        char char_val;
    } value;
} Token;

// Lexer state
typedef struct {
    const char *source;
    const char *current;
    int line, column;
    SymbolTable *symtab;  // For typedef name lookup
} Lexer;

Token lexer_next_token(Lexer *lex);
```

### The Typedef Problem

C has a lexical ambiguity: identifiers can be either regular identifiers or
typedef names (which act as type specifiers). The lexer must consult the
symbol table to distinguish:

```c
Token lexer_next_token(Lexer *lex) {
    // ... skip whitespace, comments ...

    if (is_alpha(*lex->current) || *lex->current == '_') {
        // Read identifier
        char *ident = read_identifier(lex);

        // Check if it's a keyword
        TokenType kw = lookup_keyword(ident);
        if (kw != TOK_IDENTIFIER) {
            return make_token(kw, ident);
        }

        // Check if it's a typedef name (CRITICAL!)
        if (symtab_is_typedef(lex->symtab, ident)) {
            return make_token(TOK_TYPEDEF_NAME, ident);
        }

        return make_token(TOK_IDENTIFIER, ident);
    }
    // ... handle numbers, strings, operators ...
}
```

---

## Phase 2: Syntax Analysis with LALRGen

### Step 1: Create the Grammar File

Use the complete ANSI C grammar (see `test_grammars/ansi_c_full.y`):

```yacc
%{
/* c_parser.y - ANSI C Grammar for LALRGen */
#include "ast.h"
#include "symtab.h"
%}

%token IDENTIFIER INTEGER_CONSTANT FLOATING_CONSTANT ...
%token AUTO BREAK CASE CHAR CONST ...

/* Operator precedence */
%right '=' ADD_ASSIGN SUB_ASSIGN ...
%left OR_OP
%left AND_OP
...

%%

translation_unit
    : external_declaration
        { $$ = ast_create_translation_unit($1); }
    | translation_unit external_declaration
        { $$ = ast_add_declaration($1, $2); }
    ;

/* ... rest of grammar with semantic actions ... */

%%
```

### Step 2: Generate Parser Tables

```batch
LALRGen.exe c_parser.y
```

This generates `synout0.cpp` containing:
- Token symbol table
- LALR(1) action table
- Goto table
- Reduction rules

### Step 3: Integrate with Parser Driver

```c
// parser.c
#include "synout0.cpp"  // LALRGen output

typedef struct {
    int state;
    YYSTYPE value;  // Semantic value (AST node pointer)
} StackEntry;

AST_Node *parse(Lexer *lex) {
    Stack stack;
    stack_push(&stack, 0, NULL);  // Initial state

    Token tok = lexer_next_token(lex);

    while (1) {
        int state = stack_top_state(&stack);
        int action = action_table[state][tok.type];

        if (action > 0) {
            // Shift
            stack_push(&stack, action, tok.value);
            tok = lexer_next_token(lex);
        }
        else if (action < 0) {
            // Reduce by production -action
            int prod = -action;
            int len = production_length[prod];

            // Pop len items, call semantic action
            YYSTYPE result = call_semantic_action(prod, &stack);

            // Push goto state
            int goto_state = goto_table[stack_top_state(&stack)][lhs[prod]];
            stack_push(&stack, goto_state, result);
        }
        else if (action == 0) {
            // Accept
            return stack_top_value(&stack);
        }
        else {
            // Error
            syntax_error(tok);
        }
    }
}
```

### Semantic Actions for AST Construction

```c
// In grammar file, semantic actions build the AST:

primary_expression
    : IDENTIFIER
        { $$ = ast_identifier($1); }
    | INTEGER_CONSTANT
        { $$ = ast_integer_constant($1); }
    | '(' expression ')'
        { $$ = $2; }
    ;

postfix_expression
    : primary_expression
        { $$ = $1; }
    | postfix_expression '[' expression ']'
        { $$ = ast_array_subscript($1, $3); }
    | postfix_expression '(' argument_expression_list ')'
        { $$ = ast_function_call($1, $3); }
    ;
```

---

## Phase 3: Semantic Analysis

### Symbol Table Structure

```c
typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_TYPEDEF,
    SYM_STRUCT_TAG,
    SYM_UNION_TAG,
    SYM_ENUM_TAG,
    SYM_ENUM_CONSTANT,
    SYM_LABEL
} SymbolKind;

typedef struct Symbol {
    char *name;
    SymbolKind kind;
    Type *type;
    int scope_level;
    int offset;           // Stack offset for locals
    int is_defined;       // For forward declarations
    struct Symbol *next;  // Hash chain
} Symbol;

typedef struct Scope {
    Symbol *symbols[HASH_SIZE];
    struct Scope *parent;
    int level;
} Scope;

typedef struct SymbolTable {
    Scope *current_scope;
    Scope *global_scope;
} SymbolTable;
```

### Type System

```c
typedef enum {
    TYPE_VOID,
    TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG,
    TYPE_FLOAT, TYPE_DOUBLE,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM
} TypeKind;

typedef struct Type {
    TypeKind kind;
    int is_unsigned;
    int is_const;
    int is_volatile;

    union {
        struct Type *pointed_to;     // For pointers
        struct {
            struct Type *element;
            int size;
        } array;
        struct {
            struct Type *return_type;
            struct Type **params;
            int param_count;
            int is_variadic;
        } function;
        struct {
            Symbol *members;
            int size;
            int alignment;
        } aggregate;
    };
} Type;
```

### Type Checking

```c
void check_expression(AST_Node *node, SymbolTable *symtab) {
    switch (node->kind) {
        case AST_BINARY_OP:
            check_expression(node->left, symtab);
            check_expression(node->right, symtab);

            // Type compatibility check
            Type *left_type = node->left->type;
            Type *right_type = node->right->type;

            switch (node->op) {
                case OP_ADD:
                case OP_SUB:
                    if (is_pointer(left_type) && is_integer(right_type)) {
                        node->type = left_type;  // Pointer arithmetic
                    } else if (is_arithmetic(left_type) && is_arithmetic(right_type)) {
                        node->type = usual_arithmetic_conversions(left_type, right_type);
                    } else {
                        error("Invalid operands to binary +/-");
                    }
                    break;
                // ... other operators
            }
            break;

        case AST_ASSIGN:
            check_expression(node->left, symtab);
            check_expression(node->right, symtab);

            if (!is_lvalue(node->left)) {
                error("Left operand must be lvalue");
            }
            if (!types_compatible(node->left->type, node->right->type)) {
                // Insert implicit conversion if possible
                node->right = insert_conversion(node->right, node->left->type);
            }
            node->type = node->left->type;
            break;

        // ... other cases
    }
}
```

---

## Phase 4: Intermediate Representation

### Three-Address Code (TAC)

```c
typedef enum {
    TAC_ADD, TAC_SUB, TAC_MUL, TAC_DIV, TAC_MOD,
    TAC_NEG, TAC_NOT,
    TAC_COPY, TAC_LOAD, TAC_STORE,
    TAC_LABEL, TAC_JUMP, TAC_CJUMP,
    TAC_CALL, TAC_RETURN,
    TAC_PARAM, TAC_ARG
} TacOp;

typedef struct TacInstr {
    TacOp op;
    Operand *result;
    Operand *arg1;
    Operand *arg2;
    struct TacInstr *next;
} TacInstr;

// Example: a = b + c * d
//   t1 = c * d
//   t2 = b + t1
//   a = t2
```

### IR Generation from AST

```c
Operand *generate_expr(AST_Node *node, TacList *code) {
    switch (node->kind) {
        case AST_INTEGER:
            return make_constant(node->int_value);

        case AST_IDENTIFIER:
            return lookup_variable(node->name);

        case AST_BINARY_OP: {
            Operand *left = generate_expr(node->left, code);
            Operand *right = generate_expr(node->right, code);
            Operand *result = new_temp();

            TacOp op;
            switch (node->op) {
                case OP_ADD: op = TAC_ADD; break;
                case OP_SUB: op = TAC_SUB; break;
                case OP_MUL: op = TAC_MUL; break;
                case OP_DIV: op = TAC_DIV; break;
            }

            emit(code, op, result, left, right);
            return result;
        }

        case AST_ASSIGN: {
            Operand *value = generate_expr(node->right, code);
            Operand *target = generate_lvalue(node->left, code);
            emit(code, TAC_STORE, target, value, NULL);
            return value;
        }
    }
}
```

---

## Phase 5: Code Generation

### Target: x86-64 Assembly

```c
void generate_x64(TacInstr *code, FILE *out) {
    fprintf(out, "    .text\n");

    for (TacInstr *instr = code; instr; instr = instr->next) {
        switch (instr->op) {
            case TAC_ADD:
                fprintf(out, "    movq %s, %%rax\n", operand_str(instr->arg1));
                fprintf(out, "    addq %s, %%rax\n", operand_str(instr->arg2));
                fprintf(out, "    movq %%rax, %s\n", operand_str(instr->result));
                break;

            case TAC_CALL:
                // Set up arguments per ABI
                fprintf(out, "    call %s\n", instr->arg1->name);
                fprintf(out, "    movq %%rax, %s\n", operand_str(instr->result));
                break;

            case TAC_RETURN:
                if (instr->arg1) {
                    fprintf(out, "    movq %s, %%rax\n", operand_str(instr->arg1));
                }
                fprintf(out, "    ret\n");
                break;

            // ... other instructions
        }
    }
}
```

### Register Allocation

Simple approach using graph coloring or linear scan:

```c
typedef struct {
    int reg;           // Assigned register (-1 if spilled)
    int spill_offset;  // Stack offset if spilled
} Allocation;

void allocate_registers(TacList *code) {
    // Build interference graph
    Graph *interference = build_interference_graph(code);

    // Color graph with available registers
    // x86-64 has RAX, RBX, RCX, RDX, RSI, RDI, R8-R15 (some reserved)
    int num_regs = 10;  // Usable registers

    color_graph(interference, num_regs);

    // Spill variables that couldn't be colored
    for (Temp *t = temps; t; t = t->next) {
        if (t->color == -1) {
            t->spill_offset = allocate_stack_slot();
        }
    }
}
```

---

## Complete Build Workflow

### Directory Structure

```
c_compiler/
├── src/
│   ├── main.c           # Driver
│   ├── lexer.c          # Lexical analyzer
│   ├── lexer.h
│   ├── parser.c         # Parser driver
│   ├── parser.h
│   ├── c_grammar.y      # LALRGen grammar
│   ├── synout0.cpp      # Generated parser tables
│   ├── ast.c            # AST construction
│   ├── ast.h
│   ├── symtab.c         # Symbol table
│   ├── symtab.h
│   ├── types.c          # Type system
│   ├── types.h
│   ├── semantic.c       # Semantic analysis
│   ├── semantic.h
│   ├── ir.c             # IR generation
│   ├── ir.h
│   ├── codegen.c        # Code generation
│   └── codegen.h
├── include/
│   └── tokens.h         # Token definitions
├── test/
│   └── *.c              # Test programs
└── CMakeLists.txt
```

### Build Steps

```batch
REM 1. Generate parser tables from grammar
LALRGen.exe src/c_grammar.y
move synout0.cpp src/

REM 2. Build the compiler
cmake -B build -S .
cmake --build build --config Release

REM 3. Test with a C program
build\Release\cc.exe test\hello.c -o hello.exe
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(CCompiler C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(cc
    src/main.c
    src/lexer.c
    src/parser.c
    src/synout0.cpp
    src/ast.c
    src/symtab.c
    src/types.c
    src/semantic.c
    src/ir.c
    src/codegen.c
)

target_include_directories(cc PRIVATE include)
```

---

## Example Implementation

### Minimal Working Example

Here's a simplified example that compiles expressions to x86-64:

```c
// main.c - Minimal expression compiler
#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cc <source.c>\n");
        return 1;
    }

    // Read source file
    char *source = read_file(argv[1]);

    // Lexical analysis
    Lexer lexer;
    lexer_init(&lexer, source);

    // Syntax analysis (using LALRGen tables)
    AST_Node *ast = parse(&lexer);
    if (!ast) {
        fprintf(stderr, "Parse error\n");
        return 1;
    }

    // Semantic analysis
    SymbolTable symtab;
    symtab_init(&symtab);
    if (!check_semantics(ast, &symtab)) {
        fprintf(stderr, "Semantic error\n");
        return 1;
    }

    // Generate IR
    TacList *ir = generate_ir(ast);

    // Generate assembly
    FILE *out = fopen("output.s", "w");
    generate_x64(ir, out);
    fclose(out);

    // Assemble and link (using system assembler)
    system("as -o output.o output.s");
    system("ld -o a.out output.o -lc");

    printf("Compilation successful\n");
    return 0;
}
```

---

## Further Reading

1. **"Compiler Design in C"** by Allen Holub
   - Original source of LALRGen
   - Complete compiler implementation examples

2. **"Engineering a Compiler"** by Cooper & Torczon
   - Modern compiler construction techniques
   - SSA form and optimization

3. **"Modern Compiler Implementation in C"** by Andrew Appel
   - Complete compiler for a C-like language
   - Detailed treatment of all phases

4. **Intel x86-64 Architecture Manual**
   - Instruction set reference
   - Calling conventions (System V ABI)

5. **ISO/IEC 9899:1990**
   - Official C90 language specification
   - Formal grammar in Appendix A

---

## Appendix: LALRGen Grammar Template

See `test_grammars/ansi_c_full.y` for the complete ANSI C grammar ready for
use with LALRGen.

Key points:
- All 32 C89 keywords declared as tokens
- Complete operator precedence declarations
- Full expression, declaration, statement, and external definition rules
- Expected conflicts documented (typedef name and dangling else)
