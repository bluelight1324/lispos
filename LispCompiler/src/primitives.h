/*
 * primitives.h - Built-in Lisp Functions
 *
 * Registers all primitive functions in an environment.
 */

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "lisp.h"
#include "env.h"

/* Register all primitive functions in an environment */
void register_primitives(Environment *env);

/* Individual primitives (can be called directly if needed) */

/* List operations */
LispObject *prim_car(LispObject *args);
LispObject *prim_cdr(LispObject *args);
LispObject *prim_cons(LispObject *args);
LispObject *prim_list(LispObject *args);
LispObject *prim_length(LispObject *args);
LispObject *prim_append(LispObject *args);
LispObject *prim_reverse(LispObject *args);

/* Type predicates */
LispObject *prim_null_p(LispObject *args);
LispObject *prim_pair_p(LispObject *args);
LispObject *prim_number_p(LispObject *args);
LispObject *prim_symbol_p(LispObject *args);
LispObject *prim_string_p(LispObject *args);
LispObject *prim_procedure_p(LispObject *args);
LispObject *prim_boolean_p(LispObject *args);

/* Arithmetic */
LispObject *prim_add(LispObject *args);
LispObject *prim_sub(LispObject *args);
LispObject *prim_mul(LispObject *args);
LispObject *prim_div(LispObject *args);
LispObject *prim_mod(LispObject *args);
LispObject *prim_abs(LispObject *args);

/* Comparison */
LispObject *prim_eq_num(LispObject *args);
LispObject *prim_lt(LispObject *args);
LispObject *prim_gt(LispObject *args);
LispObject *prim_le(LispObject *args);
LispObject *prim_ge(LispObject *args);
LispObject *prim_eq(LispObject *args);
LispObject *prim_equal(LispObject *args);

/* Boolean */
LispObject *prim_not(LispObject *args);

/* I/O */
LispObject *prim_display(LispObject *args);
LispObject *prim_newline(LispObject *args);
LispObject *prim_print(LispObject *args);

/* String operations */
LispObject *prim_string_length(LispObject *args);
LispObject *prim_string_append(LispObject *args);
LispObject *prim_string_ref(LispObject *args);
LispObject *prim_number_to_string(LispObject *args);
LispObject *prim_string_to_number(LispObject *args);
LispObject *prim_symbol_to_string(LispObject *args);
LispObject *prim_string_to_symbol(LispObject *args);

/* Utility */
LispObject *prim_apply(LispObject *args);
LispObject *prim_error(LispObject *args);

/* R6RS: Vectors */
LispObject *prim_vector_p(LispObject *args);
LispObject *prim_make_vector(LispObject *args);
LispObject *prim_vector(LispObject *args);
LispObject *prim_vector_length(LispObject *args);
LispObject *prim_vector_ref(LispObject *args);
LispObject *prim_vector_set(LispObject *args);
LispObject *prim_vector_to_list(LispObject *args);
LispObject *prim_list_to_vector(LispObject *args);

/* R6RS: Bytevectors */
LispObject *prim_bytevector_p(LispObject *args);
LispObject *prim_make_bytevector(LispObject *args);
LispObject *prim_bytevector_length(LispObject *args);
LispObject *prim_bytevector_u8_ref(LispObject *args);
LispObject *prim_bytevector_u8_set(LispObject *args);

/* R6RS: Hashtables */
LispObject *prim_hashtable_p(LispObject *args);
LispObject *prim_make_eq_hashtable(LispObject *args);
LispObject *prim_make_eqv_hashtable(LispObject *args);
LispObject *prim_make_hashtable(LispObject *args);
LispObject *prim_hashtable_ref(LispObject *args);
LispObject *prim_hashtable_set(LispObject *args);
LispObject *prim_hashtable_delete(LispObject *args);
LispObject *prim_hashtable_contains(LispObject *args);
LispObject *prim_hashtable_size(LispObject *args);
LispObject *prim_hashtable_keys(LispObject *args);

/* R6RS: Additional numeric */
LispObject *prim_floor(LispObject *args);
LispObject *prim_ceiling(LispObject *args);
LispObject *prim_truncate(LispObject *args);
LispObject *prim_round(LispObject *args);
LispObject *prim_sqrt(LispObject *args);
LispObject *prim_expt(LispObject *args);
LispObject *prim_log(LispObject *args);
LispObject *prim_sin(LispObject *args);
LispObject *prim_cos(LispObject *args);
LispObject *prim_tan(LispObject *args);
LispObject *prim_quotient(LispObject *args);
LispObject *prim_remainder(LispObject *args);
LispObject *prim_modulo(LispObject *args);
LispObject *prim_integer_p(LispObject *args);
LispObject *prim_real_p(LispObject *args);
LispObject *prim_zero_p(LispObject *args);
LispObject *prim_positive_p(LispObject *args);
LispObject *prim_negative_p(LispObject *args);
LispObject *prim_odd_p(LispObject *args);
LispObject *prim_even_p(LispObject *args);
LispObject *prim_min(LispObject *args);
LispObject *prim_max(LispObject *args);

/* R6RS: Additional list operations */
LispObject *prim_list_p(LispObject *args);
LispObject *prim_list_ref(LispObject *args);
LispObject *prim_list_tail(LispObject *args);
LispObject *prim_memq(LispObject *args);
LispObject *prim_memv(LispObject *args);
LispObject *prim_member(LispObject *args);
LispObject *prim_assq(LispObject *args);
LispObject *prim_assv(LispObject *args);
LispObject *prim_assoc(LispObject *args);

/* R6RS: Characters */
LispObject *prim_char_p(LispObject *args);
LispObject *prim_char_eq(LispObject *args);
LispObject *prim_char_lt(LispObject *args);
LispObject *prim_char_to_integer(LispObject *args);
LispObject *prim_integer_to_char(LispObject *args);

/* R7RS: Multiple values */
LispObject *prim_values(LispObject *args);
LispObject *prim_call_with_values(LispObject *args);

/* R7RS: List operations */
LispObject *prim_make_list(LispObject *args);
LispObject *prim_list_copy(LispObject *args);
LispObject *prim_list_set(LispObject *args);

/* R7RS: Vector operations */
LispObject *prim_vector_copy(LispObject *args);
LispObject *prim_vector_fill(LispObject *args);
LispObject *prim_vector_append(LispObject *args);

/* R7RS: String operations */
LispObject *prim_string_copy(LispObject *args);
LispObject *prim_substring(LispObject *args);
LispObject *prim_string_eq(LispObject *args);
LispObject *prim_string_lt(LispObject *args);

/* R7RS: Numeric operations */
LispObject *prim_square(LispObject *args);
LispObject *prim_exact(LispObject *args);
LispObject *prim_inexact(LispObject *args);
LispObject *prim_finite_p(LispObject *args);
LispObject *prim_infinite_p(LispObject *args);
LispObject *prim_nan_p(LispObject *args);
LispObject *prim_gcd(LispObject *args);
LispObject *prim_lcm(LispObject *args);

/* R7RS: Equivalence */
LispObject *prim_boolean_eq(LispObject *args);
LispObject *prim_symbol_eq(LispObject *args);

/* R7RS: Higher-order functions */
LispObject *prim_map(LispObject *args);
LispObject *prim_for_each(LispObject *args);
LispObject *prim_filter(LispObject *args);
LispObject *prim_fold(LispObject *args);
LispObject *prim_fold_right(LispObject *args);

#endif /* PRIMITIVES_H */
