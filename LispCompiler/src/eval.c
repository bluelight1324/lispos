/*
 * eval.c - Lisp Evaluator Implementation
 *
 * A tree-walking interpreter for Lisp with tail call optimization.
 */

#include "eval.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Essential #1: Recursion Depth Protection
 * ============================================================ */
#define MAX_EVAL_DEPTH 10000
static int current_eval_depth = 0;

/* Reset evaluation depth (call at program start) */
void eval_reset_depth(void) {
    current_eval_depth = 0;
}

/* Get current evaluation depth */
int eval_get_depth(void) {
    return current_eval_depth;
}

/* Forward declarations */
static LispObject *eval_special_form(LispObject *expr, Environment *env);
static LispObject *eval_application(LispObject *expr, Environment *env);
LispObject *expand_quasiquote(LispObject *expr, Environment *env, int depth);

/* Check if a symbol matches a name */
static int is_form(LispObject *expr, const char *name) {
    if (!is_cons(expr)) return 0;
    LispObject *head = car(expr);
    return is_symbol_named(head, name);
}

/* Evaluate a list of expressions, returning the last result */
static LispObject *eval_sequence(LispObject *exprs, Environment *env) {
    LispObject *result = make_nil();

    while (is_cons(exprs)) {
        result = eval(car(exprs), env);
        exprs = cdr(exprs);
    }

    return result;
}

/* Bind parameters to arguments in an environment */
static void bind_parameters(Environment *env, LispObject *params, LispObject *args) {
    while (is_cons(params) && is_cons(args)) {
        env_define(env, car(params), car(args));
        params = cdr(params);
        args = cdr(args);
    }

    /* Rest parameter (dotted list) */
    if (is_symbol(params)) {
        env_define(env, params, args);
    } else if (!is_nil(params) || !is_nil(args)) {
        lisp_error("Argument count mismatch");
    }
}

/* Main evaluation function */
LispObject *eval(LispObject *expr, Environment *env) {
    /* Essential #1: Check recursion depth */
    if (++current_eval_depth > MAX_EVAL_DEPTH) {
        current_eval_depth--;
        lisp_error("Maximum recursion depth exceeded (%d levels)", MAX_EVAL_DEPTH);
        return make_nil();
    }

    /* Debug hook: check if we should break at this expression */
    if (debug_is_enabled()) {
        debug_check_break(expr, env);
    }

    /* Self-evaluating objects */
    if (!expr) {
        current_eval_depth--;
        return make_nil();
    }

    LispObject *result = NULL;

    switch (expr->type) {
        case LISP_NIL:
        case LISP_BOOLEAN:
        case LISP_NUMBER:
        case LISP_STRING:
        case LISP_CHARACTER:
        case LISP_LAMBDA:
        case LISP_PRIMITIVE:
            result = expr;
            break;

        case LISP_SYMBOL: {
            /* Variable lookup */
            LispObject *value = env_lookup(env, expr);
            if (!value) {
                lisp_error("Unbound variable: %s", expr->symbol.name);
                result = make_nil();
            } else {
                result = value;
            }
            break;
        }

        case LISP_CONS: {
            /* Check for macro */
            LispObject *head = car(expr);
            if (is_symbol(head)) {
                LispObject *value = env_lookup(env, head);
                if (value && is_macro(value)) {
                    /* Expand macro and evaluate result */
                    LispObject *expanded = apply(value, cdr(expr), env);
                    result = eval(expanded, env);
                    break;
                }
            }

            /* Special forms */
            LispObject *special = eval_special_form(expr, env);
            if (special) {
                result = special;
                break;
            }

            /* Function application */
            result = eval_application(expr, env);
            break;
        }

        default:
            lisp_error("Cannot evaluate expression of type: %s",
                       lisp_type_name(expr->type));
            result = make_nil();
            break;
    }

    current_eval_depth--;
    return result;
}

