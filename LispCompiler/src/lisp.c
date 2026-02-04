/*
 * lisp.c - Lisp Object Implementation
 */

#include "lisp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Global singletons */
LispObject *LISP_NIL_OBJ = NULL;
LispObject *LISP_TRUE = NULL;
LispObject *LISP_FALSE = NULL;

/* Symbol interning table */
#define SYMBOL_TABLE_SIZE 1024
static LispObject *symbol_table[SYMBOL_TABLE_SIZE];

/* Simple memory tracking for GC */
#define MAX_OBJECTS 262144  /* Increased from 65536 for deep recursion support (256K objects) */
static LispObject *all_objects[MAX_OBJECTS];
static int num_objects = 0;

/* Hash function for symbols */
static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/* Allocate a new object */
LispObject *lisp_alloc(void) {
    if (num_objects >= MAX_OBJECTS) {
        lisp_error("Out of memory: too many objects");
        return NULL;
    }
    LispObject *obj = (LispObject *)calloc(1, sizeof(LispObject));
    if (!obj) {
        lisp_error("Out of memory");
        return NULL;
    }
    all_objects[num_objects++] = obj;
    return obj;
}

/* Free an object (not typically called directly) */
void lisp_free(LispObject *obj) {
    if (!obj) return;

    switch (obj->type) {
        case LISP_STRING:
            free(obj->string.data);
            break;
        case LISP_SYMBOL:
            /* Symbols are interned, don't free name */
            break;
        case LISP_LAMBDA:
            if (obj->lambda.name) free(obj->lambda.name);
            break;
        case LISP_VECTOR:
            free(obj->vector.elements);
            break;
        case LISP_BYTEVECTOR:
            free(obj->bytevector.bytes);
            break;
        case LISP_HASHTABLE:
            free(obj->hashtable.keys);
            free(obj->hashtable.values);
            break;
        case LISP_RECORD:
            free(obj->record.fields);
            break;
        case LISP_VALUES:
            free(obj->values.vals);
            break;
        case LISP_PORT:
            if (obj->port.name) free(obj->port.name);
            break;
        default:
            break;
    }
    free(obj);
}

/* Initialize the Lisp system */
void lisp_init(void) {
    /* Initialize symbol table */
    memset(symbol_table, 0, sizeof(symbol_table));

    /* Create singleton objects */
    LISP_NIL_OBJ = lisp_alloc();
    LISP_NIL_OBJ->type = LISP_NIL;

    LISP_TRUE = lisp_alloc();
    LISP_TRUE->type = LISP_BOOLEAN;
    LISP_TRUE->boolean = 1;

    LISP_FALSE = lisp_alloc();
    LISP_FALSE->type = LISP_BOOLEAN;
    LISP_FALSE->boolean = 0;
}

/* Shutdown the Lisp system */
void lisp_shutdown(void) {
    /* Free all allocated objects */
    for (int i = 0; i < num_objects; i++) {
        if (all_objects[i]) {
            lisp_free(all_objects[i]);
            all_objects[i] = NULL;
        }
    }
    num_objects = 0;
    LISP_NIL_OBJ = NULL;
    LISP_TRUE = NULL;
    LISP_FALSE = NULL;
}

/* Object constructors */

LispObject *make_nil(void) {
    return LISP_NIL_OBJ;
}

LispObject *make_boolean(int value) {
    return value ? LISP_TRUE : LISP_FALSE;
}

LispObject *make_number(double value) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_NUMBER;
    obj->number = value;
    return obj;
}

LispObject *make_character(char c) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_CHARACTER;
    obj->character = c;
    return obj;
}

LispObject *make_string(const char *str) {
    return make_string_n(str, strlen(str));
}

LispObject *make_string_n(const char *str, size_t len) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_STRING;
    obj->string.data = (char *)malloc(len + 1);
    memcpy(obj->string.data, str, len);
    obj->string.data[len] = '\0';
    obj->string.length = len;
    return obj;
}

