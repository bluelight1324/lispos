/*
 * env.h - Lisp Environment (Scoping)
 *
 * Environments provide lexical scoping for variable bindings.
 * Each environment has a parent, forming a chain for nested scopes.
 */

#ifndef ENV_H
#define ENV_H

#include "lisp.h"

/* Forward declaration (defined in lisp.h) */
typedef struct Environment Environment;

/* Binding: a variable-value pair */
typedef struct Binding {
    LispObject *symbol;
    LispObject *value;
    struct Binding *next;
} Binding;

/* Environment: a scope with bindings and a parent scope */
struct Environment {
    Binding *bindings;
    Environment *parent;
    int level;              /* Nesting level for debugging */
};

/* Create a new environment */
Environment *env_create(Environment *parent);

/* Free an environment (does not free parent) */
void env_free(Environment *env);

/* Look up a variable in the environment chain */
LispObject *env_lookup(Environment *env, LispObject *symbol);

/* Define a new variable in the current environment */
void env_define(Environment *env, LispObject *symbol, LispObject *value);

/* Set an existing variable (searches parent scopes) */
int env_set(Environment *env, LispObject *symbol, LispObject *value);

/* Check if a variable is defined in the current scope only */
int env_is_defined_local(Environment *env, LispObject *symbol);

/* Print environment for debugging */
void env_print(Environment *env);

/* Create a global environment with standard bindings */
Environment *env_create_global(void);

/* Get all bindings as an alist ((name . value) ...) - for debugger */
LispObject *env_get_all_bindings(Environment *env);

#endif /* ENV_H */