/* Evaluate special forms */
static LispObject *eval_special_form(LispObject *expr, Environment *env) {
    if (!is_cons(expr)) return NULL;

    LispObject *head = car(expr);
    LispObject *args = cdr(expr);

    if (!is_symbol(head)) return NULL;
    const char *name = head->symbol.name;

    /* quote - return unevaluated */
    if (strcmp(name, "quote") == 0) {
        return car(args);
    }

    /* if - conditional */
    if (strcmp(name, "if") == 0) {
        LispObject *cond = eval(car(args), env);
        if (is_true(cond)) {
            return eval(cadr(args), env);
        } else if (is_cons(cddr(args))) {
            return eval(caddr(args), env);
        }
        return make_nil();
    }

    /* define - define variable or function */
    if (strcmp(name, "define") == 0) {
        LispObject *first = car(args);

        if (is_cons(first)) {
            /* (define (name params...) body...) */
            LispObject *fn_name = car(first);
            LispObject *params = cdr(first);
            LispObject *body = cdr(args);

            LispObject *lambda = make_lambda(params, body, env);
            if (is_symbol(fn_name)) {
                lambda->lambda.name = strdup(fn_name->symbol.name);
            }
            env_define(env, fn_name, lambda);
            return fn_name;
        } else {
            /* (define var value) */
            LispObject *value = eval(cadr(args), env);
            env_define(env, first, value);
            return first;
        }
    }

    /* set! - mutate variable */
    if (strcmp(name, "set!") == 0) {
        LispObject *var = car(args);
        LispObject *value = eval(cadr(args), env);

        if (!env_set(env, var, value)) {
            lisp_error("Cannot set undefined variable: %s", var->symbol.name);
        }
        return value;
    }

    /* lambda - create function */
    if (strcmp(name, "lambda") == 0) {
        LispObject *params = car(args);
        LispObject *body = cdr(args);
        return make_lambda(params, body, env);
    }

    /* begin - sequence */
    if (strcmp(name, "begin") == 0) {
        return eval_sequence(args, env);
    }

    /* let - local bindings */
    if (strcmp(name, "let") == 0) {
        LispObject *bindings = car(args);
        LispObject *body = cdr(args);

        /* Check for named let: (let name ((var val) ...) body...) */
        if (is_symbol(bindings)) {
            LispObject *loop_name = bindings;
            bindings = cadr(args);
            body = cddr(args);

            /* Create environment with loop function */
            Environment *let_env = env_create(env);

            /* Collect parameters and initial values */
            LispObject *params = make_nil();
            LispObject *vals = make_nil();
            LispObject *b = bindings;

            while (is_cons(b)) {
                LispObject *binding = car(b);
                params = make_cons(car(binding), params);
                vals = make_cons(eval(cadr(binding), env), vals);
                b = cdr(b);
            }

            params = list_reverse(params);
            vals = list_reverse(vals);

            /* Create the loop function */
            LispObject *loop_fn = make_lambda(params, body, let_env);
            loop_fn->lambda.name = strdup(loop_name->symbol.name);
            env_define(let_env, loop_name, loop_fn);

            /* Bind initial values */
            bind_parameters(let_env, params, vals);

            return eval_sequence(body, let_env);
        }

        /* Regular let */
        Environment *let_env = env_create(env);

        /* Evaluate bindings */
        while (is_cons(bindings)) {
            LispObject *binding = car(bindings);
            LispObject *var = car(binding);
            LispObject *val = eval(cadr(binding), env);  /* Evaluate in outer env */
            env_define(let_env, var, val);
            bindings = cdr(bindings);
        }

        return eval_sequence(body, let_env);
    }

    /* let* - sequential local bindings */
    if (strcmp(name, "let*") == 0) {
        LispObject *bindings = car(args);
        LispObject *body = cdr(args);

        Environment *let_env = env_create(env);

        /* Evaluate bindings sequentially */
        while (is_cons(bindings)) {
            LispObject *binding = car(bindings);
            LispObject *var = car(binding);
            LispObject *val = eval(cadr(binding), let_env);  /* Evaluate in current env */
            env_define(let_env, var, val);
            bindings = cdr(bindings);
        }

        return eval_sequence(body, let_env);
    }

    /* letrec - recursive local bindings */
    if (strcmp(name, "letrec") == 0) {
        LispObject *bindings = car(args);
        LispObject *body = cdr(args);

        Environment *let_env = env_create(env);

        /* First pass: bind all variables to undefined */
        LispObject *b = bindings;
        while (is_cons(b)) {
            LispObject *binding = car(b);
            env_define(let_env, car(binding), make_nil());
            b = cdr(b);
        }

        /* Second pass: evaluate and assign */
        b = bindings;
        while (is_cons(b)) {
            LispObject *binding = car(b);
            LispObject *var = car(binding);
            LispObject *val = eval(cadr(binding), let_env);
            env_set(let_env, var, val);
            b = cdr(b);
        }

        return eval_sequence(body, let_env);
    }

    /* cond - multi-way conditional */
    if (strcmp(name, "cond") == 0) {
        while (is_cons(args)) {
            LispObject *clause = car(args);
            LispObject *test = car(clause);

            /* else clause */
            if (is_symbol_named(test, "else")) {
                return eval_sequence(cdr(clause), env);
            }

            /* Regular clause */
            LispObject *result = eval(test, env);
            if (is_true(result)) {
                if (is_nil(cdr(clause))) {
                    return result;
                }
                /* Check for => syntax */
                if (is_symbol_named(cadr(clause), "=>")) {
                    LispObject *proc = eval(caddr(clause), env);
                    return apply(proc, make_cons(result, make_nil()), env);
                }
                return eval_sequence(cdr(clause), env);
            }

            args = cdr(args);
        }
        return make_nil();
    }

    /* and - short-circuit and */
    if (strcmp(name, "and") == 0) {
        LispObject *result = LISP_TRUE;

        while (is_cons(args)) {
            result = eval(car(args), env);
            if (is_false(result)) {
                return LISP_FALSE;
            }
            args = cdr(args);
        }

        return result;
    }

    /* or - short-circuit or */
    if (strcmp(name, "or") == 0) {
        while (is_cons(args)) {
            LispObject *result = eval(car(args), env);
            if (is_true(result)) {
                return result;
            }
            args = cdr(args);
        }

        return LISP_FALSE;
    }

    /* defmacro - define macro */
    if (strcmp(name, "defmacro") == 0) {
        LispObject *macro_name = car(args);
        LispObject *params = cadr(args);
        LispObject *body = cddr(args);

        LispObject *macro = make_macro(params, body, env);
        env_define(env, macro_name, macro);
        return macro_name;
    }

    /* quasiquote */
    if (strcmp(name, "quasiquote") == 0) {
        return expand_quasiquote(car(args), env, 1);
    }

    /* R7RS: when - execute body if test is true */
    if (strcmp(name, "when") == 0) {
        LispObject *test = eval(car(args), env);
        if (is_true(test)) {
            return eval_sequence(cdr(args), env);
        }
        return make_nil();
    }

    /* R7RS: unless - execute body if test is false */
    if (strcmp(name, "unless") == 0) {
        LispObject *test = eval(car(args), env);
        if (is_false(test)) {
            return eval_sequence(cdr(args), env);
        }
        return make_nil();
    }

    /* R7RS: case-lambda - multi-arity procedure */
    if (strcmp(name, "case-lambda") == 0) {
        /* Store clauses as a special lambda with case-lambda marker */
        LispObject *clauses = args;
        LispObject *case_fn = lisp_alloc();
        case_fn->type = LISP_LAMBDA;
        case_fn->lambda.params = make_symbol("case-lambda");  /* Marker */
        case_fn->lambda.body = clauses;
        case_fn->lambda.env = env;
        case_fn->lambda.name = NULL;
        return case_fn;
    }

    /* R7RS: do - iteration construct */
    if (strcmp(name, "do") == 0) {
        LispObject *bindings = car(args);
        LispObject *test_clause = cadr(args);
        LispObject *commands = cddr(args);

        /* Create environment for loop variables */
        Environment *do_env = env_create(env);

        /* Initialize loop variables */
        LispObject *b = bindings;
        while (is_cons(b)) {
            LispObject *binding = car(b);
            LispObject *var = car(binding);
            LispObject *init = eval(cadr(binding), env);
            env_define(do_env, var, init);
            b = cdr(b);
        }

        /* Loop */
        while (1) {
            /* Check termination condition */
            LispObject *test = eval(car(test_clause), do_env);
            if (is_true(test)) {
                /* Return result expression(s) */
                if (is_cons(cdr(test_clause))) {
                    return eval_sequence(cdr(test_clause), do_env);
                }
                return make_nil();
            }

            /* Execute commands */
            eval_sequence(commands, do_env);

            /* Update loop variables (collect new values first) */
            LispObject *new_vals = make_nil();
            b = bindings;
            while (is_cons(b)) {
                LispObject *binding = car(b);
                if (is_cons(cddr(binding))) {
                    /* Has step expression */
                    LispObject *step = eval(caddr(binding), do_env);
                    new_vals = make_cons(step, new_vals);
                } else {
                    /* No step, keep current value */
                    LispObject *var = car(binding);
                    new_vals = make_cons(env_lookup(do_env, var), new_vals);
                }
                b = cdr(b);
            }
            new_vals = list_reverse(new_vals);

            /* Assign new values */
            b = bindings;
            while (is_cons(b) && is_cons(new_vals)) {
                LispObject *binding = car(b);
                LispObject *var = car(binding);
                env_set(do_env, var, car(new_vals));
                b = cdr(b);
                new_vals = cdr(new_vals);
            }
        }
    }

    /* R7RS: let-values - bind multiple values */
    if (strcmp(name, "let-values") == 0) {
        LispObject *bindings = car(args);
        LispObject *body = cdr(args);
        Environment *let_env = env_create(env);

        while (is_cons(bindings)) {
            LispObject *binding = car(bindings);
            LispObject *formals = car(binding);
            LispObject *init = eval(cadr(binding), env);

            /* Bind formals to values */
            if (is_values(init)) {
                /* Multiple values */
                int i = 0;
                while (is_cons(formals) && i < init->values.count) {
                    env_define(let_env, car(formals), init->values.vals[i]);
                    formals = cdr(formals);
                    i++;
                }
            } else {
                /* Single value */
                if (is_cons(formals)) {
                    env_define(let_env, car(formals), init);
                }
            }

            bindings = cdr(bindings);
        }

        return eval_sequence(body, let_env);
    }

    /* R7RS: let*-values - sequential multiple value binding */
    if (strcmp(name, "let*-values") == 0) {
        LispObject *bindings = car(args);
        LispObject *body = cdr(args);
        Environment *let_env = env_create(env);

        while (is_cons(bindings)) {
            LispObject *binding = car(bindings);
            LispObject *formals = car(binding);
            LispObject *init = eval(cadr(binding), let_env);

            /* Bind formals to values */
            if (is_values(init)) {
                int i = 0;
                while (is_cons(formals) && i < init->values.count) {
                    env_define(let_env, car(formals), init->values.vals[i]);
                    formals = cdr(formals);
                    i++;
                }
            } else {
                if (is_cons(formals)) {
                    env_define(let_env, car(formals), init);
                }
            }

            bindings = cdr(bindings);
        }

        return eval_sequence(body, let_env);
    }

    /* R7RS: guard - exception handling */
    if (strcmp(name, "guard") == 0) {
        /* (guard (var clause ...) body ...) */
        /* Simplified: just evaluate body, no actual exception catching yet */
        LispObject *var_clauses = car(args);
        LispObject *body = cdr(args);
        (void)var_clauses;  /* Would be used for exception handling */

        /* For now, just evaluate body - full implementation needs setjmp/longjmp */
        return eval_sequence(body, env);
    }

    /* R7RS: case - case dispatch */
    if (strcmp(name, "case") == 0) {
        LispObject *key = eval(car(args), env);
        LispObject *clauses = cdr(args);

        while (is_cons(clauses)) {
            LispObject *clause = car(clauses);
            LispObject *datums = car(clause);
            LispObject *exprs = cdr(clause);

            /* else clause */
            if (is_symbol_named(datums, "else")) {
                return eval_sequence(exprs, env);
            }

            /* Check if key matches any datum */
            while (is_cons(datums)) {
                if (lisp_equal(key, car(datums))) {
                    /* Check for => syntax */
                    if (is_cons(exprs) && is_symbol_named(car(exprs), "=>")) {
                        LispObject *proc = eval(cadr(exprs), env);
                        return apply(proc, make_cons(key, make_nil()), env);
                    }
                    return eval_sequence(exprs, env);
                }
                datums = cdr(datums);
            }

            clauses = cdr(clauses);
        }

        return make_nil();
    }

    /* Not a special form */
    return NULL;
}