LispObject *make_symbol(const char *name) {
    uint32_t hash = hash_string(name);
    int index = hash % SYMBOL_TABLE_SIZE;

    /* Look for existing symbol */
    LispObject *sym = symbol_table[index];
    while (sym) {
        if (sym->symbol.hash == hash && strcmp(sym->symbol.name, name) == 0) {
            return sym;  /* Return interned symbol */
        }
        /* Linear probing for collision */
        index = (index + 1) % SYMBOL_TABLE_SIZE;
        sym = symbol_table[index];
    }

    /* Create new symbol */
    LispObject *obj = lisp_alloc();
    obj->type = LISP_SYMBOL;
    obj->symbol.name = strdup(name);
    obj->symbol.hash = hash;
    symbol_table[index] = obj;
    return obj;
}

LispObject *make_cons(LispObject *car_val, LispObject *cdr_val) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_CONS;
    obj->cons.car = car_val;
    obj->cons.cdr = cdr_val;
    return obj;
}

LispObject *make_lambda(LispObject *params, LispObject *body, Environment *env) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_LAMBDA;
    obj->lambda.params = params;
    obj->lambda.body = body;
    obj->lambda.env = env;
    obj->lambda.name = NULL;
    return obj;
}

LispObject *make_primitive(const char *name, LispPrimitiveFn func, int min_args, int max_args) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_PRIMITIVE;
    obj->primitive.name = name;
    obj->primitive.func = func;
    obj->primitive.min_args = min_args;
    obj->primitive.max_args = max_args;
    return obj;
}

LispObject *make_macro(LispObject *params, LispObject *body, Environment *env) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_MACRO;
    obj->macro.params = params;
    obj->macro.body = body;
    obj->macro.env = env;
    return obj;
}

/* Type checking */

int is_nil(LispObject *obj) {
    return obj == LISP_NIL_OBJ || obj->type == LISP_NIL;
}

int is_true(LispObject *obj) {
    /* In Lisp, everything except #f is true */
    return obj != LISP_FALSE && !(obj->type == LISP_BOOLEAN && !obj->boolean);
}

int is_false(LispObject *obj) {
    return obj == LISP_FALSE || (obj->type == LISP_BOOLEAN && !obj->boolean);
}

int is_boolean(LispObject *obj) {
    return obj->type == LISP_BOOLEAN;
}

int is_number(LispObject *obj) {
    return obj->type == LISP_NUMBER;
}

int is_string(LispObject *obj) {
    return obj->type == LISP_STRING;
}

int is_symbol(LispObject *obj) {
    return obj->type == LISP_SYMBOL;
}

int is_cons(LispObject *obj) {
    return obj->type == LISP_CONS;
}

int is_list(LispObject *obj) {
    return is_nil(obj) || is_cons(obj);
}

int is_lambda(LispObject *obj) {
    return obj->type == LISP_LAMBDA;
}

int is_primitive(LispObject *obj) {
    return obj->type == LISP_PRIMITIVE;
}

int is_callable(LispObject *obj) {
    return is_lambda(obj) || is_primitive(obj);
}

int is_macro(LispObject *obj) {
    return obj->type == LISP_MACRO;
}

/* Accessors */

LispObject *car(LispObject *obj) {
    if (!is_cons(obj)) {
        lisp_error("car: not a pair");
        return LISP_NIL_OBJ;
    }
    return obj->cons.car;
}

LispObject *cdr(LispObject *obj) {
    if (!is_cons(obj)) {
        lisp_error("cdr: not a pair");
        return LISP_NIL_OBJ;
    }
    return obj->cons.cdr;
}

LispObject *cadr(LispObject *obj) { return car(cdr(obj)); }
LispObject *caddr(LispObject *obj) { return car(cdr(cdr(obj))); }
LispObject *cddr(LispObject *obj) { return cdr(cdr(obj)); }
LispObject *caar(LispObject *obj) { return car(car(obj)); }
LispObject *cdar(LispObject *obj) { return cdr(car(obj)); }

/* List utilities */

int list_length(LispObject *list) {
    int len = 0;
    while (is_cons(list)) {
        len++;
        list = cdr(list);
    }
    return len;
}

