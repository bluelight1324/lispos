%{
/* Test 5: Extended C Grammar - pointers, arrays, structs, more operators */
%}

%token ID NUM STRING CHARCONST
%token INT CHAR VOID FLOAT DOUBLE LONG SHORT UNSIGNED SIGNED
%token STRUCT UNION ENUM TYPEDEF
%token IF ELSE WHILE FOR DO SWITCH CASE DEFAULT BREAK CONTINUE RETURN GOTO
%token SIZEOF
%token PLUS MINUS STAR SLASH MOD
%token AMPERSAND PIPE CARET TILDE
%token LSHIFT RSHIFT
%token EQ NE LT GT LE GE
%token ASSIGN PLUSEQ MINUSEQ STAREQ SLASHEQ MODEQ
%token ANDEQ OREQ XOREQ LSHIFTEQ RSHIFTEQ
%token AND OR NOT
%token INC DEC
%token QUESTION COLON
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMI COMMA DOT ARROW
%token ELLIPSIS
%token CONST VOLATILE

%right ASSIGN PLUSEQ MINUSEQ STAREQ SLASHEQ MODEQ ANDEQ OREQ XOREQ LSHIFTEQ RSHIFTEQ
%right QUESTION COLON
%left OR
%left AND
%left PIPE
%left CARET
%left AMPERSAND
%left EQ NE
%left LT GT LE GE
%left LSHIFT RSHIFT
%left PLUS MINUS
%left STAR SLASH MOD
%right NOT TILDE INC DEC SIZEOF

%%

start: translation_unit
     ;

translation_unit: external_decl
                | translation_unit external_decl
                ;

external_decl: function_def
             | declaration
             ;

function_def: decl_specs declarator compound_stmt
            | declarator compound_stmt
            ;

declaration: decl_specs init_declarator_list SEMI
           | decl_specs SEMI
           ;

decl_specs: storage_class_spec decl_specs
          | storage_class_spec
          | type_spec decl_specs
          | type_spec
          | type_qualifier decl_specs
          | type_qualifier
          ;

storage_class_spec: TYPEDEF
                  ;

type_spec: VOID
         | CHAR
         | SHORT
         | INT
         | LONG
         | FLOAT
         | DOUBLE
         | SIGNED
         | UNSIGNED
         | struct_or_union_spec
         | enum_spec
         | ID
         ;

type_qualifier: CONST
              | VOLATILE
              ;

struct_or_union_spec: struct_or_union ID LBRACE struct_decl_list RBRACE
                    | struct_or_union LBRACE struct_decl_list RBRACE
                    | struct_or_union ID
                    ;

struct_or_union: STRUCT
               | UNION
               ;

struct_decl_list: struct_decl
                | struct_decl_list struct_decl
                ;

struct_decl: spec_qualifier_list struct_declarator_list SEMI
           ;

spec_qualifier_list: type_spec spec_qualifier_list
                   | type_spec
                   ;

struct_declarator_list: struct_declarator
                      | struct_declarator_list COMMA struct_declarator
                      ;

struct_declarator: declarator
                 | COLON const_expr
                 | declarator COLON const_expr
                 ;

enum_spec: ENUM ID LBRACE enumerator_list RBRACE
         | ENUM LBRACE enumerator_list RBRACE
         | ENUM ID
         ;

enumerator_list: enumerator
               | enumerator_list COMMA enumerator
               ;

enumerator: ID
          | ID ASSIGN const_expr
          ;

declarator: pointer direct_declarator
          | direct_declarator
          ;

direct_declarator: ID
                 | LPAREN declarator RPAREN
                 | direct_declarator LBRACKET const_expr RBRACKET
                 | direct_declarator LBRACKET RBRACKET
                 | direct_declarator LPAREN param_type_list RPAREN
                 | direct_declarator LPAREN id_list RPAREN
                 | direct_declarator LPAREN RPAREN
                 ;

pointer: STAR
       | STAR pointer
       ;

param_type_list: param_list
               | param_list COMMA ELLIPSIS
               ;

param_list: param_decl
          | param_list COMMA param_decl
          ;

param_decl: decl_specs declarator
          | decl_specs abstract_declarator
          | decl_specs
          ;

id_list: ID
       | id_list COMMA ID
       ;

init_declarator_list: init_declarator
                    | init_declarator_list COMMA init_declarator
                    ;

init_declarator: declarator
               | declarator ASSIGN initializer
               ;

initializer: assign_expr
           | LBRACE initializer_list RBRACE
           | LBRACE initializer_list COMMA RBRACE
           ;

initializer_list: initializer
                | initializer_list COMMA initializer
                ;

abstract_declarator: pointer
                   | direct_abstract_declarator
                   | pointer direct_abstract_declarator
                   ;

