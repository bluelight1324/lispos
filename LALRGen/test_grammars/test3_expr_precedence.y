%{
/* Test 3: Expressions with operator precedence */
%}

%token NUM
%token ID
%token PLUS MINUS
%token STAR SLASH
%token LPAREN RPAREN

%left PLUS MINUS
%left STAR SLASH

%%

start: expr
     ;

expr: expr PLUS expr
    | expr MINUS expr
    | expr STAR expr
    | expr SLASH expr
    | LPAREN expr RPAREN
    | NUM
    | ID
    ;

%%