LispObject *list_reverse(LispObject *list) {
    LispObject *result = LISP_NIL_OBJ;
    while (is_cons(list)) {
        result = make_cons(car(list), result);
        list = cdr(list);
    }
    return result;
}

LispObject *list_append(LispObject *list1, LispObject *list2) {
    if (is_nil(list1)) return list2;
    return make_cons(car(list1), list_append(cdr(list1), list2));
}

LispObject *list_nth(LispObject *list, int n) {
    while (n > 0 && is_cons(list)) {
        list = cdr(list);
        n--;
    }
    if (is_cons(list)) {
        return car(list);
    }
    return LISP_NIL_OBJ;
}

LispObject *list_set_tail(LispObject *list, LispObject *tail) {
    if (is_nil(list)) {
        return tail;
    }

    /* Find the last cons cell */
    LispObject *last = list;
    while (is_cons(cdr(last))) {
        last = cdr(last);
    }

    /* Set its cdr to the tail */
    last->cons.cdr = tail;
    return list;
}

/* Symbol utilities */

int symbol_eq(LispObject *a, LispObject *b) {
    if (!is_symbol(a) || !is_symbol(b)) return 0;
    return a == b;  /* Symbols are interned */
}

int is_symbol_named(LispObject *obj, const char *name) {
    if (!is_symbol(obj)) return 0;
    return strcmp(obj->symbol.name, name) == 0;
}

/* Object comparison */

int lisp_eq(LispObject *a, LispObject *b) {
    return a == b;
}

int lisp_equal(LispObject *a, LispObject *b) {
    if (a == b) return 1;
    if (a->type != b->type) return 0;

    switch (a->type) {
        case LISP_NIL:
            return 1;
        case LISP_BOOLEAN:
            return a->boolean == b->boolean;
        case LISP_NUMBER:
            return a->number == b->number;
        case LISP_CHARACTER:
            return a->character == b->character;
        case LISP_STRING:
            return a->string.length == b->string.length &&
                   strcmp(a->string.data, b->string.data) == 0;
        case LISP_SYMBOL:
            return a == b;  /* Symbols are interned */
        case LISP_CONS:
            return lisp_equal(car(a), car(b)) && lisp_equal(cdr(a), cdr(b));
        default:
            return 0;
    }
}

/* Printing */