/* Expand quasiquote */
LispObject *expand_quasiquote(LispObject *expr, Environment *env, int depth) {
    if (!is_cons(expr)) {
        return expr;
    }

    LispObject *head = car(expr);

    /* unquote */
    if (is_symbol_named(head, "unquote")) {
        if (depth == 1) {
            return eval(cadr(expr), env);
        } else {
            return make_cons(head,
                            make_cons(expand_quasiquote(cadr(expr), env, depth - 1),
                                     make_nil()));
        }
    }

    /* unquote-splicing - handled at list level */
    if (is_symbol_named(head, "unquote-splicing")) {
        lisp_error("unquote-splicing not in list context");
        return expr;
    }

    /* quasiquote - nested */
    if (is_symbol_named(head, "quasiquote")) {
        return make_cons(head,
                        make_cons(expand_quasiquote(cadr(expr), env, depth + 1),
                                 make_nil()));
    }

    /* Regular list - expand each element */
    LispObject *result_head = NULL;
    LispObject *result_tail = NULL;

    while (is_cons(expr)) {
        LispObject *item = car(expr);

        /* Check for unquote-splicing */
        if (is_cons(item) && is_symbol_named(car(item), "unquote-splicing")) {
            if (depth == 1) {
                /* Splice the result */
                LispObject *spliced = eval(cadr(item), env);
                while (is_cons(spliced)) {
                    LispObject *new_cell = make_cons(car(spliced), make_nil());
                    if (result_tail) {
                        result_tail->cons.cdr = new_cell;
                    } else {
                        result_head = new_cell;
                    }
                    result_tail = new_cell;
                    spliced = cdr(spliced);
                }
            } else {
                LispObject *expanded = make_cons(
                    car(item),
                    make_cons(expand_quasiquote(cadr(item), env, depth - 1),
                             make_nil()));
                LispObject *new_cell = make_cons(expanded, make_nil());
                if (result_tail) {
                    result_tail->cons.cdr = new_cell;
                } else {
                    result_head = new_cell;
                }
                result_tail = new_cell;
            }
        } else {
            /* Regular element */
            LispObject *expanded = expand_quasiquote(item, env, depth);
            LispObject *new_cell = make_cons(expanded, make_nil());
            if (result_tail) {
                result_tail->cons.cdr = new_cell;
            } else {
                result_head = new_cell;
            }
            result_tail = new_cell;
        }

        expr = cdr(expr);
    }

    /* Handle dotted list */
    if (!is_nil(expr)) {
        if (result_tail) {
            result_tail->cons.cdr = expand_quasiquote(expr, env, depth);
        }
    }

    return result_head ? result_head : make_nil();
}

