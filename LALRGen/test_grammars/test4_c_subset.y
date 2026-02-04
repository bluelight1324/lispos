%{
/* Test 4: C Language Subset - declarations and basic statements */
%}

%token ID NUM STRING
%token INT CHAR VOID FLOAT DOUBLE
%token IF ELSE WHILE FOR RETURN
%token PLUS MINUS STAR SLASH MOD
%token EQ NE LT GT LE GE
%token ASSIGN
%token AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMI COMMA

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH MOD
%right NOT

%%

program: decl_list
       ;

decl_list: decl_list decl
         | decl
         ;

decl: var_decl
    | func_decl
    ;

var_decl: type_spec ID SEMI
        | type_spec ID ASSIGN expr SEMI
        ;

type_spec: INT
         | CHAR
         | VOID
         | FLOAT
         | DOUBLE
         ;

func_decl: type_spec ID LPAREN param_list RPAREN compound_stmt
         | type_spec ID LPAREN RPAREN compound_stmt
         ;

param_list: param_list COMMA param
          | param
          ;

param: type_spec ID
     ;

compound_stmt: LBRACE local_decls stmt_list RBRACE
             ;

local_decls: local_decls var_decl
           | /* empty */
           ;

stmt_list: stmt_list stmt
         | /* empty */
         ;

stmt: expr_stmt
    | compound_stmt
    | if_stmt
    | while_stmt
    | return_stmt
    ;

expr_stmt: expr SEMI
         | SEMI
         ;

if_stmt: IF LPAREN expr RPAREN stmt
       | IF LPAREN expr RPAREN stmt ELSE stmt
       ;

while_stmt: WHILE LPAREN expr RPAREN stmt
          ;

return_stmt: RETURN SEMI
           | RETURN expr SEMI
           ;

expr: ID ASSIGN expr
    | simple_expr
    ;

simple_expr: simple_expr OR and_expr
           | and_expr
           ;

and_expr: and_expr AND rel_expr
        | rel_expr
        ;

rel_expr: rel_expr LT add_expr
        | rel_expr GT add_expr
        | rel_expr LE add_expr
        | rel_expr GE add_expr
        | rel_expr EQ add_expr
        | rel_expr NE add_expr
        | add_expr
        ;

add_expr: add_expr PLUS term
        | add_expr MINUS term
        | term
        ;

term: term STAR factor
    | term SLASH factor
    | term MOD factor
    | factor
    ;

factor: LPAREN expr RPAREN
      | ID
      | ID LPAREN args RPAREN
      | ID LPAREN RPAREN
      | NUM
      | STRING
      | NOT factor
      ;

args: args COMMA expr
    | expr
    ;

%%