static void print_recursive(LispObject *obj, FILE *out, int quoted) {
    if (!obj) {
        fprintf(out, "#<null>");
        return;
    }

    switch (obj->type) {
        case LISP_NIL:
            fprintf(out, "()");
            break;

        case LISP_BOOLEAN:
            fprintf(out, obj->boolean ? "#t" : "#f");
            break;

        case LISP_NUMBER:
            if (obj->number == (long long)obj->number) {
                fprintf(out, "%lld", (long long)obj->number);
            } else {
                fprintf(out, "%g", obj->number);
            }
            break;

        case LISP_CHARACTER:
            if (quoted) {
                switch (obj->character) {
                    case '\n': fprintf(out, "#\\newline"); break;
                    case ' ':  fprintf(out, "#\\space"); break;
                    case '\t': fprintf(out, "#\\tab"); break;
                    default:   fprintf(out, "#\\%c", obj->character); break;
                }
            } else {
                fprintf(out, "%c", obj->character);
            }
            break;

        case LISP_STRING:
            if (quoted) {
                fprintf(out, "\"");
                for (size_t i = 0; i < obj->string.length; i++) {
                    char c = obj->string.data[i];
                    switch (c) {
                        case '\n': fprintf(out, "\\n"); break;
                        case '\t': fprintf(out, "\\t"); break;
                        case '\\': fprintf(out, "\\\\"); break;
                        case '"':  fprintf(out, "\\\""); break;
                        default:   fprintf(out, "%c", c); break;
                    }
                }
                fprintf(out, "\"");
            } else {
                fprintf(out, "%s", obj->string.data);
            }
            break;

        case LISP_SYMBOL:
            fprintf(out, "%s", obj->symbol.name);
            break;

        case LISP_CONS:
            fprintf(out, "(");
            print_recursive(car(obj), out, quoted);
            obj = cdr(obj);
            while (is_cons(obj)) {
                fprintf(out, " ");
                print_recursive(car(obj), out, quoted);
                obj = cdr(obj);
            }
            if (!is_nil(obj)) {
                fprintf(out, " . ");
                print_recursive(obj, out, quoted);
            }
            fprintf(out, ")");
            break;

        case LISP_LAMBDA:
            if (obj->lambda.name) {
                fprintf(out, "#<lambda:%s>", obj->lambda.name);
            } else {
                fprintf(out, "#<lambda>");
            }
            break;

        case LISP_PRIMITIVE:
            fprintf(out, "#<primitive:%s>", obj->primitive.name);
            break;

        case LISP_MACRO:
            fprintf(out, "#<macro>");
            break;

        case LISP_VECTOR:
            fprintf(out, "#(");
            for (size_t i = 0; i < obj->vector.length; i++) {
                if (i > 0) fprintf(out, " ");
                print_recursive(obj->vector.elements[i], out, quoted);
            }
            fprintf(out, ")");
            break;

        case LISP_BYTEVECTOR:
            fprintf(out, "#vu8(");
            for (size_t i = 0; i < obj->bytevector.length; i++) {
                if (i > 0) fprintf(out, " ");
                fprintf(out, "%u", obj->bytevector.bytes[i]);
            }
            fprintf(out, ")");
            break;

        case LISP_HASHTABLE:
            fprintf(out, "#<hashtable count=%zu>", obj->hashtable.count);
            break;

        case LISP_RECORD_TYPE:
            fprintf(out, "#<record-type-descriptor ");
            print_recursive(obj->record_type.name, out, quoted);
            fprintf(out, ">");
            break;

        case LISP_RECORD:
            fprintf(out, "#<record ");
            if (obj->record.rtd && is_record_type(obj->record.rtd)) {
                print_recursive(obj->record.rtd->record_type.name, out, quoted);
            }
            fprintf(out, ">");
            break;

        case LISP_CONDITION:
            fprintf(out, "#<condition ");
            print_recursive(obj->condition.type, out, quoted);
            if (obj->condition.message && !is_nil(obj->condition.message)) {
                fprintf(out, ": ");
                print_recursive(obj->condition.message, out, 0);
            }
            fprintf(out, ">");
            break;

        case LISP_VALUES:
            fprintf(out, "#<values");
            for (int i = 0; i < obj->values.count; i++) {
                fprintf(out, " ");
                print_recursive(obj->values.vals[i], out, quoted);
            }
            fprintf(out, ">");
            break;

        case LISP_PORT:
            fprintf(out, "#<%s%s-port",
                    obj->port.is_input ? "input" : "",
                    obj->port.is_output ? "output" : "");
            if (obj->port.name) {
                fprintf(out, " %s", obj->port.name);
            }
            fprintf(out, "%s>", obj->port.is_open ? "" : " closed");
            break;

        default:
            fprintf(out, "#<unknown>");
            break;
    }
}

void lisp_print(LispObject *obj) {
    print_recursive(obj, stdout, 1);
}

void lisp_print_to_buffer(LispObject *obj, char *buffer, size_t size) {
    /* Simple implementation using a temp file */
    FILE *tmp = tmpfile();
    if (!tmp) {
        snprintf(buffer, size, "#<error>");
        return;
    }
    print_recursive(obj, tmp, 1);
    long len = ftell(tmp);
    rewind(tmp);
    if ((size_t)len >= size) len = size - 1;
    fread(buffer, 1, len, tmp);
    buffer[len] = '\0';
    fclose(tmp);
}

