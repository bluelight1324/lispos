/*
 * primitives.c - Built-in Lisp Functions Implementation
 */

#include "primitives.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Helper to get required argument */
static LispObject *require_arg(LispObject *args, int n, const char *func_name) {
    for (int i = 0; i < n; i++) {
        if (!is_cons(args)) {
            lisp_error("%s: missing argument %d", func_name, i + 1);
            return NULL;
        }
        args = cdr(args);
    }
    if (!is_cons(args)) {
        lisp_error("%s: missing argument %d", func_name, n + 1);
        return NULL;
    }
    return car(args);
}

/* Helper to require a specific type */
static int require_type(LispObject *obj, LispType type, const char *func_name) {
    if (obj->type != type) {
        lisp_error("%s: expected %s, got %s",
                   func_name, lisp_type_name(type), lisp_type_name(obj->type));
        return 0;
    }
    return 1;
}

/* List operations */

LispObject *prim_car(LispObject *args) {
    LispObject *pair = require_arg(args, 0, "car");
    if (!pair) return make_nil();
    if (!require_type(pair, LISP_CONS, "car")) return make_nil();
    return car(pair);
}

LispObject *prim_cdr(LispObject *args) {
    LispObject *pair = require_arg(args, 0, "cdr");
    if (!pair) return make_nil();
    if (!require_type(pair, LISP_CONS, "cdr")) return make_nil();
    return cdr(pair);
}

LispObject *prim_cons(LispObject *args) {
    LispObject *a = require_arg(args, 0, "cons");
    LispObject *b = require_arg(args, 1, "cons");
    if (!a || !b) return make_nil();
    return make_cons(a, b);
}

LispObject *prim_list(LispObject *args) {
    return args;  /* Arguments are already a list */
}

LispObject *prim_length(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "length");
    if (!lst) return make_number(0);
    return make_number(list_length(lst));
}

LispObject *prim_append(LispObject *args) {
    LispObject *result = make_nil();

    while (is_cons(args)) {
        LispObject *lst = car(args);
        result = list_append(result, lst);
        args = cdr(args);
    }

    return result;
}

LispObject *prim_reverse(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "reverse");
    if (!lst) return make_nil();
    return list_reverse(lst);
}

/* Type predicates */

LispObject *prim_null_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "null?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_nil(obj));
}

LispObject *prim_pair_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "pair?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_cons(obj));
}

LispObject *prim_number_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "number?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_number(obj));
}

LispObject *prim_symbol_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "symbol?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_symbol(obj));
}

LispObject *prim_string_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "string?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_string(obj));
}

LispObject *prim_procedure_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "procedure?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_callable(obj));
}

LispObject *prim_boolean_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "boolean?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_boolean(obj));
}

/* Arithmetic */

LispObject *prim_add(LispObject *args) {
    double sum = 0;

    while (is_cons(args)) {
        LispObject *n = car(args);
        if (!is_number(n)) {
            lisp_error("+: expected number, got %s", lisp_type_name(n->type));
            return make_number(0);
        }
        sum += n->number;
        args = cdr(args);
    }

    return make_number(sum);
}

LispObject *prim_sub(LispObject *args) {
    if (!is_cons(args)) {
        lisp_error("-: requires at least one argument");
        return make_number(0);
    }

    LispObject *first = car(args);
    if (!is_number(first)) {
        lisp_error("-: expected number, got %s", lisp_type_name(first->type));
        return make_number(0);
    }

    /* Unary minus */
    if (!is_cons(cdr(args))) {
        return make_number(-first->number);
    }

    /* Binary/multi subtraction */
    double result = first->number;
    args = cdr(args);

    while (is_cons(args)) {
        LispObject *n = car(args);
        if (!is_number(n)) {
            lisp_error("-: expected number, got %s", lisp_type_name(n->type));
            return make_number(0);
        }
        result -= n->number;
        args = cdr(args);
    }

    return make_number(result);
}

LispObject *prim_mul(LispObject *args) {
    double product = 1;

    while (is_cons(args)) {
        LispObject *n = car(args);
        if (!is_number(n)) {
            lisp_error("*: expected number, got %s", lisp_type_name(n->type));
            return make_number(0);
        }
        product *= n->number;
        args = cdr(args);
    }

    return make_number(product);
}

LispObject *prim_div(LispObject *args) {
    LispObject *a = require_arg(args, 0, "/");
    LispObject *b = require_arg(args, 1, "/");
    if (!a || !b) return make_number(0);

    if (!is_number(a) || !is_number(b)) {
        lisp_error("/: expected numbers");
        return make_number(0);
    }

    if (b->number == 0) {
        lisp_error("/: division by zero");
        return make_number(0);
    }

    return make_number(a->number / b->number);
}

LispObject *prim_mod(LispObject *args) {
    LispObject *a = require_arg(args, 0, "mod");
    LispObject *b = require_arg(args, 1, "mod");
    if (!a || !b) return make_number(0);

    if (!is_number(a) || !is_number(b)) {
        lisp_error("mod: expected numbers");
        return make_number(0);
    }

    if (b->number == 0) {
        lisp_error("mod: division by zero");
        return make_number(0);
    }

    return make_number(fmod(a->number, b->number));
}

LispObject *prim_abs(LispObject *args) {
    LispObject *n = require_arg(args, 0, "abs");
    if (!n) return make_number(0);

    if (!is_number(n)) {
        lisp_error("abs: expected number, got %s", lisp_type_name(n->type));
        return make_number(0);
    }

    return make_number(fabs(n->number));
}

/* Comparison */

LispObject *prim_eq_num(LispObject *args) {
    LispObject *a = require_arg(args, 0, "=");
    LispObject *b = require_arg(args, 1, "=");
    if (!a || !b) return LISP_FALSE;

    if (!is_number(a) || !is_number(b)) {
        lisp_error("=: expected numbers");
        return LISP_FALSE;
    }

    return make_boolean(a->number == b->number);
}

LispObject *prim_lt(LispObject *args) {
    LispObject *a = require_arg(args, 0, "<");
    LispObject *b = require_arg(args, 1, "<");
    if (!a || !b) return LISP_FALSE;

    if (!is_number(a) || !is_number(b)) {
        lisp_error("<: expected numbers");
        return LISP_FALSE;
    }

    return make_boolean(a->number < b->number);
}

LispObject *prim_gt(LispObject *args) {
    LispObject *a = require_arg(args, 0, ">");
    LispObject *b = require_arg(args, 1, ">");
    if (!a || !b) return LISP_FALSE;

    if (!is_number(a) || !is_number(b)) {
        lisp_error(">: expected numbers");
        return LISP_FALSE;
    }

    return make_boolean(a->number > b->number);
}

LispObject *prim_le(LispObject *args) {
    LispObject *a = require_arg(args, 0, "<=");
    LispObject *b = require_arg(args, 1, "<=");
    if (!a || !b) return LISP_FALSE;

    if (!is_number(a) || !is_number(b)) {
        lisp_error("<=: expected numbers");
        return LISP_FALSE;
    }

    return make_boolean(a->number <= b->number);
}

