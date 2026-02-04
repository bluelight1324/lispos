/*
 * lisp.h - Lisp Object Representation
 *
 * Core data structures for the Lisp compiler/interpreter.
 * All Lisp values are represented as LispObject pointers.
 */

#ifndef LISP_H
#define LISP_H

#include <stdint.h>
#include <stddef.h>

/* Forward declarations */
typedef struct LispObject LispObject;
typedef struct Environment Environment;

/* Object types */
typedef enum {
    LISP_NIL,
    LISP_BOOLEAN,
    LISP_NUMBER,
    LISP_CHARACTER,
    LISP_STRING,
    LISP_SYMBOL,
    LISP_CONS,
    LISP_LAMBDA,
    LISP_PRIMITIVE,
    LISP_MACRO,
    /* R6RS types */
    LISP_VECTOR,
    LISP_BYTEVECTOR,
    LISP_HASHTABLE,
    LISP_RECORD_TYPE,
    LISP_RECORD,
    LISP_CONDITION,
    LISP_VALUES,
    LISP_PORT
} LispType;

/* Primitive function pointer type */
typedef LispObject* (*LispPrimitiveFn)(LispObject *args);

/* The universal Lisp object structure */
struct LispObject {
    LispType type;
    uint8_t gc_mark;      /* For garbage collection */

    union {
        /* Boolean */
        int boolean;

        /* Number (double precision) */
        double number;

        /* Character */
        char character;

        /* String */
        struct {
            char *data;
            size_t length;
        } string;

        /* Symbol */
        struct {
            char *name;
            uint32_t hash;
        } symbol;

        /* Cons cell (pair) */
        struct {
            LispObject *car;
            LispObject *cdr;
        } cons;

        /* Lambda (closure) */
        struct {
            LispObject *params;      /* Parameter list */
            LispObject *body;        /* Body expressions */
            Environment *env;        /* Captured environment */
            char *name;              /* Optional name for debugging */
        } lambda;

        /* Primitive function */
        struct {
            const char *name;
            LispPrimitiveFn func;
            int min_args;
            int max_args;            /* -1 for variadic */
        } primitive;

        /* Macro */
        struct {
            LispObject *params;
            LispObject *body;
            Environment *env;
        } macro;

        /* R6RS: Vector */
        struct {
            LispObject **elements;
            size_t length;
        } vector;

        /* R6RS: Bytevector */
        struct {
            uint8_t *bytes;
            size_t length;
        } bytevector;

        /* R6RS: Hash table */
        struct {
            LispObject **keys;
            LispObject **values;
            size_t capacity;
            size_t count;
            int hash_type;  /* 0=eq, 1=eqv, 2=equal */
        } hashtable;

        /* R6RS: Record type descriptor */
        struct {
            LispObject *name;
            LispObject *parent;
            LispObject *fields;     /* List of field names */
            int field_count;
            int sealed;
            int opaque;
        } record_type;

        /* R6RS: Record instance */
        struct {
            LispObject *rtd;        /* Record type descriptor */
            LispObject **fields;
        } record;

        /* R6RS: Condition */
        struct {
            LispObject *type;       /* Condition type */
            LispObject *message;
            LispObject *irritants;
            LispObject *who;
        } condition;

        /* R6RS: Multiple values */
        struct {
            LispObject **vals;
            int count;
        } values;

        /* R6RS: Port */
        struct {
            void *stream;
            int is_input;
            int is_output;
            int is_binary;
            int is_open;
            char *name;
        } port;
    };
};

/* Global singleton objects */
extern LispObject *LISP_NIL_OBJ;
extern LispObject *LISP_TRUE;
extern LispObject *LISP_FALSE;

/* Initialize the Lisp system */
void lisp_init(void);
void lisp_shutdown(void);

/* Object constructors */
LispObject *make_nil(void);
LispObject *make_boolean(int value);
LispObject *make_number(double value);
LispObject *make_character(char c);
LispObject *make_string(const char *str);
LispObject *make_string_n(const char *str, size_t len);
LispObject *make_symbol(const char *name);
LispObject *make_cons(LispObject *car, LispObject *cdr);
LispObject *make_lambda(LispObject *params, LispObject *body, Environment *env);
LispObject *make_primitive(const char *name, LispPrimitiveFn func, int min_args, int max_args);
LispObject *make_macro(LispObject *params, LispObject *body, Environment *env);