const char *lisp_type_name(LispType type) {
    switch (type) {
        case LISP_NIL:         return "nil";
        case LISP_BOOLEAN:     return "boolean";
        case LISP_NUMBER:      return "number";
        case LISP_CHARACTER:   return "character";
        case LISP_STRING:      return "string";
        case LISP_SYMBOL:      return "symbol";
        case LISP_CONS:        return "pair";
        case LISP_LAMBDA:      return "procedure";
        case LISP_PRIMITIVE:   return "primitive";
        case LISP_MACRO:       return "macro";
        case LISP_VECTOR:      return "vector";
        case LISP_BYTEVECTOR:  return "bytevector";
        case LISP_HASHTABLE:   return "hashtable";
        case LISP_RECORD_TYPE: return "record-type-descriptor";
        case LISP_RECORD:      return "record";
        case LISP_CONDITION:   return "condition";
        case LISP_VALUES:      return "values";
        case LISP_PORT:        return "port";
        default:               return "unknown";
    }
}

/* Error handling */
void lisp_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/* ============================================================
 * R6RS: Vector Operations
 * ============================================================ */

LispObject *make_vector(size_t length, LispObject *fill) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_VECTOR;
    obj->vector.length = length;
    obj->vector.elements = (LispObject **)calloc(length, sizeof(LispObject *));
    if (!obj->vector.elements && length > 0) {
        lisp_error("Out of memory allocating vector");
        return LISP_NIL_OBJ;
    }
    for (size_t i = 0; i < length; i++) {
        obj->vector.elements[i] = fill;
    }
    return obj;
}

LispObject *make_vector_from_list(LispObject *list) {
    int len = list_length(list);
    LispObject *vec = make_vector(len, LISP_NIL_OBJ);
    LispObject *curr = list;
    for (int i = 0; i < len && is_cons(curr); i++) {
        vec->vector.elements[i] = car(curr);
        curr = cdr(curr);
    }
    return vec;
}

int is_vector(LispObject *obj) {
    return obj && obj->type == LISP_VECTOR;
}

LispObject *vector_ref(LispObject *vec, size_t index) {
    if (!is_vector(vec)) {
        lisp_error("vector-ref: not a vector");
        return LISP_NIL_OBJ;
    }
    if (index >= vec->vector.length) {
        lisp_error("vector-ref: index out of bounds");
        return LISP_NIL_OBJ;
    }
    return vec->vector.elements[index];
}

void vector_set(LispObject *vec, size_t index, LispObject *value) {
    if (!is_vector(vec)) {
        lisp_error("vector-set!: not a vector");
        return;
    }
    if (index >= vec->vector.length) {
        lisp_error("vector-set!: index out of bounds");
        return;
    }
    vec->vector.elements[index] = value;
}

size_t vector_length(LispObject *vec) {
    if (!is_vector(vec)) {
        lisp_error("vector-length: not a vector");
        return 0;
    }
    return vec->vector.length;
}

LispObject *vector_to_list(LispObject *vec) {
    if (!is_vector(vec)) {
        lisp_error("vector->list: not a vector");
        return LISP_NIL_OBJ;
    }
    LispObject *result = LISP_NIL_OBJ;
    for (int i = (int)vec->vector.length - 1; i >= 0; i--) {
        result = make_cons(vec->vector.elements[i], result);
    }
    return result;
}

/* ============================================================
 * R6RS: Bytevector Operations
 * ============================================================ */

LispObject *make_bytevector(size_t length, uint8_t fill) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_BYTEVECTOR;
    obj->bytevector.length = length;
    obj->bytevector.bytes = (uint8_t *)malloc(length);
    if (!obj->bytevector.bytes && length > 0) {
        lisp_error("Out of memory allocating bytevector");
        return LISP_NIL_OBJ;
    }
    memset(obj->bytevector.bytes, fill, length);
    return obj;
}

int is_bytevector(LispObject *obj) {
    return obj && obj->type == LISP_BYTEVECTOR;
}

uint8_t bytevector_ref(LispObject *bv, size_t index) {
    if (!is_bytevector(bv)) {
        lisp_error("bytevector-u8-ref: not a bytevector");
        return 0;
    }
    if (index >= bv->bytevector.length) {
        lisp_error("bytevector-u8-ref: index out of bounds");
        return 0;
    }
    return bv->bytevector.bytes[index];
}