LispObject *prim_ge(LispObject *args) {
    LispObject *a = require_arg(args, 0, ">=");
    LispObject *b = require_arg(args, 1, ">=");
    if (!a || !b) return LISP_FALSE;

    if (!is_number(a) || !is_number(b)) {
        lisp_error(">=: expected numbers");
        return LISP_FALSE;
    }

    return make_boolean(a->number >= b->number);
}

LispObject *prim_eq(LispObject *args) {
    LispObject *a = require_arg(args, 0, "eq?");
    LispObject *b = require_arg(args, 1, "eq?");
    if (!a || !b) return LISP_FALSE;

    return make_boolean(lisp_eq(a, b));
}

LispObject *prim_equal(LispObject *args) {
    LispObject *a = require_arg(args, 0, "equal?");
    LispObject *b = require_arg(args, 1, "equal?");
    if (!a || !b) return LISP_FALSE;

    return make_boolean(lisp_equal(a, b));
}

/* Boolean */

LispObject *prim_not(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "not");
    if (!obj) return LISP_TRUE;

    return make_boolean(is_false(obj));
}

/* I/O */

LispObject *prim_display(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "display");
    if (!obj) return make_nil();

    /* Print without quotes for strings */
    if (is_string(obj)) {
        printf("%s", obj->string.data);
    } else {
        lisp_print(obj);
    }

    return make_nil();
}

LispObject *prim_newline(LispObject *args) {
    (void)args;
    printf("\n");
    return make_nil();
}

LispObject *prim_print(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "print");
    if (!obj) return make_nil();

    lisp_print(obj);
    printf("\n");

    return obj;
}

/* String operations */

LispObject *prim_string_length(LispObject *args) {
    LispObject *s = require_arg(args, 0, "string-length");
    if (!s) return make_number(0);

    if (!is_string(s)) {
        lisp_error("string-length: expected string");
        return make_number(0);
    }

    return make_number((double)s->string.length);
}

LispObject *prim_string_append(LispObject *args) {
    /* Calculate total length */
    size_t total_len = 0;
    LispObject *a = args;
    while (is_cons(a)) {
        LispObject *s = car(a);
        if (!is_string(s)) {
            lisp_error("string-append: expected string");
            return make_string("");
        }
        total_len += s->string.length;
        a = cdr(a);
    }

    /* Allocate and concatenate */
    char *buffer = (char *)malloc(total_len + 1);
    char *p = buffer;

    a = args;
    while (is_cons(a)) {
        LispObject *s = car(a);
        memcpy(p, s->string.data, s->string.length);
        p += s->string.length;
        a = cdr(a);
    }
    *p = '\0';

    LispObject *result = make_string_n(buffer, total_len);
    free(buffer);
    return result;
}

LispObject *prim_string_ref(LispObject *args) {
    LispObject *s = require_arg(args, 0, "string-ref");
    LispObject *idx = require_arg(args, 1, "string-ref");
    if (!s || !idx) return make_character('\0');

    if (!is_string(s)) {
        lisp_error("string-ref: expected string");
        return make_character('\0');
    }
    if (!is_number(idx)) {
        lisp_error("string-ref: expected number for index");
        return make_character('\0');
    }

    int i = (int)idx->number;
    if (i < 0 || (size_t)i >= s->string.length) {
        lisp_error("string-ref: index out of bounds");
        return make_character('\0');
    }

    return make_character(s->string.data[i]);
}

LispObject *prim_number_to_string(LispObject *args) {
    LispObject *n = require_arg(args, 0, "number->string");
    if (!n) return make_string("0");

    if (!is_number(n)) {
        lisp_error("number->string: expected number");
        return make_string("0");
    }

    char buffer[64];
    if (n->number == (long long)n->number) {
        snprintf(buffer, sizeof(buffer), "%lld", (long long)n->number);
    } else {
        snprintf(buffer, sizeof(buffer), "%g", n->number);
    }

    return make_string(buffer);
}

LispObject *prim_string_to_number(LispObject *args) {
    LispObject *s = require_arg(args, 0, "string->number");
    if (!s) return LISP_FALSE;

    if (!is_string(s)) {
        lisp_error("string->number: expected string");
        return LISP_FALSE;
    }

    char *endptr;
    double value = strtod(s->string.data, &endptr);

    if (endptr == s->string.data) {
        return LISP_FALSE;  /* No conversion */
    }

    return make_number(value);
}

LispObject *prim_symbol_to_string(LispObject *args) {
    LispObject *sym = require_arg(args, 0, "symbol->string");
    if (!sym) return make_string("");

    if (!is_symbol(sym)) {
        lisp_error("symbol->string: expected symbol");
        return make_string("");
    }

    return make_string(sym->symbol.name);
}

LispObject *prim_string_to_symbol(LispObject *args) {
    LispObject *s = require_arg(args, 0, "string->symbol");
    if (!s) return make_symbol("");

    if (!is_string(s)) {
        lisp_error("string->symbol: expected string");
        return make_symbol("");
    }

    return make_symbol(s->string.data);
}

/* Utility */

LispObject *prim_apply(LispObject *args) {
    LispObject *func = require_arg(args, 0, "apply");
    LispObject *arg_list = require_arg(args, 1, "apply");
    if (!func || !arg_list) return make_nil();

    return apply(func, arg_list, NULL);
}

LispObject *prim_error(LispObject *args) {
    if (is_cons(args)) {
        LispObject *msg = car(args);
        if (is_string(msg)) {
            lisp_error("%s", msg->string.data);
        } else {
            lisp_error("User error");
            lisp_print(msg);
            printf("\n");
        }
    } else {
        lisp_error("User error");
    }

    return make_nil();
}

/* ============================================================
 * R6RS: Vector Primitives
 * ============================================================ */

LispObject *prim_vector_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "vector?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_vector(obj));
}

LispObject *prim_make_vector(LispObject *args) {
    LispObject *len_obj = require_arg(args, 0, "make-vector");
    if (!len_obj) return make_nil();

    if (!is_number(len_obj)) {
        lisp_error("make-vector: expected number for length");
        return make_nil();
    }

    size_t len = (size_t)len_obj->number;
    LispObject *fill = LISP_NIL_OBJ;

    /* Optional fill argument */
    if (is_cons(cdr(args))) {
        fill = cadr(args);
    }

    return make_vector(len, fill);
}

LispObject *prim_vector(LispObject *args) {
    return make_vector_from_list(args);
}

LispObject *prim_vector_length(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector-length");
    if (!vec) return make_number(0);

    if (!is_vector(vec)) {
        lisp_error("vector-length: expected vector");
        return make_number(0);
    }

    return make_number((double)vector_length(vec));
}

LispObject *prim_vector_ref(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector-ref");
    LispObject *idx = require_arg(args, 1, "vector-ref");
    if (!vec || !idx) return make_nil();

    if (!is_vector(vec)) {
        lisp_error("vector-ref: expected vector");
        return make_nil();
    }
    if (!is_number(idx)) {
        lisp_error("vector-ref: expected number for index");
        return make_nil();
    }

    return vector_ref(vec, (size_t)idx->number);
}