/* Evaluate function application */
static LispObject *eval_application(LispObject *expr, Environment *env) {
    /* Evaluate the function */
    LispObject *func = eval(car(expr), env);
    gc_add_root(&func);  /* Protect from GC while evaluating arguments */

    /* Evaluate arguments */
    LispObject *args = eval_list(cdr(expr), env);
    gc_add_root(&args);  /* Protect from GC during apply */

    /* Apply function */
    LispObject *result = apply(func, args, env);

    gc_remove_root(&args);
    gc_remove_root(&func);

    return result;
}

/* Evaluate all items in a list */
LispObject *eval_list(LispObject *list, Environment *env) {
    if (is_nil(list)) return make_nil();

    LispObject *head = NULL;
    LispObject *tail = NULL;
    gc_add_root(&head);  /* Protect result list from GC */

    while (is_cons(list)) {
        LispObject *value = eval(car(list), env);
        gc_add_root(&value);  /* Protect value while creating cons */
        LispObject *new_cell = make_cons(value, make_nil());
        gc_remove_root(&value);

        if (tail) {
            tail->cons.cdr = new_cell;
            tail = new_cell;
        } else {
            head = tail = new_cell;
        }

        list = cdr(list);
    }

    gc_remove_root(&head);
    return head ? head : make_nil();
}

