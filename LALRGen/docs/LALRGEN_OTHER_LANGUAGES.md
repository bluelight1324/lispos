# LALRGen for Other Languages and Codes

LALRGen is a general-purpose LALR(1) parser generator. While tested with C
grammars, it can generate parsers for any language expressible as a
context-free grammar. This document covers the full range of applications.

## Table of Contents

1. [Overview](#overview)
2. [Programming Languages](#programming-languages)
3. [Domain-Specific Languages](#domain-specific-languages)
4. [Data Formats](#data-formats)
5. [Mathematical and Logic Languages](#mathematical-and-logic-languages)
6. [Hardware Description Languages](#hardware-description-languages)
7. [Query Languages](#query-languages)
8. [Configuration and Markup](#configuration-and-markup)
9. [Example Grammars](#example-grammars)
10. [Limitations](#limitations)

---

## Overview

### What LALRGen Can Parse

Any language that can be expressed as a **context-free grammar (CFG)**
with at most LALR(1) lookahead. This covers the vast majority of
programming languages, data formats, and DSLs.

```
LALRGen Applicability:

✅ Can parse:
  • Deterministic context-free languages
  • Languages with 1-token lookahead
  • Ambiguous grammars (with precedence/associativity declarations)

❌ Cannot parse:
  • Context-sensitive constructs (without lexer hacks)
  • Languages requiring unbounded lookahead
  • Indentation-sensitive languages (without preprocessor)
```

### Tested Capabilities

| Metric | Demonstrated |
|--------|-------------|
| Max productions | 150 (ANSI C) |
| Max output size | 80 KB |
| Max conflicts handled | 6 |
| Precedence support | %left, %right, %nonassoc |

---

## Programming Languages

### Languages Suitable for LALRGen

| Language | Difficulty | Notes |
|----------|------------|-------|
| **C** | Medium | Tested (typedef hack needed) |
| **Pascal** | Easy | Clean LALR(1) grammar |
| **Java** | Medium | Large but mostly LALR(1) |
| **JavaScript** | Hard | Ambiguities, ASI |
| **Python** | Hard | Indentation-sensitive (needs preprocessor) |
| **Go** | Easy | Designed for simple parsing |
| **Rust** | Hard | Complex syntax, macros |
| **Lua** | Easy | Small, clean grammar |
| **SQL** | Medium | Well-suited to LALR |
| **BASIC** | Easy | Simple grammar |
| **Fortran** | Medium | Fixed-format variants easy |
| **Scheme/Lisp** | Trivial | S-expressions are minimal |
| **Prolog** | Easy | Operator-based grammar |
| **Haskell** | Hard | Indentation, operators |

### Example: Pascal Grammar

```yacc
%token PROGRAM BEGIN END VAR IF THEN ELSE WHILE DO
%token PROCEDURE FUNCTION ARRAY OF INTEGER REAL
%token ASSIGN SEMICOLON COLON COMMA DOT DOTDOT
%token LPAREN RPAREN LBRACKET RBRACKET
%token PLUS MINUS STAR SLASH EQ NE LT LE GT GE
%token AND OR NOT DIV MOD
%token IDENTIFIER INTEGER_LIT REAL_LIT STRING_LIT

%%

program
    : PROGRAM IDENTIFIER SEMICOLON block DOT
    ;

block
    : declarations compound_statement
    ;

declarations
    : /* empty */
    | declarations var_declaration
    | declarations procedure_declaration
    | declarations function_declaration
    ;

var_declaration
    : VAR var_decl_list
    ;

var_decl_list
    : var_decl
    | var_decl_list var_decl
    ;

var_decl
    : identifier_list COLON type SEMICOLON
    ;

type
    : INTEGER
    | REAL
    | ARRAY LBRACKET INTEGER_LIT DOTDOT INTEGER_LIT RBRACKET OF type
    ;

compound_statement
    : BEGIN statement_list END
    ;

statement_list
    : statement
    | statement_list SEMICOLON statement
    ;

statement
    : /* empty */
    | IDENTIFIER ASSIGN expression
    | IDENTIFIER LPAREN expression_list RPAREN
    | compound_statement
    | IF expression THEN statement
    | IF expression THEN statement ELSE statement
    | WHILE expression DO statement
    ;

expression
    : simple_expression
    | simple_expression relop simple_expression
    ;

simple_expression
    : term
    | simple_expression addop term
    ;

term
    : factor
    | term mulop factor
    ;

factor
    : IDENTIFIER
    | INTEGER_LIT
    | REAL_LIT
    | LPAREN expression RPAREN
    | NOT factor
    ;

relop : EQ | NE | LT | LE | GT | GE ;
addop : PLUS | MINUS | OR ;
mulop : STAR | SLASH | DIV | MOD | AND ;

%%
```

### Example: Lua Grammar (Simplified)

```yacc
%token NAME NUMBER STRING
%token AND BREAK DO ELSE ELSEIF END
%token FALSE FOR FUNCTION IF IN
%token LOCAL NIL NOT OR REPEAT
%token RETURN THEN TRUE UNTIL WHILE
%token PLUS MINUS STAR SLASH PERCENT CARET HASH
%token EQ NE LT GT LE GE
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMICOLON COLON COMMA DOT DOTDOT DOTS ASSIGN
%token CONCAT AND_OP OR_OP

%%

chunk
    : block
    ;

block
    : stat_list
    | stat_list laststat
    | stat_list laststat SEMICOLON
    ;

stat_list
    : /* empty */
    | stat_list stat
    | stat_list stat SEMICOLON
    ;

stat
    : varlist ASSIGN explist
    | functioncall
    | DO block END
    | WHILE exp DO block END
    | REPEAT block UNTIL exp
    | IF exp THEN block elseif_list END
    | IF exp THEN block elseif_list ELSE block END
    | FOR NAME ASSIGN exp COMMA exp DO block END
    | FOR NAME ASSIGN exp COMMA exp COMMA exp DO block END
    | FOR namelist IN explist DO block END
    | FUNCTION funcname funcbody
    | LOCAL FUNCTION NAME funcbody
    | LOCAL namelist
    | LOCAL namelist ASSIGN explist
    ;

laststat
    : RETURN
    | RETURN explist
    | BREAK
    ;

exp
    : NIL | FALSE | TRUE | NUMBER | STRING | DOTS
    | function | prefixexp | tableconstructor
    | exp binop exp
    | unop exp
    ;

%%
```

### Example: Scheme (S-Expressions)

```yacc
%token LPAREN RPAREN DOT QUOTE
%token SYMBOL NUMBER STRING BOOLEAN CHARACTER

%%

program
    : datum_list
    ;

datum_list
    : datum
    | datum_list datum
    ;

datum
    : simple_datum
    | compound_datum
    | QUOTE datum
    ;

simple_datum
    : SYMBOL
    | NUMBER
    | STRING
    | BOOLEAN
    | CHARACTER
    ;

compound_datum
    : LPAREN datum_list RPAREN
    | LPAREN datum_list DOT datum RPAREN
    | LPAREN RPAREN
    ;

%%
```

---

## Domain-Specific Languages

### Languages Well-Suited to LALRGen

| DSL | Domain | Complexity |
|-----|--------|------------|
| **Regular expressions** | Text matching | Simple |
| **Shader languages (GLSL/HLSL)** | Graphics | Medium |
| **Make/Build DSLs** | Build systems | Simple |
| **Awk** | Text processing | Simple |
| **Template engines** | Web/text | Medium |
| **Game scripting** | Games | Simple-Medium |
| **Music notation (LilyPond)** | Music | Medium |
| **Chemical formulas** | Chemistry | Simple |
| **Math expressions** | Computation | Simple |

### Example: Regular Expression Grammar

```yacc
%token CHAR DOT STAR PLUS QUESTION
%token LPAREN RPAREN LBRACKET RBRACKET CARET DASH
%token PIPE BACKSLASH

%%

regex
    : alternation
    ;

alternation
    : concatenation
    | alternation PIPE concatenation
    ;

concatenation
    : repetition
    | concatenation repetition
    ;

repetition
    : atom
    | atom STAR
    | atom PLUS
    | atom QUESTION
    ;

atom
    : CHAR
    | DOT
    | BACKSLASH CHAR
    | LPAREN regex RPAREN
    | LBRACKET char_class RBRACKET
    | LBRACKET CARET char_class RBRACKET
    ;

char_class
    : char_range
    | char_class char_range
    ;

char_range
    : CHAR
    | CHAR DASH CHAR
    ;

%%
```

### Example: Shader Language (GLSL-like)

```yacc
%token VOID FLOAT VEC2 VEC3 VEC4 MAT4 SAMPLER2D
%token UNIFORM VARYING ATTRIBUTE IN OUT
%token IF ELSE FOR WHILE RETURN DISCARD
%token IDENTIFIER FLOAT_LIT INT_LIT
%token ASSIGN SEMICOLON COMMA LBRACE RBRACE
%token LPAREN RPAREN LBRACKET RBRACKET DOT

%left PLUS MINUS
%left STAR SLASH
%right UMINUS

%%

shader
    : declaration_list
    ;

declaration_list
    : declaration
    | declaration_list declaration
    ;

declaration
    : qualifier type IDENTIFIER SEMICOLON
    | type IDENTIFIER LPAREN param_list RPAREN compound_stmt
    ;

qualifier
    : /* empty */
    | UNIFORM
    | VARYING
    | ATTRIBUTE
    | IN
    | OUT
    ;

type
    : VOID | FLOAT | VEC2 | VEC3 | VEC4 | MAT4 | SAMPLER2D
    ;

compound_stmt
    : LBRACE statement_list RBRACE
    ;

statement_list
    : /* empty */
    | statement_list statement
    ;

statement
    : expression SEMICOLON
    | type IDENTIFIER ASSIGN expression SEMICOLON
    | IF LPAREN expression RPAREN statement
    | IF LPAREN expression RPAREN statement ELSE statement
    | FOR LPAREN expression SEMICOLON expression SEMICOLON expression RPAREN statement
    | RETURN expression SEMICOLON
    | RETURN SEMICOLON
    | DISCARD SEMICOLON
    | compound_stmt
    ;

expression
    : FLOAT_LIT | INT_LIT | IDENTIFIER
    | expression DOT IDENTIFIER
    | IDENTIFIER LPAREN arg_list RPAREN
    | expression PLUS expression
    | expression MINUS expression
    | expression STAR expression
    | expression SLASH expression
    | LPAREN expression RPAREN
    ;

%%
```

---

## Data Formats

### Parseable Data Formats

| Format | Complexity | Notes |
|--------|------------|-------|
| **JSON** | Trivial | ~15 productions |
| **XML** | Simple | Needs lexer cooperation |
| **CSV** | Trivial | ~5 productions |
| **TOML** | Simple | Clean grammar |
| **YAML** | Hard | Indentation-sensitive |
| **Protocol Buffers** | Simple | Schema definition |
| **GraphQL** | Medium | Query + schema |

### Example: JSON Grammar

```yacc
%token STRING NUMBER TRUE FALSE NULL_TOKEN
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA

%%

json
    : value
    ;

value
    : object
    | array
    | STRING
    | NUMBER
    | TRUE
    | FALSE
    | NULL_TOKEN
    ;

object
    : LBRACE RBRACE
    | LBRACE member_list RBRACE
    ;

member_list
    : member
    | member_list COMMA member
    ;

member
    : STRING COLON value
    ;

array
    : LBRACKET RBRACKET
    | LBRACKET value_list RBRACKET
    ;

value_list
    : value
    | value_list COMMA value
    ;

%%
```

### Example: GraphQL Grammar

```yacc
%token NAME INT_VALUE FLOAT_VALUE STRING_VALUE
%token QUERY MUTATION SUBSCRIPTION FRAGMENT ON
%token TYPE INTERFACE UNION ENUM INPUT EXTEND SCALAR
%token IMPLEMENTS DIRECTIVE SCHEMA
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET
%token COLON BANG EQUALS AT DOLLAR PIPE DOTS
%token TRUE FALSE NULL_TOKEN

%%

document
    : definition_list
    ;

definition_list
    : definition
    | definition_list definition
    ;

definition
    : operation_definition
    | fragment_definition
    | type_definition
    ;

operation_definition
    : selection_set
    | operation_type NAME selection_set
    | operation_type NAME variable_definitions selection_set
    ;

operation_type
    : QUERY | MUTATION | SUBSCRIPTION
    ;

selection_set
    : LBRACE selection_list RBRACE
    ;

selection_list
    : selection
    | selection_list selection
    ;

selection
    : field
    | DOTS ON NAME selection_set
    | DOTS NAME
    ;

field
    : NAME
    | NAME arguments
    | NAME selection_set
    | NAME arguments selection_set
    | NAME COLON NAME
    | NAME COLON NAME arguments
    | NAME COLON NAME selection_set
    | NAME COLON NAME arguments selection_set
    ;

%%
```

---

## Mathematical and Logic Languages

| Language | Domain | Complexity |
|----------|--------|------------|
| **LaTeX math** | Typesetting | Medium |
| **Prolog** | Logic programming | Easy |
| **Lambda calculus** | Computation | Simple |
| **First-order logic** | Formal logic | Simple |
| **Category theory DSL** | Mathematics | Medium |
| **Linear algebra DSL** | Mathematics | Simple |

### Example: Lambda Calculus

```yacc
%token LAMBDA DOT LPAREN RPAREN IDENTIFIER

%%

term
    : IDENTIFIER
    | LAMBDA IDENTIFIER DOT term
    | term term
    | LPAREN term RPAREN
    ;

%%
```

### Example: First-Order Logic

```yacc
%token FORALL EXISTS AND OR NOT IMPLIES IFF
%token LPAREN RPAREN COMMA
%token PREDICATE VARIABLE CONSTANT FUNCTION

%right IMPLIES IFF
%left OR
%left AND
%right NOT
%nonassoc FORALL EXISTS

%%

formula
    : atomic_formula
    | NOT formula
    | formula AND formula
    | formula OR formula
    | formula IMPLIES formula
    | formula IFF formula
    | FORALL VARIABLE formula
    | EXISTS VARIABLE formula
    | LPAREN formula RPAREN
    ;

atomic_formula
    : PREDICATE LPAREN term_list RPAREN
    | term EQ term
    ;

term
    : VARIABLE
    | CONSTANT
    | FUNCTION LPAREN term_list RPAREN
    ;

term_list
    : term
    | term_list COMMA term
    ;

%%
```

### Example: Category Theory DSL

```yacc
%token OBJECT MORPHISM FUNCTOR NATURAL_TRANSFORMATION
%token IDENTITY COMPOSE PRODUCT COPRODUCT EXPONENTIAL
%token TERMINAL INITIAL
%token ARROW DOUBLE_ARROW CROSS PLUS IMPLIES
%token IDENTIFIER LPAREN RPAREN LBRACKET RBRACKET
%token COMMA COLON SEMICOLON DOT

%%

program
    : declaration_list
    ;

declaration_list
    : declaration
    | declaration_list declaration
    ;

declaration
    : OBJECT IDENTIFIER SEMICOLON
    | MORPHISM IDENTIFIER COLON IDENTIFIER ARROW IDENTIFIER SEMICOLON
    | FUNCTOR IDENTIFIER COLON IDENTIFIER DOUBLE_ARROW IDENTIFIER SEMICOLON
    | equation SEMICOLON
    ;

equation
    : morphism_expr EQ morphism_expr
    ;

morphism_expr
    : IDENTIFIER
    | IDENTITY LPAREN IDENTIFIER RPAREN
    | morphism_expr COMPOSE morphism_expr
    | PRODUCT LPAREN morphism_expr COMMA morphism_expr RPAREN
    | EXPONENTIAL LPAREN IDENTIFIER COMMA IDENTIFIER RPAREN
    | LPAREN morphism_expr RPAREN
    ;

%%
```

---

## Hardware Description Languages

| Language | Domain | Complexity |
|----------|--------|------------|
| **Verilog** | Digital circuits | Medium |
| **VHDL** | Digital circuits | Medium |
| **SystemVerilog** | Verification | Hard |
| **Chisel (subset)** | HW generation | Medium |

### Example: Simple Verilog Subset

```yacc
%token MODULE ENDMODULE INPUT OUTPUT WIRE REG
%token ASSIGN ALWAYS POSEDGE NEGEDGE IF ELSE BEGIN END
%token IDENTIFIER NUMBER
%token SEMICOLON COMMA COLON LPAREN RPAREN
%token LBRACKET RBRACKET LBRACE RBRACE
%token ASSIGN_OP LE_ASSIGN
%token AND OR XOR NOT PLUS MINUS

%%

module_decl
    : MODULE IDENTIFIER LPAREN port_list RPAREN SEMICOLON
      module_body ENDMODULE
    ;

port_list
    : port
    | port_list COMMA port
    ;

port
    : IDENTIFIER
    ;

module_body
    : /* empty */
    | module_body module_item
    ;

module_item
    : port_declaration
    | wire_declaration
    | reg_declaration
    | continuous_assign
    | always_block
    ;

port_declaration
    : INPUT range IDENTIFIER SEMICOLON
    | OUTPUT range IDENTIFIER SEMICOLON
    ;

range
    : /* empty */
    | LBRACKET NUMBER COLON NUMBER RBRACKET
    ;

continuous_assign
    : ASSIGN IDENTIFIER ASSIGN_OP expression SEMICOLON
    ;

always_block
    : ALWAYS AT LPAREN sensitivity_list RPAREN statement
    ;

sensitivity_list
    : POSEDGE IDENTIFIER
    | NEGEDGE IDENTIFIER
    | sensitivity_list OR POSEDGE IDENTIFIER
    ;

statement
    : IDENTIFIER LE_ASSIGN expression SEMICOLON
    | IF LPAREN expression RPAREN statement
    | IF LPAREN expression RPAREN statement ELSE statement
    | BEGIN statement_list END
    ;

expression
    : IDENTIFIER | NUMBER
    | expression AND expression
    | expression OR expression
    | expression XOR expression
    | NOT expression
    | LPAREN expression RPAREN
    ;

%%
```

---

## Query Languages

| Language | Domain | Complexity |
|----------|--------|------------|
| **SQL** | Databases | Medium |
| **SPARQL** | RDF/Semantic web | Medium |
| **Cypher** | Graph databases | Medium |
| **XPath** | XML navigation | Simple |
| **jq** | JSON processing | Simple |

### Example: SQL Subset

```yacc
%token SELECT FROM WHERE GROUP BY HAVING ORDER ASC DESC
%token INSERT INTO VALUES UPDATE SET DELETE
%token CREATE TABLE DROP ALTER ADD
%token JOIN LEFT RIGHT INNER OUTER ON
%token AND OR NOT IN BETWEEN LIKE IS NULL_TOKEN
%token AS DISTINCT ALL COUNT SUM AVG MIN MAX
%token IDENTIFIER STRING_LIT NUMBER_LIT
%token STAR COMMA DOT SEMICOLON
%token LPAREN RPAREN
%token EQ NE LT GT LE GE

%%

statement
    : select_stmt SEMICOLON
    | insert_stmt SEMICOLON
    | update_stmt SEMICOLON
    | delete_stmt SEMICOLON
    | create_stmt SEMICOLON
    ;

select_stmt
    : SELECT select_list FROM table_ref_list
    | SELECT select_list FROM table_ref_list WHERE condition
    | SELECT select_list FROM table_ref_list WHERE condition
      GROUP BY column_list
    | SELECT select_list FROM table_ref_list WHERE condition
      GROUP BY column_list HAVING condition
    | SELECT select_list FROM table_ref_list WHERE condition
      ORDER BY order_list
    | SELECT DISTINCT select_list FROM table_ref_list
    ;

select_list
    : STAR
    | select_item_list
    ;

select_item_list
    : select_item
    | select_item_list COMMA select_item
    ;

select_item
    : expression
    | expression AS IDENTIFIER
    | aggregate_func
    ;

aggregate_func
    : COUNT LPAREN STAR RPAREN
    | COUNT LPAREN expression RPAREN
    | SUM LPAREN expression RPAREN
    | AVG LPAREN expression RPAREN
    | MIN LPAREN expression RPAREN
    | MAX LPAREN expression RPAREN
    ;

table_ref_list
    : table_ref
    | table_ref_list COMMA table_ref
    | table_ref_list JOIN table_ref ON condition
    | table_ref_list LEFT JOIN table_ref ON condition
    ;

table_ref
    : IDENTIFIER
    | IDENTIFIER AS IDENTIFIER
    ;

condition
    : expression EQ expression
    | expression NE expression
    | expression LT expression
    | expression GT expression
    | expression IS NULL_TOKEN
    | expression IS NOT NULL_TOKEN
    | expression IN LPAREN select_stmt RPAREN
    | expression BETWEEN expression AND expression
    | expression LIKE STRING_LIT
    | condition AND condition
    | condition OR condition
    | NOT condition
    | LPAREN condition RPAREN
    ;

expression
    : IDENTIFIER
    | IDENTIFIER DOT IDENTIFIER
    | NUMBER_LIT
    | STRING_LIT
    ;

%%
```

---

## Configuration and Markup

| Format | Domain | Complexity |
|--------|--------|------------|
| **INI files** | Configuration | Trivial |
| **Nginx config** | Web server | Simple |
| **Dockerfile** | Containers | Simple |
| **Makefile** | Build | Simple |
| **CSS** | Styling | Medium |
| **Markdown (subset)** | Documentation | Medium |

### Example: CSS Subset

```yacc
%token IDENTIFIER STRING NUMBER HASH DOT COLON
%token LBRACE RBRACE SEMICOLON COMMA
%token PX EM REM PERCENT
%token STAR GT PLUS TILDE

%%

stylesheet
    : rule_list
    ;

rule_list
    : rule
    | rule_list rule
    ;

rule
    : selector_list LBRACE declaration_list RBRACE
    ;

selector_list
    : selector
    | selector_list COMMA selector
    ;

selector
    : simple_selector
    | selector GT simple_selector
    | selector PLUS simple_selector
    | selector TILDE simple_selector
    | selector simple_selector
    ;

simple_selector
    : IDENTIFIER
    | HASH IDENTIFIER
    | DOT IDENTIFIER
    | COLON IDENTIFIER
    | STAR
    ;

declaration_list
    : /* empty */
    | declaration_list declaration
    ;

declaration
    : IDENTIFIER COLON value_list SEMICOLON
    ;

value_list
    : value
    | value_list value
    ;

value
    : IDENTIFIER
    | NUMBER
    | NUMBER PX
    | NUMBER EM
    | NUMBER REM
    | NUMBER PERCENT
    | HASH IDENTIFIER
    | STRING
    ;

%%
```

---

## Limitations

### Languages That Are Difficult for LALR(1)

| Language | Issue | Workaround |
|----------|-------|------------|
| **C++** | Template syntax `>>`, complex disambiguation | GLR parser |
| **Python** | Significant whitespace | Lexer emits INDENT/DEDENT |
| **Haskell** | Layout rule, operator sections | Preprocessor |
| **Ruby** | Highly context-sensitive | Hand-written parser |
| **Perl** | "Only perl can parse Perl" | Not feasible |
| **YAML** | Indentation + complex rules | Specialized parser |

### Workarounds for Common Issues

**Indentation-sensitive languages:**
```c
// Lexer preprocessor: convert indentation to tokens
// Python: INDENT/DEDENT tokens
if_stmt : IF expr COLON NEWLINE INDENT stmt_list DEDENT ;
```

**Context-sensitive lexing (typedef problem):**
```c
// Lexer consults symbol table
if (is_typedef_name(identifier))
    return TYPEDEF_NAME;
else
    return IDENTIFIER;
```

**Template syntax (C++ `>>`):**
```c
// Split >> into > > when inside template context
```

---

## Summary

### LALRGen Applicability by Domain

| Domain | Suitability | Example Languages |
|--------|-------------|-------------------|
| **Systems languages** | ★★★★★ | C, Pascal, Go |
| **Scripting languages** | ★★★★☆ | Lua, Awk, Tcl |
| **Query languages** | ★★★★★ | SQL, SPARQL, Cypher |
| **Data formats** | ★★★★★ | JSON, XML schemas, TOML |
| **DSLs** | ★★★★★ | Shaders, configs, math |
| **Logic languages** | ★★★★☆ | Prolog, FOL, lambda calculus |
| **HDLs** | ★★★★☆ | Verilog, VHDL |
| **Markup** | ★★★☆☆ | CSS, subset of HTML |
| **Complex PLs** | ★★☆☆☆ | C++, Rust, Haskell |

### Key Takeaway

LALRGen is a **general-purpose** parser generator. Any context-free grammar
with LALR(1) properties can be processed. The grammars demonstrated with C are
just one application - the same tool works for SQL, JSON, shader languages,
logic languages, hardware descriptions, and custom DSLs.

### Getting Started

1. Write your grammar in `.y` format
2. Run `LALRGen.exe grammar.y`
3. Include the generated `synout0.cpp` in your project
4. Write a lexer for your language
5. Connect lexer → parser → semantic actions

All example grammars in this document can be directly processed by LALRGen.