LispObject *prim_vector_set(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector-set!");
    LispObject *idx = require_arg(args, 1, "vector-set!");
    LispObject *val = require_arg(args, 2, "vector-set!");
    if (!vec || !idx || !val) return make_nil();

    if (!is_vector(vec)) {
        lisp_error("vector-set!: expected vector");
        return make_nil();
    }
    if (!is_number(idx)) {
        lisp_error("vector-set!: expected number for index");
        return make_nil();
    }

    vector_set(vec, (size_t)idx->number, val);
    return make_nil();
}

LispObject *prim_vector_to_list(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector->list");
    if (!vec) return make_nil();

    if (!is_vector(vec)) {
        lisp_error("vector->list: expected vector");
        return make_nil();
    }

    return vector_to_list(vec);
}

LispObject *prim_list_to_vector(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "list->vector");
    if (!lst) return make_vector(0, make_nil());

    return make_vector_from_list(lst);
}

/* ============================================================
 * R6RS: Bytevector Primitives
 * ============================================================ */

LispObject *prim_bytevector_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "bytevector?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_bytevector(obj));
}

LispObject *prim_make_bytevector(LispObject *args) {
    LispObject *len_obj = require_arg(args, 0, "make-bytevector");
    if (!len_obj) return make_nil();

    if (!is_number(len_obj)) {
        lisp_error("make-bytevector: expected number for length");
        return make_nil();
    }

    size_t len = (size_t)len_obj->number;
    uint8_t fill = 0;

    /* Optional fill argument */
    if (is_cons(cdr(args))) {
        LispObject *fill_obj = cadr(args);
        if (is_number(fill_obj)) {
            fill = (uint8_t)fill_obj->number;
        }
    }

    return make_bytevector(len, fill);
}

LispObject *prim_bytevector_length(LispObject *args) {
    LispObject *bv = require_arg(args, 0, "bytevector-length");
    if (!bv) return make_number(0);

    if (!is_bytevector(bv)) {
        lisp_error("bytevector-length: expected bytevector");
        return make_number(0);
    }

    return make_number((double)bytevector_length(bv));
}

LispObject *prim_bytevector_u8_ref(LispObject *args) {
    LispObject *bv = require_arg(args, 0, "bytevector-u8-ref");
    LispObject *idx = require_arg(args, 1, "bytevector-u8-ref");
    if (!bv || !idx) return make_number(0);

    if (!is_bytevector(bv)) {
        lisp_error("bytevector-u8-ref: expected bytevector");
        return make_number(0);
    }
    if (!is_number(idx)) {
        lisp_error("bytevector-u8-ref: expected number for index");
        return make_number(0);
    }

    return make_number((double)bytevector_ref(bv, (size_t)idx->number));
}

LispObject *prim_bytevector_u8_set(LispObject *args) {
    LispObject *bv = require_arg(args, 0, "bytevector-u8-set!");
    LispObject *idx = require_arg(args, 1, "bytevector-u8-set!");
    LispObject *val = require_arg(args, 2, "bytevector-u8-set!");
    if (!bv || !idx || !val) return make_nil();

    if (!is_bytevector(bv)) {
        lisp_error("bytevector-u8-set!: expected bytevector");
        return make_nil();
    }
    if (!is_number(idx)) {
        lisp_error("bytevector-u8-set!: expected number for index");
        return make_nil();
    }
    if (!is_number(val)) {
        lisp_error("bytevector-u8-set!: expected number for value");
        return make_nil();
    }

    bytevector_set(bv, (size_t)idx->number, (uint8_t)val->number);
    return make_nil();
}

/* ============================================================
 * R6RS: Hashtable Primitives
 * ============================================================ */

LispObject *prim_hashtable_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "hashtable?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_hashtable(obj));
}

LispObject *prim_make_eq_hashtable(LispObject *args) {
    (void)args;
    return make_hashtable(0, 16);  /* eq hash */
}

LispObject *prim_make_eqv_hashtable(LispObject *args) {
    (void)args;
    return make_hashtable(1, 16);  /* eqv hash */
}

LispObject *prim_make_hashtable(LispObject *args) {
    /* (make-hashtable hash-fn equiv-fn) - simplified, we use equal hash */
    (void)args;
    return make_hashtable(2, 16);  /* equal hash */
}

LispObject *prim_hashtable_ref(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-ref");
    LispObject *key = require_arg(args, 1, "hashtable-ref");
    LispObject *default_val = require_arg(args, 2, "hashtable-ref");
    if (!ht || !key || !default_val) return make_nil();

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-ref: expected hashtable");
        return make_nil();
    }

    return hashtable_ref(ht, key, default_val);
}

LispObject *prim_hashtable_set(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-set!");
    LispObject *key = require_arg(args, 1, "hashtable-set!");
    LispObject *val = require_arg(args, 2, "hashtable-set!");
    if (!ht || !key || !val) return make_nil();

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-set!: expected hashtable");
        return make_nil();
    }

    hashtable_set(ht, key, val);
    return make_nil();
}

LispObject *prim_hashtable_delete(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-delete!");
    LispObject *key = require_arg(args, 1, "hashtable-delete!");
    if (!ht || !key) return make_nil();

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-delete!: expected hashtable");
        return make_nil();
    }

    hashtable_delete(ht, key);
    return make_nil();
}

LispObject *prim_hashtable_contains(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-contains?");
    LispObject *key = require_arg(args, 1, "hashtable-contains?");
    if (!ht || !key) return LISP_FALSE;

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-contains?: expected hashtable");
        return LISP_FALSE;
    }

    return make_boolean(hashtable_contains(ht, key));
}

LispObject *prim_hashtable_size(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-size");
    if (!ht) return make_number(0);

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-size: expected hashtable");
        return make_number(0);
    }

    return make_number((double)hashtable_size(ht));
}

LispObject *prim_hashtable_keys(LispObject *args) {
    LispObject *ht = require_arg(args, 0, "hashtable-keys");
    if (!ht) return make_nil();

    if (!is_hashtable(ht)) {
        lisp_error("hashtable-keys: expected hashtable");
        return make_nil();
    }

    return hashtable_keys(ht);
}

/* ============================================================
 * R6RS: Additional Numeric Primitives
 * ============================================================ */

LispObject *prim_floor(LispObject *args) {
    LispObject *n = require_arg(args, 0, "floor");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("floor: expected number");
        return make_number(0);
    }
    return make_number(floor(n->number));
}

LispObject *prim_ceiling(LispObject *args) {
    LispObject *n = require_arg(args, 0, "ceiling");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("ceiling: expected number");
        return make_number(0);
    }
    return make_number(ceil(n->number));
}

LispObject *prim_truncate(LispObject *args) {
    LispObject *n = require_arg(args, 0, "truncate");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("truncate: expected number");
        return make_number(0);
    }
    return make_number(trunc(n->number));
}

LispObject *prim_round(LispObject *args) {
    LispObject *n = require_arg(args, 0, "round");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("round: expected number");
        return make_number(0);
    }
    return make_number(round(n->number));
}

LispObject *prim_sqrt(LispObject *args) {
    LispObject *n = require_arg(args, 0, "sqrt");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("sqrt: expected number");
        return make_number(0);
    }
    return make_number(sqrt(n->number));
}