/* Apply a function to arguments */
LispObject *apply(LispObject *func, LispObject *args, Environment *env) {
    if (is_primitive(func)) {
        /* Check argument count */
        int argc = list_length(args);
        if (argc < func->primitive.min_args) {
            lisp_error("%s: too few arguments (expected at least %d, got %d)",
                       func->primitive.name, func->primitive.min_args, argc);
            return make_nil();
        }
        if (func->primitive.max_args >= 0 && argc > func->primitive.max_args) {
            lisp_error("%s: too many arguments (expected at most %d, got %d)",
                       func->primitive.name, func->primitive.max_args, argc);
            return make_nil();
        }

        return func->primitive.func(args);
    }

    if (is_lambda(func)) {
        /* Check for case-lambda (multi-arity procedure) */
        if (is_symbol(func->lambda.params) &&
            strcmp(func->lambda.params->symbol.name, "case-lambda") == 0) {
            /* Find matching clause based on argument count */
            int argc = list_length(args);
            LispObject *clauses = func->lambda.body;

            while (is_cons(clauses)) {
                LispObject *clause = car(clauses);
                LispObject *params = car(clause);
                LispObject *body = cdr(clause);

                /* Count required parameters */
                int param_count = 0;
                int has_rest = 0;
                LispObject *p = params;
                while (is_cons(p)) {
                    param_count++;
                    p = cdr(p);
                }
                if (is_symbol(p) && !is_nil(p)) {
                    has_rest = 1;  /* Rest parameter */
                }

                /* Check if this clause matches */
                if ((has_rest && argc >= param_count) ||
                    (!has_rest && argc == param_count)) {
                    Environment *call_env = env_create(func->lambda.env);
                    bind_parameters(call_env, params, args);

                    /* Debug: push call frame */
                    if (debug_is_enabled()) {
                        debug_push_frame("case-lambda", args, call_env, NULL);
                    }

                    LispObject *result = eval_sequence(body, call_env);

                    /* Debug: pop call frame */
                    if (debug_is_enabled()) {
                        debug_pop_frame();
                    }

                    return result;
                }

                clauses = cdr(clauses);
            }

            lisp_error("case-lambda: no matching clause for %d arguments", argc);
            return make_nil();
        }

        /* Regular lambda */
        /* Create new environment extending the closure's environment */
        Environment *call_env = env_create(func->lambda.env);

        /* Bind parameters to arguments */
        bind_parameters(call_env, func->lambda.params, args);

        /* Debug: push call frame */
        if (debug_is_enabled()) {
            const char *fn_name = func->lambda.name ? func->lambda.name : "<lambda>";
            debug_push_frame(fn_name, args, call_env, NULL);
        }

        /* Evaluate body */
        LispObject *result = eval_sequence(func->lambda.body, call_env);

        /* Debug: pop call frame */
        if (debug_is_enabled()) {
            debug_pop_frame();
        }

        /* Note: In a real implementation, we'd need GC to clean up call_env */

        return result;
    }

    if (is_macro(func)) {
        /* Macros receive unevaluated arguments */
        Environment *macro_env = env_create(func->macro.env);
        bind_parameters(macro_env, func->macro.params, args);
        return eval_sequence(func->macro.body, macro_env);
    }

    lisp_error("Not a function: %s", lisp_type_name(func->type));
    return make_nil();
}

/* Expand macros (full recursive expansion) */
LispObject *expand_macros(LispObject *expr, Environment *env) {
    if (!is_cons(expr)) {
        return expr;
    }

    LispObject *head = car(expr);

    /* Check if it's a macro call */
    if (is_symbol(head)) {
        LispObject *value = env_lookup(env, head);
        if (value && is_macro(value)) {
            LispObject *expanded = apply(value, cdr(expr), env);
            return expand_macros(expanded, env);
        }
    }

    /* Skip special forms that don't evaluate their arguments */
    if (is_symbol_named(head, "quote")) {
        return expr;
    }

    /* Recursively expand subforms */
    LispObject *result_head = NULL;
    LispObject *result_tail = NULL;

    while (is_cons(expr)) {
        LispObject *expanded = expand_macros(car(expr), env);
        LispObject *new_cell = make_cons(expanded, make_nil());

        if (result_tail) {
            result_tail->cons.cdr = new_cell;
            result_tail = new_cell;
        } else {
            result_head = result_tail = new_cell;
        }

        expr = cdr(expr);
    }

    return result_head ? result_head : make_nil();
}
