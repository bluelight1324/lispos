/*
 * eval.h - Lisp Evaluator
 *
 * Tree-walking interpreter for Lisp expressions.
 * Supports all standard special forms and function application.
 */

#ifndef EVAL_H
#define EVAL_H

#include "lisp.h"
#include "env.h"

/* Evaluate an expression in an environment */
LispObject *eval(LispObject *expr, Environment *env);

/* Apply a function to arguments (arguments already evaluated) */
LispObject *apply(LispObject *func, LispObject *args, Environment *env);

/* Evaluate all items in a list */
LispObject *eval_list(LispObject *list, Environment *env);

/* Expand macros in an expression */
LispObject *expand_macros(LispObject *expr, Environment *env);

/* Expand quasiquote expression */
LispObject *expand_quasiquote(LispObject *expr, Environment *env, int depth);

/* Essential #1: Recursion depth management */
void eval_reset_depth(void);
int eval_get_depth(void);

#endif /* EVAL_H */