LispObject *prim_expt(LispObject *args) {
    LispObject *base = require_arg(args, 0, "expt");
    LispObject *exp = require_arg(args, 1, "expt");
    if (!base || !exp) return make_number(0);
    if (!is_number(base) || !is_number(exp)) {
        lisp_error("expt: expected numbers");
        return make_number(0);
    }
    return make_number(pow(base->number, exp->number));
}

LispObject *prim_log(LispObject *args) {
    LispObject *n = require_arg(args, 0, "log");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("log: expected number");
        return make_number(0);
    }
    return make_number(log(n->number));
}

LispObject *prim_sin(LispObject *args) {
    LispObject *n = require_arg(args, 0, "sin");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("sin: expected number");
        return make_number(0);
    }
    return make_number(sin(n->number));
}

LispObject *prim_cos(LispObject *args) {
    LispObject *n = require_arg(args, 0, "cos");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("cos: expected number");
        return make_number(0);
    }
    return make_number(cos(n->number));
}

LispObject *prim_tan(LispObject *args) {
    LispObject *n = require_arg(args, 0, "tan");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("tan: expected number");
        return make_number(0);
    }
    return make_number(tan(n->number));
}

LispObject *prim_quotient(LispObject *args) {
    LispObject *a = require_arg(args, 0, "quotient");
    LispObject *b = require_arg(args, 1, "quotient");
    if (!a || !b) return make_number(0);
    if (!is_number(a) || !is_number(b)) {
        lisp_error("quotient: expected numbers");
        return make_number(0);
    }
    if (b->number == 0) {
        lisp_error("quotient: division by zero");
        return make_number(0);
    }
    return make_number(trunc(a->number / b->number));
}

LispObject *prim_remainder(LispObject *args) {
    LispObject *a = require_arg(args, 0, "remainder");
    LispObject *b = require_arg(args, 1, "remainder");
    if (!a || !b) return make_number(0);
    if (!is_number(a) || !is_number(b)) {
        lisp_error("remainder: expected numbers");
        return make_number(0);
    }
    if (b->number == 0) {
        lisp_error("remainder: division by zero");
        return make_number(0);
    }
    return make_number(fmod(a->number, b->number));
}

LispObject *prim_modulo(LispObject *args) {
    LispObject *a = require_arg(args, 0, "modulo");
    LispObject *b = require_arg(args, 1, "modulo");
    if (!a || !b) return make_number(0);
    if (!is_number(a) || !is_number(b)) {
        lisp_error("modulo: expected numbers");
        return make_number(0);
    }
    if (b->number == 0) {
        lisp_error("modulo: division by zero");
        return make_number(0);
    }
    double r = fmod(a->number, b->number);
    /* Ensure result has same sign as divisor */
    if ((r < 0 && b->number > 0) || (r > 0 && b->number < 0)) {
        r += b->number;
    }
    return make_number(r);
}

LispObject *prim_integer_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "integer?");
    if (!obj) return LISP_FALSE;
    if (!is_number(obj)) return LISP_FALSE;
    return make_boolean(obj->number == floor(obj->number));
}

LispObject *prim_real_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "real?");
    if (!obj) return LISP_FALSE;
    return make_boolean(is_number(obj));
}

LispObject *prim_zero_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "zero?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("zero?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(n->number == 0);
}

LispObject *prim_positive_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "positive?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("positive?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(n->number > 0);
}

LispObject *prim_negative_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "negative?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("negative?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(n->number < 0);
}

LispObject *prim_odd_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "odd?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("odd?: expected number");
        return LISP_FALSE;
    }
    return make_boolean((long long)n->number % 2 != 0);
}

LispObject *prim_even_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "even?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("even?: expected number");
        return LISP_FALSE;
    }
    return make_boolean((long long)n->number % 2 == 0);
}

LispObject *prim_min(LispObject *args) {
    if (!is_cons(args)) {
        lisp_error("min: requires at least one argument");
        return make_number(0);
    }
    LispObject *first = car(args);
    if (!is_number(first)) {
        lisp_error("min: expected number");
        return make_number(0);
    }
    double result = first->number;
    args = cdr(args);
    while (is_cons(args)) {
        LispObject *n = car(args);
        if (!is_number(n)) {
            lisp_error("min: expected number");
            return make_number(0);
        }
        if (n->number < result) result = n->number;
        args = cdr(args);
    }
    return make_number(result);
}

LispObject *prim_max(LispObject *args) {
    if (!is_cons(args)) {
        lisp_error("max: requires at least one argument");
        return make_number(0);
    }
    LispObject *first = car(args);
    if (!is_number(first)) {
        lisp_error("max: expected number");
        return make_number(0);
    }
    double result = first->number;
    args = cdr(args);
    while (is_cons(args)) {
        LispObject *n = car(args);
        if (!is_number(n)) {
            lisp_error("max: expected number");
            return make_number(0);
        }
        if (n->number > result) result = n->number;
        args = cdr(args);
    }
    return make_number(result);
}

/* ============================================================
 * R6RS: Additional List Primitives
 * ============================================================ */

LispObject *prim_list_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "list?");
    if (!obj) return LISP_FALSE;
    /* Check for proper list (ends in nil) */
    while (is_cons(obj)) {
        obj = cdr(obj);
    }
    return make_boolean(is_nil(obj));
}

LispObject *prim_list_ref(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "list-ref");
    LispObject *idx = require_arg(args, 1, "list-ref");
    if (!lst || !idx) return make_nil();
    if (!is_number(idx)) {
        lisp_error("list-ref: expected number for index");
        return make_nil();
    }
    return list_nth(lst, (int)idx->number);
}

LispObject *prim_list_tail(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "list-tail");
    LispObject *idx = require_arg(args, 1, "list-tail");
    if (!lst || !idx) return make_nil();
    if (!is_number(idx)) {
        lisp_error("list-tail: expected number for index");
        return make_nil();
    }
    int n = (int)idx->number;
    while (n > 0 && is_cons(lst)) {
        lst = cdr(lst);
        n--;
    }
    return lst;
}

LispObject *prim_memq(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "memq");
    LispObject *lst = require_arg(args, 1, "memq");
    if (!obj || !lst) return LISP_FALSE;
    while (is_cons(lst)) {
        if (lisp_eq(obj, car(lst))) return lst;
        lst = cdr(lst);
    }
    return LISP_FALSE;
}

LispObject *prim_memv(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "memv");
    LispObject *lst = require_arg(args, 1, "memv");
    if (!obj || !lst) return LISP_FALSE;
    while (is_cons(lst)) {
        LispObject *item = car(lst);
        if (lisp_eq(obj, item)) return lst;
        if (is_number(obj) && is_number(item) && obj->number == item->number) return lst;
        lst = cdr(lst);
    }
    return LISP_FALSE;
}

LispObject *prim_member(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "member");
    LispObject *lst = require_arg(args, 1, "member");
    if (!obj || !lst) return LISP_FALSE;
    while (is_cons(lst)) {
        if (lisp_equal(obj, car(lst))) return lst;
        lst = cdr(lst);
    }
    return LISP_FALSE;
}

LispObject *prim_assq(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "assq");
    LispObject *alist = require_arg(args, 1, "assq");
    if (!obj || !alist) return LISP_FALSE;
    while (is_cons(alist)) {
        LispObject *pair = car(alist);
        if (is_cons(pair) && lisp_eq(obj, car(pair))) return pair;
        alist = cdr(alist);
    }
    return LISP_FALSE;
}