/* R6RS constructors */
LispObject *make_vector(size_t length, LispObject *fill);
LispObject *make_vector_from_list(LispObject *list);
LispObject *make_bytevector(size_t length, uint8_t fill);
LispObject *make_hashtable(int hash_type, size_t initial_capacity);
LispObject *make_record_type(LispObject *name, LispObject *parent, LispObject *fields);
LispObject *make_record(LispObject *rtd);
LispObject *make_condition(LispObject *type, LispObject *message, LispObject *irritants);
LispObject *make_values(LispObject **vals, int count);
LispObject *make_port(void *stream, int is_input, int is_output, int is_binary, const char *name);

/* Type checking */
int is_nil(LispObject *obj);
int is_true(LispObject *obj);
int is_false(LispObject *obj);
int is_boolean(LispObject *obj);
int is_number(LispObject *obj);
int is_string(LispObject *obj);
int is_symbol(LispObject *obj);
int is_cons(LispObject *obj);
int is_list(LispObject *obj);
int is_lambda(LispObject *obj);
int is_primitive(LispObject *obj);
int is_callable(LispObject *obj);
int is_macro(LispObject *obj);

/* R6RS type checking */
int is_vector(LispObject *obj);
int is_bytevector(LispObject *obj);
int is_hashtable(LispObject *obj);
int is_record_type(LispObject *obj);
int is_record(LispObject *obj);
int is_condition(LispObject *obj);
int is_values(LispObject *obj);
int is_port(LispObject *obj);
int is_input_port(LispObject *obj);
int is_output_port(LispObject *obj);

/* Accessors for cons cells */
LispObject *car(LispObject *obj);
LispObject *cdr(LispObject *obj);
LispObject *cadr(LispObject *obj);
LispObject *caddr(LispObject *obj);
LispObject *cddr(LispObject *obj);
LispObject *caar(LispObject *obj);
LispObject *cdar(LispObject *obj);

/* List utilities */
int list_length(LispObject *list);
LispObject *list_reverse(LispObject *list);
LispObject *list_append(LispObject *list1, LispObject *list2);
LispObject *list_nth(LispObject *list, int n);
LispObject *list_set_tail(LispObject *list, LispObject *tail);

/* Symbol utilities */
int symbol_eq(LispObject *a, LispObject *b);
int is_symbol_named(LispObject *obj, const char *name);

/* Object comparison */
int lisp_eq(LispObject *a, LispObject *b);
int lisp_equal(LispObject *a, LispObject *b);

/* Printing */
void lisp_print(LispObject *obj);
void lisp_print_to_buffer(LispObject *obj, char *buffer, size_t size);
const char *lisp_type_name(LispType type);

/* Memory management */
LispObject *lisp_alloc(void);
void lisp_free(LispObject *obj);

/* Error handling */
void lisp_error(const char *format, ...);

/* R6RS: Vector operations */
LispObject *vector_ref(LispObject *vec, size_t index);
void vector_set(LispObject *vec, size_t index, LispObject *value);
size_t vector_length(LispObject *vec);
LispObject *vector_to_list(LispObject *vec);

/* R6RS: Bytevector operations */
uint8_t bytevector_ref(LispObject *bv, size_t index);
void bytevector_set(LispObject *bv, size_t index, uint8_t value);
size_t bytevector_length(LispObject *bv);

/* R6RS: Hashtable operations */
LispObject *hashtable_ref(LispObject *ht, LispObject *key, LispObject *default_val);
void hashtable_set(LispObject *ht, LispObject *key, LispObject *value);
int hashtable_contains(LispObject *ht, LispObject *key);
void hashtable_delete(LispObject *ht, LispObject *key);
size_t hashtable_size(LispObject *ht);
LispObject *hashtable_keys(LispObject *ht);
LispObject *hashtable_values(LispObject *ht);

/* R6RS: Record operations */
LispObject *record_ref(LispObject *rec, int field_index);
void record_set(LispObject *rec, int field_index, LispObject *value);
LispObject *record_rtd(LispObject *rec);

#endif /* LISP_H */
