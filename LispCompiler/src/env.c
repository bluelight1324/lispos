/*
 * env.c - Lisp Environment Implementation
 */

#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Create a new environment */
Environment *env_create(Environment *parent) {
    Environment *env = (Environment *)malloc(sizeof(Environment));
    env->bindings = NULL;
    env->parent = parent;
    env->level = parent ? parent->level + 1 : 0;
    return env;
}

/* Free an environment */
void env_free(Environment *env) {
    if (!env) return;

    /* Free bindings */
    Binding *b = env->bindings;
    while (b) {
        Binding *next = b->next;
        free(b);
        b = next;
    }

    free(env);
}

/* Look up a variable */
LispObject *env_lookup(Environment *env, LispObject *symbol) {
    if (!is_symbol(symbol)) {
        lisp_error("env_lookup: not a symbol");
        return NULL;
    }

    /* Search through the environment chain */
    for (Environment *e = env; e != NULL; e = e->parent) {
        for (Binding *b = e->bindings; b != NULL; b = b->next) {
            if (symbol_eq(b->symbol, symbol)) {
                return b->value;
            }
        }
    }

    return NULL;  /* Not found */
}

/* Define a new variable in current scope */
void env_define(Environment *env, LispObject *symbol, LispObject *value) {
    if (!is_symbol(symbol)) {
        lisp_error("env_define: not a symbol");
        return;
    }

    /* Check if already defined in current scope */
    for (Binding *b = env->bindings; b != NULL; b = b->next) {
        if (symbol_eq(b->symbol, symbol)) {
            /* Update existing binding */
            b->value = value;
            return;
        }
    }

    /* Create new binding */
    Binding *b = (Binding *)malloc(sizeof(Binding));
    b->symbol = symbol;
    b->value = value;
    b->next = env->bindings;
    env->bindings = b;
}

/* Set an existing variable */
int env_set(Environment *env, LispObject *symbol, LispObject *value) {
    if (!is_symbol(symbol)) {
        lisp_error("env_set: not a symbol");
        return 0;
    }

    /* Search through the environment chain */
    for (Environment *e = env; e != NULL; e = e->parent) {
        for (Binding *b = e->bindings; b != NULL; b = b->next) {
            if (symbol_eq(b->symbol, symbol)) {
                b->value = value;
                return 1;  /* Success */
            }
        }
    }

    return 0;  /* Not found */
}

/* Check if defined in current scope only */
int env_is_defined_local(Environment *env, LispObject *symbol) {
    if (!is_symbol(symbol)) return 0;

    for (Binding *b = env->bindings; b != NULL; b = b->next) {
        if (symbol_eq(b->symbol, symbol)) {
            return 1;
        }
    }

    return 0;
}

/* Print environment for debugging */
void env_print(Environment *env) {
    printf("Environment (level %d):\n", env->level);

    for (Binding *b = env->bindings; b != NULL; b = b->next) {
        printf("  %s = ", b->symbol->symbol.name);
        lisp_print(b->value);
        printf("\n");
    }

    if (env->parent) {
        printf("  [parent scope follows]\n");
    }
}

/* Create global environment - primitives are added separately */
Environment *env_create_global(void) {
    Environment *global = env_create(NULL);

    /* Define basic constants */
    env_define(global, make_symbol("#t"), LISP_TRUE);
    env_define(global, make_symbol("#f"), LISP_FALSE);

    return global;
}