LispObject *prim_assv(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "assv");
    LispObject *alist = require_arg(args, 1, "assv");
    if (!obj || !alist) return LISP_FALSE;
    while (is_cons(alist)) {
        LispObject *pair = car(alist);
        if (is_cons(pair)) {
            LispObject *key = car(pair);
            if (lisp_eq(obj, key)) return pair;
            if (is_number(obj) && is_number(key) && obj->number == key->number) return pair;
        }
        alist = cdr(alist);
    }
    return LISP_FALSE;
}

LispObject *prim_assoc(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "assoc");
    LispObject *alist = require_arg(args, 1, "assoc");
    if (!obj || !alist) return LISP_FALSE;
    while (is_cons(alist)) {
        LispObject *pair = car(alist);
        if (is_cons(pair) && lisp_equal(obj, car(pair))) return pair;
        alist = cdr(alist);
    }
    return LISP_FALSE;
}

/* ============================================================
 * R6RS: Character Primitives
 * ============================================================ */

LispObject *prim_char_p(LispObject *args) {
    LispObject *obj = require_arg(args, 0, "char?");
    if (!obj) return LISP_FALSE;
    return make_boolean(obj->type == LISP_CHARACTER);
}

LispObject *prim_char_eq(LispObject *args) {
    LispObject *a = require_arg(args, 0, "char=?");
    LispObject *b = require_arg(args, 1, "char=?");
    if (!a || !b) return LISP_FALSE;
    if (a->type != LISP_CHARACTER || b->type != LISP_CHARACTER) {
        lisp_error("char=?: expected characters");
        return LISP_FALSE;
    }
    return make_boolean(a->character == b->character);
}

LispObject *prim_char_lt(LispObject *args) {
    LispObject *a = require_arg(args, 0, "char<?");
    LispObject *b = require_arg(args, 1, "char<?");
    if (!a || !b) return LISP_FALSE;
    if (a->type != LISP_CHARACTER || b->type != LISP_CHARACTER) {
        lisp_error("char<?: expected characters");
        return LISP_FALSE;
    }
    return make_boolean(a->character < b->character);
}

LispObject *prim_char_to_integer(LispObject *args) {
    LispObject *c = require_arg(args, 0, "char->integer");
    if (!c) return make_number(0);
    if (c->type != LISP_CHARACTER) {
        lisp_error("char->integer: expected character");
        return make_number(0);
    }
    return make_number((double)(unsigned char)c->character);
}

LispObject *prim_integer_to_char(LispObject *args) {
    LispObject *n = require_arg(args, 0, "integer->char");
    if (!n) return make_character('\0');
    if (!is_number(n)) {
        lisp_error("integer->char: expected number");
        return make_character('\0');
    }
    return make_character((char)(int)n->number);
}

/* ============================================================
 * R7RS: Multiple Values
 * ============================================================ */

LispObject *prim_values(LispObject *args) {
    int count = list_length(args);
    if (count == 0) {
        return make_nil();
    }
    if (count == 1) {
        return car(args);
    }

    LispObject **vals = (LispObject **)malloc(count * sizeof(LispObject *));
    int i = 0;
    while (is_cons(args)) {
        vals[i++] = car(args);
        args = cdr(args);
    }
    return make_values(vals, count);
}

LispObject *prim_call_with_values(LispObject *args) {
    LispObject *producer = require_arg(args, 0, "call-with-values");
    LispObject *consumer = require_arg(args, 1, "call-with-values");
    if (!producer || !consumer) return make_nil();

    /* Call producer with no arguments */
    LispObject *vals = apply(producer, make_nil(), NULL);

    /* Call consumer with produced values */
    if (is_values(vals)) {
        /* Convert values to list */
        LispObject *val_list = make_nil();
        for (int i = vals->values.count - 1; i >= 0; i--) {
            val_list = make_cons(vals->values.vals[i], val_list);
        }
        return apply(consumer, val_list, NULL);
    } else {
        /* Single value */
        return apply(consumer, make_cons(vals, make_nil()), NULL);
    }
}

/* ============================================================
 * R7RS: Additional List Operations
 * ============================================================ */

LispObject *prim_make_list(LispObject *args) {
    LispObject *k_obj = require_arg(args, 0, "make-list");
    if (!k_obj) return make_nil();
    if (!is_number(k_obj)) {
        lisp_error("make-list: expected number for length");
        return make_nil();
    }

    int k = (int)k_obj->number;
    LispObject *fill = make_nil();
    if (is_cons(cdr(args))) {
        fill = cadr(args);
    }

    LispObject *result = make_nil();
    for (int i = 0; i < k; i++) {
        result = make_cons(fill, result);
    }
    return result;
}

LispObject *prim_list_copy(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "list-copy");
    if (!lst) return make_nil();

    LispObject *result = make_nil();
    LispObject *tail = NULL;

    while (is_cons(lst)) {
        LispObject *new_cell = make_cons(car(lst), make_nil());
        if (tail) {
            tail->cons.cdr = new_cell;
        } else {
            result = new_cell;
        }
        tail = new_cell;
        lst = cdr(lst);
    }

    /* Handle improper list */
    if (!is_nil(lst) && tail) {
        tail->cons.cdr = lst;
    }

    return result;
}

LispObject *prim_list_set(LispObject *args) {
    LispObject *lst = require_arg(args, 0, "list-set!");
    LispObject *k = require_arg(args, 1, "list-set!");
    LispObject *obj = require_arg(args, 2, "list-set!");
    if (!lst || !k || !obj) return make_nil();

    if (!is_number(k)) {
        lisp_error("list-set!: expected number for index");
        return make_nil();
    }

    int index = (int)k->number;
    while (index > 0 && is_cons(lst)) {
        lst = cdr(lst);
        index--;
    }

    if (is_cons(lst)) {
        lst->cons.car = obj;
    }

    return make_nil();
}

/* ============================================================
 * R7RS: Additional Vector Operations
 * ============================================================ */

LispObject *prim_vector_copy(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector-copy");
    if (!vec) return make_nil();
    if (!is_vector(vec)) {
        lisp_error("vector-copy: expected vector");
        return make_nil();
    }

    size_t start = 0;
    size_t end = vec->vector.length;

    if (is_cons(cdr(args))) {
        LispObject *start_obj = cadr(args);
        if (is_number(start_obj)) {
            start = (size_t)start_obj->number;
        }
    }
    if (is_cons(cddr(args))) {
        LispObject *end_obj = caddr(args);
        if (is_number(end_obj)) {
            end = (size_t)end_obj->number;
        }
    }

    if (start > end || end > vec->vector.length) {
        lisp_error("vector-copy: invalid range");
        return make_nil();
    }

    size_t len = end - start;
    LispObject *result = make_vector(len, make_nil());
    for (size_t i = 0; i < len; i++) {
        result->vector.elements[i] = vec->vector.elements[start + i];
    }
    return result;
}