void bytevector_set(LispObject *bv, size_t index, uint8_t value) {
    if (!is_bytevector(bv)) {
        lisp_error("bytevector-u8-set!: not a bytevector");
        return;
    }
    if (index >= bv->bytevector.length) {
        lisp_error("bytevector-u8-set!: index out of bounds");
        return;
    }
    bv->bytevector.bytes[index] = value;
}

size_t bytevector_length(LispObject *bv) {
    if (!is_bytevector(bv)) {
        lisp_error("bytevector-length: not a bytevector");
        return 0;
    }
    return bv->bytevector.length;
}

/* ============================================================
 * R6RS: Hashtable Operations
 * ============================================================ */

#define HASHTABLE_INITIAL_SIZE 16
#define HASHTABLE_LOAD_FACTOR 0.75

static size_t hashtable_hash(LispObject *ht, LispObject *key) {
    uint32_t h;
    switch (ht->hashtable.hash_type) {
        case 0:  /* eq hash */
            h = (uint32_t)(uintptr_t)key;
            break;
        case 1:  /* eqv hash */
            if (is_number(key)) {
                double d = key->number;
                h = (uint32_t)(*(uint64_t *)&d);
            } else if (is_symbol(key)) {
                h = key->symbol.hash;
            } else {
                h = (uint32_t)(uintptr_t)key;
            }
            break;
        case 2:  /* equal hash */
        default:
            if (is_string(key)) {
                h = hash_string(key->string.data);
            } else if (is_symbol(key)) {
                h = key->symbol.hash;
            } else if (is_number(key)) {
                double d = key->number;
                h = (uint32_t)(*(uint64_t *)&d);
            } else {
                h = (uint32_t)(uintptr_t)key;
            }
            break;
    }
    return h % ht->hashtable.capacity;
}

static int hashtable_keys_equal(LispObject *ht, LispObject *a, LispObject *b) {
    switch (ht->hashtable.hash_type) {
        case 0:  return lisp_eq(a, b);
        case 1:  return lisp_eq(a, b) || (is_number(a) && is_number(b) && a->number == b->number);
        case 2:
        default: return lisp_equal(a, b);
    }
}

LispObject *make_hashtable(int hash_type, size_t initial_capacity) {
    if (initial_capacity < HASHTABLE_INITIAL_SIZE) {
        initial_capacity = HASHTABLE_INITIAL_SIZE;
    }
    LispObject *obj = lisp_alloc();
    obj->type = LISP_HASHTABLE;
    obj->hashtable.hash_type = hash_type;
    obj->hashtable.capacity = initial_capacity;
    obj->hashtable.count = 0;
    obj->hashtable.keys = (LispObject **)calloc(initial_capacity, sizeof(LispObject *));
    obj->hashtable.values = (LispObject **)calloc(initial_capacity, sizeof(LispObject *));
    if (!obj->hashtable.keys || !obj->hashtable.values) {
        lisp_error("Out of memory allocating hashtable");
        return LISP_NIL_OBJ;
    }
    return obj;
}

int is_hashtable(LispObject *obj) {
    return obj && obj->type == LISP_HASHTABLE;
}

static void hashtable_resize(LispObject *ht) {
    size_t old_capacity = ht->hashtable.capacity;
    LispObject **old_keys = ht->hashtable.keys;
    LispObject **old_values = ht->hashtable.values;

    ht->hashtable.capacity *= 2;
    ht->hashtable.keys = (LispObject **)calloc(ht->hashtable.capacity, sizeof(LispObject *));
    ht->hashtable.values = (LispObject **)calloc(ht->hashtable.capacity, sizeof(LispObject *));
    ht->hashtable.count = 0;

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_keys[i]) {
            hashtable_set(ht, old_keys[i], old_values[i]);
        }
    }

    free(old_keys);
    free(old_values);
}

