%{
/* Test 1: Minimal grammar - single production */
%}

%token NUM

%%

start: NUM
     ;

%%