LispObject *prim_vector_fill(LispObject *args) {
    LispObject *vec = require_arg(args, 0, "vector-fill!");
    LispObject *fill = require_arg(args, 1, "vector-fill!");
    if (!vec || !fill) return make_nil();

    if (!is_vector(vec)) {
        lisp_error("vector-fill!: expected vector");
        return make_nil();
    }

    size_t start = 0;
    size_t end = vec->vector.length;

    if (is_cons(cddr(args))) {
        LispObject *start_obj = caddr(args);
        if (is_number(start_obj)) {
            start = (size_t)start_obj->number;
        }
    }
    if (is_cons(cdr(cddr(args)))) {
        LispObject *end_obj = car(cdr(cddr(args)));
        if (is_number(end_obj)) {
            end = (size_t)end_obj->number;
        }
    }

    for (size_t i = start; i < end && i < vec->vector.length; i++) {
        vec->vector.elements[i] = fill;
    }

    return make_nil();
}

LispObject *prim_vector_append(LispObject *args) {
    /* Calculate total length */
    size_t total_len = 0;
    LispObject *a = args;
    while (is_cons(a)) {
        LispObject *vec = car(a);
        if (!is_vector(vec)) {
            lisp_error("vector-append: expected vector");
            return make_nil();
        }
        total_len += vec->vector.length;
        a = cdr(a);
    }

    LispObject *result = make_vector(total_len, make_nil());
    size_t pos = 0;
    a = args;
    while (is_cons(a)) {
        LispObject *vec = car(a);
        for (size_t i = 0; i < vec->vector.length; i++) {
            result->vector.elements[pos++] = vec->vector.elements[i];
        }
        a = cdr(a);
    }

    return result;
}

/* ============================================================
 * R7RS: Additional String Operations
 * ============================================================ */

LispObject *prim_string_copy(LispObject *args) {
    LispObject *str = require_arg(args, 0, "string-copy");
    if (!str) return make_string("");
    if (!is_string(str)) {
        lisp_error("string-copy: expected string");
        return make_string("");
    }

    size_t start = 0;
    size_t end = str->string.length;

    if (is_cons(cdr(args))) {
        LispObject *start_obj = cadr(args);
        if (is_number(start_obj)) {
            start = (size_t)start_obj->number;
        }
    }
    if (is_cons(cddr(args))) {
        LispObject *end_obj = caddr(args);
        if (is_number(end_obj)) {
            end = (size_t)end_obj->number;
        }
    }

    if (start > end || end > str->string.length) {
        lisp_error("string-copy: invalid range");
        return make_string("");
    }

    return make_string_n(str->string.data + start, end - start);
}

LispObject *prim_substring(LispObject *args) {
    LispObject *str = require_arg(args, 0, "substring");
    LispObject *start_obj = require_arg(args, 1, "substring");
    LispObject *end_obj = require_arg(args, 2, "substring");
    if (!str || !start_obj || !end_obj) return make_string("");

    if (!is_string(str)) {
        lisp_error("substring: expected string");
        return make_string("");
    }
    if (!is_number(start_obj) || !is_number(end_obj)) {
        lisp_error("substring: expected numbers for indices");
        return make_string("");
    }

    size_t start = (size_t)start_obj->number;
    size_t end = (size_t)end_obj->number;

    if (start > end || end > str->string.length) {
        lisp_error("substring: invalid range");
        return make_string("");
    }

    return make_string_n(str->string.data + start, end - start);
}

LispObject *prim_string_eq(LispObject *args) {
    LispObject *a = require_arg(args, 0, "string=?");
    LispObject *b = require_arg(args, 1, "string=?");
    if (!a || !b) return LISP_FALSE;

    if (!is_string(a) || !is_string(b)) {
        lisp_error("string=?: expected strings");
        return LISP_FALSE;
    }

    return make_boolean(a->string.length == b->string.length &&
                       strcmp(a->string.data, b->string.data) == 0);
}

LispObject *prim_string_lt(LispObject *args) {
    LispObject *a = require_arg(args, 0, "string<?");
    LispObject *b = require_arg(args, 1, "string<?");
    if (!a || !b) return LISP_FALSE;

    if (!is_string(a) || !is_string(b)) {
        lisp_error("string<?: expected strings");
        return LISP_FALSE;
    }

    return make_boolean(strcmp(a->string.data, b->string.data) < 0);
}

/* ============================================================
 * R7RS: Additional Numeric Operations
 * ============================================================ */

LispObject *prim_square(LispObject *args) {
    LispObject *n = require_arg(args, 0, "square");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("square: expected number");
        return make_number(0);
    }
    return make_number(n->number * n->number);
}

LispObject *prim_exact(LispObject *args) {
    LispObject *n = require_arg(args, 0, "exact");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("exact: expected number");
        return make_number(0);
    }
    /* For our implementation, just truncate to integer */
    return make_number((double)(long long)n->number);
}

LispObject *prim_inexact(LispObject *args) {
    LispObject *n = require_arg(args, 0, "inexact");
    if (!n) return make_number(0);
    if (!is_number(n)) {
        lisp_error("inexact: expected number");
        return make_number(0);
    }
    /* All our numbers are already inexact (doubles) */
    return n;
}

LispObject *prim_finite_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "finite?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("finite?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(isfinite(n->number));
}

LispObject *prim_infinite_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "infinite?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("infinite?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(isinf(n->number));
}

LispObject *prim_nan_p(LispObject *args) {
    LispObject *n = require_arg(args, 0, "nan?");
    if (!n) return LISP_FALSE;
    if (!is_number(n)) {
        lisp_error("nan?: expected number");
        return LISP_FALSE;
    }
    return make_boolean(isnan(n->number));
}

LispObject *prim_gcd(LispObject *args) {
    if (!is_cons(args)) return make_number(0);

    long long result = (long long)car(args)->number;
    if (result < 0) result = -result;
    args = cdr(args);

    while (is_cons(args)) {
        long long b = (long long)car(args)->number;
        if (b < 0) b = -b;

        while (b != 0) {
            long long t = b;
            b = result % b;
            result = t;
        }
        args = cdr(args);
    }

    return make_number((double)result);
}

LispObject *prim_lcm(LispObject *args) {
    if (!is_cons(args)) return make_number(1);

    long long result = (long long)car(args)->number;
    if (result < 0) result = -result;
    args = cdr(args);

    while (is_cons(args)) {
        long long b = (long long)car(args)->number;
        if (b < 0) b = -b;

        /* LCM = |a * b| / GCD(a, b) */
        long long a = result;
        long long gcd_val = a;
        long long temp = b;
        while (temp != 0) {
            long long t = temp;
            temp = gcd_val % temp;
            gcd_val = t;
        }
        result = (a / gcd_val) * b;

        args = cdr(args);
    }

    return make_number((double)result);
}

/* ============================================================
 * R7RS: Equivalence Predicates
 * ============================================================ */

LispObject *prim_boolean_eq(LispObject *args) {
    if (!is_cons(args)) return LISP_TRUE;

    LispObject *first = car(args);
    if (!is_boolean(first)) {
        lisp_error("boolean=?: expected boolean");
        return LISP_FALSE;
    }

    args = cdr(args);
    while (is_cons(args)) {
        LispObject *b = car(args);
        if (!is_boolean(b)) {
            lisp_error("boolean=?: expected boolean");
            return LISP_FALSE;
        }
        if (first->boolean != b->boolean) {
            return LISP_FALSE;
        }
        args = cdr(args);
    }

    return LISP_TRUE;
}