void hashtable_set(LispObject *ht, LispObject *key, LispObject *value) {
    if (!is_hashtable(ht)) {
        lisp_error("hashtable-set!: not a hashtable");
        return;
    }

    /* Check if resize needed */
    if ((double)ht->hashtable.count / ht->hashtable.capacity > HASHTABLE_LOAD_FACTOR) {
        hashtable_resize(ht);
    }

    size_t index = hashtable_hash(ht, key);
    size_t start = index;

    /* Linear probing */
    while (ht->hashtable.keys[index]) {
        if (hashtable_keys_equal(ht, ht->hashtable.keys[index], key)) {
            /* Update existing key */
            ht->hashtable.values[index] = value;
            return;
        }
        index = (index + 1) % ht->hashtable.capacity;
        if (index == start) {
            lisp_error("hashtable-set!: hashtable is full");
            return;
        }
    }

    /* Insert new key */
    ht->hashtable.keys[index] = key;
    ht->hashtable.values[index] = value;
    ht->hashtable.count++;
}

LispObject *hashtable_ref(LispObject *ht, LispObject *key, LispObject *default_val) {
    if (!is_hashtable(ht)) {
        lisp_error("hashtable-ref: not a hashtable");
        return LISP_NIL_OBJ;
    }

    size_t index = hashtable_hash(ht, key);
    size_t start = index;

    while (ht->hashtable.keys[index]) {
        if (hashtable_keys_equal(ht, ht->hashtable.keys[index], key)) {
            return ht->hashtable.values[index];
        }
        index = (index + 1) % ht->hashtable.capacity;
        if (index == start) break;
    }

    return default_val;
}

int hashtable_contains(LispObject *ht, LispObject *key) {
    if (!is_hashtable(ht)) return 0;

    size_t index = hashtable_hash(ht, key);
    size_t start = index;

    while (ht->hashtable.keys[index]) {
        if (hashtable_keys_equal(ht, ht->hashtable.keys[index], key)) {
            return 1;
        }
        index = (index + 1) % ht->hashtable.capacity;
        if (index == start) break;
    }

    return 0;
}

void hashtable_delete(LispObject *ht, LispObject *key) {
    if (!is_hashtable(ht)) {
        lisp_error("hashtable-delete!: not a hashtable");
        return;
    }

    size_t index = hashtable_hash(ht, key);
    size_t start = index;

    while (ht->hashtable.keys[index]) {
        if (hashtable_keys_equal(ht, ht->hashtable.keys[index], key)) {
            ht->hashtable.keys[index] = NULL;
            ht->hashtable.values[index] = NULL;
            ht->hashtable.count--;
            return;
        }
        index = (index + 1) % ht->hashtable.capacity;
        if (index == start) break;
    }
}

size_t hashtable_size(LispObject *ht) {
    if (!is_hashtable(ht)) return 0;
    return ht->hashtable.count;
}

LispObject *hashtable_keys(LispObject *ht) {
    if (!is_hashtable(ht)) {
        lisp_error("hashtable-keys: not a hashtable");
        return LISP_NIL_OBJ;
    }

    LispObject *result = LISP_NIL_OBJ;
    for (size_t i = 0; i < ht->hashtable.capacity; i++) {
        if (ht->hashtable.keys[i]) {
            result = make_cons(ht->hashtable.keys[i], result);
        }
    }
    return result;
}

LispObject *hashtable_values(LispObject *ht) {
    if (!is_hashtable(ht)) {
        lisp_error("hashtable-values: not a hashtable");
        return LISP_NIL_OBJ;
    }

    LispObject *result = LISP_NIL_OBJ;
    for (size_t i = 0; i < ht->hashtable.capacity; i++) {
        if (ht->hashtable.keys[i]) {
            result = make_cons(ht->hashtable.values[i], result);
        }
    }
    return result;
}

/* ============================================================
 * R6RS: Record Types
 * ============================================================ */

LispObject *make_record_type(LispObject *name, LispObject *parent, LispObject *fields) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_RECORD_TYPE;
    obj->record_type.name = name;
    obj->record_type.parent = parent;
    obj->record_type.fields = fields;
    obj->record_type.field_count = list_length(fields);
    obj->record_type.sealed = 0;
    obj->record_type.opaque = 0;
    return obj;
}

