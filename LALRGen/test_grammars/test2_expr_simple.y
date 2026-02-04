%{
/* Test 2: Simple expressions - addition only */
%}

%token NUM
%token PLUS

%%

start: expr
     ;

expr: expr PLUS term
    | term
    ;

term: NUM
    ;

%%