LispObject *prim_symbol_eq(LispObject *args) {
    if (!is_cons(args)) return LISP_TRUE;

    LispObject *first = car(args);
    if (!is_symbol(first)) {
        lisp_error("symbol=?: expected symbol");
        return LISP_FALSE;
    }

    args = cdr(args);
    while (is_cons(args)) {
        LispObject *s = car(args);
        if (!is_symbol(s)) {
            lisp_error("symbol=?: expected symbol");
            return LISP_FALSE;
        }
        if (first != s) {  /* Symbols are interned */
            return LISP_FALSE;
        }
        args = cdr(args);
    }

    return LISP_TRUE;
}

/* ============================================================
 * R7RS: Higher-order Functions
 * ============================================================ */

LispObject *prim_map(LispObject *args) {
    LispObject *proc = require_arg(args, 0, "map");
    if (!proc) return make_nil();

    /* Collect lists */
    LispObject *lists = cdr(args);
    if (!is_cons(lists)) {
        lisp_error("map: requires at least one list");
        return make_nil();
    }

    LispObject *result = make_nil();
    LispObject *result_tail = NULL;

    while (1) {
        /* Check if any list is empty */
        LispObject *l = lists;
        while (is_cons(l)) {
            if (!is_cons(car(l))) {
                return result;
            }
            l = cdr(l);
        }

        /* Collect arguments (car of each list) */
        LispObject *call_args = make_nil();
        LispObject *call_tail = NULL;
        l = lists;
        while (is_cons(l)) {
            LispObject *new_arg = make_cons(car(car(l)), make_nil());
            if (call_tail) {
                call_tail->cons.cdr = new_arg;
            } else {
                call_args = new_arg;
            }
            call_tail = new_arg;
            l = cdr(l);
        }

        /* Apply procedure */
        LispObject *value = apply(proc, call_args, NULL);

        /* Add to result */
        LispObject *new_cell = make_cons(value, make_nil());
        if (result_tail) {
            result_tail->cons.cdr = new_cell;
        } else {
            result = new_cell;
        }
        result_tail = new_cell;

        /* Advance all lists */
        LispObject *new_lists = make_nil();
        LispObject *new_lists_tail = NULL;
        l = lists;
        while (is_cons(l)) {
            LispObject *new_list = make_cons(cdr(car(l)), make_nil());
            if (new_lists_tail) {
                new_lists_tail->cons.cdr = new_list;
            } else {
                new_lists = new_list;
            }
            new_lists_tail = new_list;
            l = cdr(l);
        }
        lists = new_lists;
    }
}

LispObject *prim_for_each(LispObject *args) {
    LispObject *proc = require_arg(args, 0, "for-each");
    if (!proc) return make_nil();

    LispObject *lists = cdr(args);
    if (!is_cons(lists)) {
        lisp_error("for-each: requires at least one list");
        return make_nil();
    }

    while (1) {
        /* Check if any list is empty */
        LispObject *l = lists;
        while (is_cons(l)) {
            if (!is_cons(car(l))) {
                return make_nil();
            }
            l = cdr(l);
        }

        /* Collect arguments */
        LispObject *call_args = make_nil();
        LispObject *call_tail = NULL;
        l = lists;
        while (is_cons(l)) {
            LispObject *new_arg = make_cons(car(car(l)), make_nil());
            if (call_tail) {
                call_tail->cons.cdr = new_arg;
            } else {
                call_args = new_arg;
            }
            call_tail = new_arg;
            l = cdr(l);
        }

        /* Apply procedure (discard result) */
        apply(proc, call_args, NULL);

        /* Advance all lists */
        LispObject *new_lists = make_nil();
        LispObject *new_lists_tail = NULL;
        l = lists;
        while (is_cons(l)) {
            LispObject *new_list = make_cons(cdr(car(l)), make_nil());
            if (new_lists_tail) {
                new_lists_tail->cons.cdr = new_list;
            } else {
                new_lists = new_list;
            }
            new_lists_tail = new_list;
            l = cdr(l);
        }
        lists = new_lists;
    }
}

LispObject *prim_filter(LispObject *args) {
    LispObject *proc = require_arg(args, 0, "filter");
    LispObject *lst = require_arg(args, 1, "filter");
    if (!proc || !lst) return make_nil();

    LispObject *result = make_nil();
    LispObject *result_tail = NULL;

    while (is_cons(lst)) {
        LispObject *item = car(lst);
        LispObject *test = apply(proc, make_cons(item, make_nil()), NULL);

        if (is_true(test)) {
            LispObject *new_cell = make_cons(item, make_nil());
            if (result_tail) {
                result_tail->cons.cdr = new_cell;
            } else {
                result = new_cell;
            }
            result_tail = new_cell;
        }

        lst = cdr(lst);
    }

    return result;
}

LispObject *prim_fold(LispObject *args) {
    LispObject *proc = require_arg(args, 0, "fold");
    LispObject *init = require_arg(args, 1, "fold");
    LispObject *lst = require_arg(args, 2, "fold");
    if (!proc || !init || !lst) return make_nil();

    LispObject *accum = init;
    while (is_cons(lst)) {
        LispObject *call_args = make_cons(car(lst), make_cons(accum, make_nil()));
        accum = apply(proc, call_args, NULL);
        lst = cdr(lst);
    }

    return accum;
}

LispObject *prim_fold_right(LispObject *args) {
    LispObject *proc = require_arg(args, 0, "fold-right");
    LispObject *init = require_arg(args, 1, "fold-right");
    LispObject *lst = require_arg(args, 2, "fold-right");
    if (!proc || !init || !lst) return make_nil();

    /* Reverse the list first */
    LispObject *reversed = list_reverse(lst);

    LispObject *accum = init;
    while (is_cons(reversed)) {
        LispObject *call_args = make_cons(car(reversed), make_cons(accum, make_nil()));
        accum = apply(proc, call_args, NULL);
        reversed = cdr(reversed);
    }

    return accum;
}