direct_abstract_declarator: LPAREN abstract_declarator RPAREN
                          | LBRACKET RBRACKET
                          | LBRACKET const_expr RBRACKET
                          | direct_abstract_declarator LBRACKET RBRACKET
                          | direct_abstract_declarator LBRACKET const_expr RBRACKET
                          | LPAREN RPAREN
                          | LPAREN param_type_list RPAREN
                          | direct_abstract_declarator LPAREN RPAREN
                          | direct_abstract_declarator LPAREN param_type_list RPAREN
                          ;

compound_stmt: LBRACE RBRACE
             | LBRACE stmt_list RBRACE
             | LBRACE decl_list RBRACE
             | LBRACE decl_list stmt_list RBRACE
             ;

decl_list: declaration
         | decl_list declaration
         ;

stmt_list: stmt
         | stmt_list stmt
         ;

stmt: labeled_stmt
    | compound_stmt
    | expr_stmt
    | selection_stmt
    | iteration_stmt
    | jump_stmt
    ;

labeled_stmt: ID COLON stmt
            | CASE const_expr COLON stmt
            | DEFAULT COLON stmt
            ;

expr_stmt: expr SEMI
         | SEMI
         ;

selection_stmt: IF LPAREN expr RPAREN stmt
              | IF LPAREN expr RPAREN stmt ELSE stmt
              | SWITCH LPAREN expr RPAREN stmt
              ;

iteration_stmt: WHILE LPAREN expr RPAREN stmt
              | DO stmt WHILE LPAREN expr RPAREN SEMI
              | FOR LPAREN expr_stmt expr_stmt RPAREN stmt
              | FOR LPAREN expr_stmt expr_stmt expr RPAREN stmt
              ;

jump_stmt: GOTO ID SEMI
         | CONTINUE SEMI
         | BREAK SEMI
         | RETURN SEMI
         | RETURN expr SEMI
         ;

expr: assign_expr
    | expr COMMA assign_expr
    ;

assign_expr: cond_expr
           | unary_expr assign_op assign_expr
           ;

assign_op: ASSIGN
         | STAREQ
         | SLASHEQ
         | MODEQ
         | PLUSEQ
         | MINUSEQ
         | LSHIFTEQ
         | RSHIFTEQ
         | ANDEQ
         | XOREQ
         | OREQ
         ;

cond_expr: log_or_expr
         | log_or_expr QUESTION expr COLON cond_expr
         ;

const_expr: cond_expr
          ;

log_or_expr: log_and_expr
           | log_or_expr OR log_and_expr
           ;

log_and_expr: incl_or_expr
            | log_and_expr AND incl_or_expr
            ;

incl_or_expr: excl_or_expr
            | incl_or_expr PIPE excl_or_expr
            ;

excl_or_expr: and_expr
            | excl_or_expr CARET and_expr
            ;

and_expr: equality_expr
        | and_expr AMPERSAND equality_expr
        ;

equality_expr: rel_expr
             | equality_expr EQ rel_expr
             | equality_expr NE rel_expr
             ;

rel_expr: shift_expr
        | rel_expr LT shift_expr
        | rel_expr GT shift_expr
        | rel_expr LE shift_expr
        | rel_expr GE shift_expr
        ;

shift_expr: add_expr
          | shift_expr LSHIFT add_expr
          | shift_expr RSHIFT add_expr
          ;

add_expr: mult_expr
        | add_expr PLUS mult_expr
        | add_expr MINUS mult_expr
        ;

mult_expr: cast_expr
         | mult_expr STAR cast_expr
         | mult_expr SLASH cast_expr
         | mult_expr MOD cast_expr
         ;

cast_expr: unary_expr
         | LPAREN type_name RPAREN cast_expr
         ;

type_name: spec_qualifier_list
         | spec_qualifier_list abstract_declarator
         ;

unary_expr: postfix_expr
          | INC unary_expr
          | DEC unary_expr
          | unary_op cast_expr
          | SIZEOF unary_expr
          | SIZEOF LPAREN type_name RPAREN
          ;

unary_op: AMPERSAND
        | STAR
        | PLUS
        | MINUS
        | TILDE
        | NOT
        ;

postfix_expr: primary_expr
            | postfix_expr LBRACKET expr RBRACKET
            | postfix_expr LPAREN RPAREN
            | postfix_expr LPAREN argument_expr_list RPAREN
            | postfix_expr DOT ID
            | postfix_expr ARROW ID
            | postfix_expr INC
            | postfix_expr DEC
            ;

primary_expr: ID
            | NUM
            | STRING
            | CHARCONST
            | LPAREN expr RPAREN
            ;

argument_expr_list: assign_expr
                  | argument_expr_list COMMA assign_expr
                  ;

%%