int is_record_type(LispObject *obj) {
    return obj && obj->type == LISP_RECORD_TYPE;
}

LispObject *make_record(LispObject *rtd) {
    if (!is_record_type(rtd)) {
        lisp_error("make-record: not a record type descriptor");
        return LISP_NIL_OBJ;
    }

    /* Count total fields including parent */
    int total_fields = rtd->record_type.field_count;
    LispObject *parent = rtd->record_type.parent;
    while (is_record_type(parent)) {
        total_fields += parent->record_type.field_count;
        parent = parent->record_type.parent;
    }

    LispObject *obj = lisp_alloc();
    obj->type = LISP_RECORD;
    obj->record.rtd = rtd;
    obj->record.fields = (LispObject **)calloc(total_fields, sizeof(LispObject *));
    if (!obj->record.fields && total_fields > 0) {
        lisp_error("Out of memory allocating record");
        return LISP_NIL_OBJ;
    }

    /* Initialize all fields to unspecified (nil) */
    for (int i = 0; i < total_fields; i++) {
        obj->record.fields[i] = LISP_NIL_OBJ;
    }

    return obj;
}

int is_record(LispObject *obj) {
    return obj && obj->type == LISP_RECORD;
}

LispObject *record_rtd(LispObject *rec) {
    if (!is_record(rec)) {
        lisp_error("record-rtd: not a record");
        return LISP_NIL_OBJ;
    }
    return rec->record.rtd;
}

LispObject *record_ref(LispObject *rec, int field_index) {
    if (!is_record(rec)) {
        lisp_error("record-ref: not a record");
        return LISP_NIL_OBJ;
    }
    return rec->record.fields[field_index];
}

void record_set(LispObject *rec, int field_index, LispObject *value) {
    if (!is_record(rec)) {
        lisp_error("record-set!: not a record");
        return;
    }
    rec->record.fields[field_index] = value;
}

/* ============================================================
 * R6RS: Conditions (Exception Types)
 * ============================================================ */

LispObject *make_condition(LispObject *type, LispObject *message, LispObject *irritants) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_CONDITION;
    obj->condition.type = type;
    obj->condition.message = message;
    obj->condition.irritants = irritants;
    obj->condition.who = LISP_NIL_OBJ;
    return obj;
}

int is_condition(LispObject *obj) {
    return obj && obj->type == LISP_CONDITION;
}

/* ============================================================
 * R6RS: Multiple Values
 * ============================================================ */

LispObject *make_values(LispObject **vals, int count) {
    /* Single value - just return it */
    if (count == 1) {
        return vals[0];
    }

    LispObject *obj = lisp_alloc();
    obj->type = LISP_VALUES;
    obj->values.count = count;
    obj->values.vals = (LispObject **)malloc(count * sizeof(LispObject *));
    if (!obj->values.vals && count > 0) {
        lisp_error("Out of memory allocating values");
        return LISP_NIL_OBJ;
    }
    for (int i = 0; i < count; i++) {
        obj->values.vals[i] = vals[i];
    }
    return obj;
}

int is_values(LispObject *obj) {
    return obj && obj->type == LISP_VALUES;
}

/* ============================================================
 * R6RS: Ports (I/O)
 * ============================================================ */

LispObject *make_port(void *stream, int is_input, int is_output, int is_binary, const char *name) {
    LispObject *obj = lisp_alloc();
    obj->type = LISP_PORT;
    obj->port.stream = stream;
    obj->port.is_input = is_input;
    obj->port.is_output = is_output;
    obj->port.is_binary = is_binary;
    obj->port.is_open = 1;
    obj->port.name = name ? strdup(name) : NULL;
    return obj;
}

int is_port(LispObject *obj) {
    return obj && obj->type == LISP_PORT;
}

int is_input_port(LispObject *obj) {
    return is_port(obj) && obj->port.is_input;
}

int is_output_port(LispObject *obj) {
    return is_port(obj) && obj->port.is_output;
}