/* Register all primitives */
void register_primitives(Environment *env) {
    struct {
        const char *name;
        LispPrimitiveFn func;
        int min_args;
        int max_args;
    } prims[] = {
        /* List operations */
        {"car",     prim_car,     1, 1},
        {"cdr",     prim_cdr,     1, 1},
        {"cons",    prim_cons,    2, 2},
        {"list",    prim_list,    0, -1},
        {"length",  prim_length,  1, 1},
        {"append",  prim_append,  0, -1},
        {"reverse", prim_reverse, 1, 1},

        /* Type predicates */
        {"null?",      prim_null_p,      1, 1},
        {"pair?",      prim_pair_p,      1, 1},
        {"number?",    prim_number_p,    1, 1},
        {"symbol?",    prim_symbol_p,    1, 1},
        {"string?",    prim_string_p,    1, 1},
        {"procedure?", prim_procedure_p, 1, 1},
        {"boolean?",   prim_boolean_p,   1, 1},

        /* Arithmetic */
        {"+",   prim_add, 0, -1},
        {"-",   prim_sub, 1, -1},
        {"*",   prim_mul, 0, -1},
        {"/",   prim_div, 2, 2},
        {"mod", prim_mod, 2, 2},
        {"abs", prim_abs, 1, 1},

        /* Comparison */
        {"=",      prim_eq_num, 2, 2},
        {"<",      prim_lt,     2, 2},
        {">",      prim_gt,     2, 2},
        {"<=",     prim_le,     2, 2},
        {">=",     prim_ge,     2, 2},
        {"eq?",    prim_eq,     2, 2},
        {"equal?", prim_equal,  2, 2},

        /* Boolean */
        {"not", prim_not, 1, 1},

        /* I/O */
        {"display", prim_display, 1, 1},
        {"newline", prim_newline, 0, 0},
        {"print",   prim_print,   1, 1},

        /* String operations */
        {"string-length",   prim_string_length,   1, 1},
        {"string-append",   prim_string_append,   0, -1},
        {"string-ref",      prim_string_ref,      2, 2},
        {"number->string",  prim_number_to_string, 1, 1},
        {"string->number",  prim_string_to_number, 1, 1},
        {"symbol->string",  prim_symbol_to_string, 1, 1},
        {"string->symbol",  prim_string_to_symbol, 1, 1},

        /* Utility */
        {"apply", prim_apply, 2, 2},
        {"error", prim_error, 0, -1},

        /* R6RS: Vectors */
        {"vector?",       prim_vector_p,       1, 1},
        {"make-vector",   prim_make_vector,    1, 2},
        {"vector",        prim_vector,         0, -1},
        {"vector-length", prim_vector_length,  1, 1},
        {"vector-ref",    prim_vector_ref,     2, 2},
        {"vector-set!",   prim_vector_set,     3, 3},
        {"vector->list",  prim_vector_to_list, 1, 1},
        {"list->vector",  prim_list_to_vector, 1, 1},

        /* R6RS: Bytevectors */
        {"bytevector?",       prim_bytevector_p,       1, 1},
        {"make-bytevector",   prim_make_bytevector,    1, 2},
        {"bytevector-length", prim_bytevector_length,  1, 1},
        {"bytevector-u8-ref", prim_bytevector_u8_ref,  2, 2},
        {"bytevector-u8-set!", prim_bytevector_u8_set, 3, 3},

        /* R6RS: Hashtables */
        {"hashtable?",         prim_hashtable_p,         1, 1},
        {"make-eq-hashtable",  prim_make_eq_hashtable,   0, 0},
        {"make-eqv-hashtable", prim_make_eqv_hashtable,  0, 0},
        {"make-hashtable",     prim_make_hashtable,      0, 2},
        {"hashtable-ref",      prim_hashtable_ref,       3, 3},
        {"hashtable-set!",     prim_hashtable_set,       3, 3},
        {"hashtable-delete!",  prim_hashtable_delete,    2, 2},
        {"hashtable-contains?", prim_hashtable_contains, 2, 2},
        {"hashtable-size",     prim_hashtable_size,      1, 1},
        {"hashtable-keys",     prim_hashtable_keys,      1, 1},

        /* R6RS: Additional numeric */
        {"floor",     prim_floor,     1, 1},
        {"ceiling",   prim_ceiling,   1, 1},
        {"truncate",  prim_truncate,  1, 1},
        {"round",     prim_round,     1, 1},
        {"sqrt",      prim_sqrt,      1, 1},
        {"expt",      prim_expt,      2, 2},
        {"log",       prim_log,       1, 1},
        {"sin",       prim_sin,       1, 1},
        {"cos",       prim_cos,       1, 1},
        {"tan",       prim_tan,       1, 1},
        {"quotient",  prim_quotient,  2, 2},
        {"remainder", prim_remainder, 2, 2},
        {"modulo",    prim_modulo,    2, 2},
        {"integer?",  prim_integer_p, 1, 1},
        {"real?",     prim_real_p,    1, 1},
        {"zero?",     prim_zero_p,    1, 1},
        {"positive?", prim_positive_p, 1, 1},
        {"negative?", prim_negative_p, 1, 1},
        {"odd?",      prim_odd_p,     1, 1},
        {"even?",     prim_even_p,    1, 1},
        {"min",       prim_min,       1, -1},
        {"max",       prim_max,       1, -1},

        /* R6RS: Additional list operations */
        {"list?",     prim_list_p,    1, 1},
        {"list-ref",  prim_list_ref,  2, 2},
        {"list-tail", prim_list_tail, 2, 2},
        {"memq",      prim_memq,      2, 2},
        {"memv",      prim_memv,      2, 2},
        {"member",    prim_member,    2, 2},
        {"assq",      prim_assq,      2, 2},
        {"assv",      prim_assv,      2, 2},
        {"assoc",     prim_assoc,     2, 2},

        /* R6RS: Characters */
        {"char?",          prim_char_p,          1, 1},
        {"char=?",         prim_char_eq,         2, 2},
        {"char<?",         prim_char_lt,         2, 2},
        {"char->integer",  prim_char_to_integer, 1, 1},
        {"integer->char",  prim_integer_to_char, 1, 1},

        /* R7RS: Multiple values */
        {"values",           prim_values,           0, -1},
        {"call-with-values", prim_call_with_values, 2, 2},

        /* R7RS: List operations */
        {"make-list",  prim_make_list,  1, 2},
        {"list-copy",  prim_list_copy,  1, 1},
        {"list-set!",  prim_list_set,   3, 3},

        /* R7RS: Vector operations */
        {"vector-copy",   prim_vector_copy,   1, 3},
        {"vector-fill!",  prim_vector_fill,   2, 4},
        {"vector-append", prim_vector_append, 0, -1},

        /* R7RS: String operations */
        {"string-copy", prim_string_copy, 1, 3},
        {"substring",   prim_substring,   3, 3},
        {"string=?",    prim_string_eq,   2, 2},
        {"string<?",    prim_string_lt,   2, 2},

        /* R7RS: Numeric operations */
        {"square",    prim_square,     1, 1},
        {"exact",     prim_exact,      1, 1},
        {"inexact",   prim_inexact,    1, 1},
        {"finite?",   prim_finite_p,   1, 1},
        {"infinite?", prim_infinite_p, 1, 1},
        {"nan?",      prim_nan_p,      1, 1},
        {"gcd",       prim_gcd,        0, -1},
        {"lcm",       prim_lcm,        0, -1},

        /* R7RS: Equivalence */
        {"boolean=?", prim_boolean_eq, 1, -1},
        {"symbol=?",  prim_symbol_eq,  1, -1},

        /* R7RS: Higher-order functions */
        {"map",        prim_map,        2, -1},
        {"for-each",   prim_for_each,   2, -1},
        {"filter",     prim_filter,     2, 2},
        {"fold",       prim_fold,       3, 3},
        {"fold-right", prim_fold_right, 3, 3},

        {NULL, NULL, 0, 0}
    };

    for (int i = 0; prims[i].name != NULL; i++) {
        LispObject *prim = make_primitive(prims[i].name, prims[i].func,
                                          prims[i].min_args, prims[i].max_args);
        env_define(env, make_symbol(prims[i].name), prim);
    }
}
