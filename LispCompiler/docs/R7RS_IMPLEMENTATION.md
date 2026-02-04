# R7RS Implementation Documentation

## Overview

This document describes the R7RS-small (Revised⁷ Report on the Algorithmic Language Scheme) features implemented in LispCompiler. R7RS-small was published in 2013 as a more conservative evolution of Scheme, focusing on practical additions while maintaining simplicity.

## Implementation Status

| R7RS Feature | Status | Notes |
|--------------|--------|-------|
| `when`/`unless` | **Complete** | Conditional forms |
| `case-lambda` | **Complete** | Multi-arity procedures |
| `do` | **Complete** | Iteration construct |
| `case` | **Complete** | With `=>` support |
| Multiple values | **Complete** | `values`, `call-with-values` |
| `let-values` | **Complete** | Multiple value binding |
| Higher-order functions | **Complete** | `map`, `for-each`, `filter`, `fold` |
| `define-library` | **Not Implemented** | Use `load` instead |
| `include` | **Not Implemented** | Use `load` instead |
| `cond-expand` | **Not Implemented** | Feature detection |
| `guard` | **Partial** | Basic form only |
| Parameters | **Not Implemented** | `make-parameter`, `parameterize` |
| String ports | **Not Implemented** | `open-input-string`, etc. |

## Special Forms

### `when` and `unless`

Conditional execution without else clause.

```scheme
(when test expr ...)      ; Execute exprs if test is true
(unless test expr ...)    ; Execute exprs if test is false
```

Examples:
```scheme
(when (> x 0)
  (display "positive")
  (newline))

(unless (null? lst)
  (process lst))
```

### `case-lambda`

Create procedures that dispatch on argument count.

```scheme
(case-lambda
  [() expr ...]           ; 0 arguments
  [(x) expr ...]          ; 1 argument
  [(x y) expr ...]        ; 2 arguments
  [(x . rest) expr ...])  ; 1+ arguments (rest parameter)
```

Example:
```scheme
(define add
  (case-lambda
    (() 0)
    ((x) x)
    ((x y) (+ x y))
    ((x y . rest) (apply + x y rest))))

(add)       ; => 0
(add 5)     ; => 5
(add 3 4)   ; => 7
```

### `do`

General iteration construct.

```scheme
(do ((var init step) ...)
    (test result ...)
  command ...)
```

Example:
```scheme
; Sum numbers 1 to 10
(do ((i 1 (+ i 1))
     (sum 0 (+ sum i)))
    ((> i 10) sum))  ; => 55

; Build a list
(do ((i 0 (+ i 1))
     (lst '() (cons i lst)))
    ((= i 5) (reverse lst)))  ; => (0 1 2 3 4)
```

### `case`

Multi-way conditional based on value matching.

```scheme
(case key
  ((datum ...) expr ...)
  ...
  (else expr ...))
```

Example:
```scheme
(case (get-month)
  ((jan feb mar) 'winter)
  ((apr may jun) 'spring)
  ((jul aug sep) 'summer)
  ((oct nov dec) 'fall)
  (else 'unknown))

; With => syntax
(case x
  ((1 2 3) => (lambda (v) (* v 10)))
  (else 0))
```

### `let-values` and `let*-values`

Bind multiple values from expressions.

```scheme
(let-values (((a b) (values 1 2)))
  (+ a b))  ; => 3

(let*-values (((a b) (values 1 2))
              ((c) (+ a b)))
  c)  ; => 3
```

## Multiple Values

### `values`

Return multiple values from a procedure.

```scheme
(values v ...)
```

Example:
```scheme
(define (div-and-mod n d)
  (values (quotient n d) (remainder n d)))

(div-and-mod 17 5)  ; => #<values 3 2>
```

### `call-with-values`

Receive multiple values from a producer and pass to a consumer.

```scheme
(call-with-values producer consumer)
```

Example:
```scheme
(call-with-values
  (lambda () (values 3 4))
  (lambda (a b) (+ a b)))  ; => 7

(call-with-values
  (lambda () (values 1 2 3))
  list)  ; => (1 2 3)
```

## Higher-Order Functions

### `map`

Apply procedure to elements of lists.

```scheme
(map proc list ...)
```

Examples:
```scheme
(map (lambda (x) (* x 2)) '(1 2 3))
; => (2 4 6)

(map + '(1 2 3) '(10 20 30))
; => (11 22 33)
```

### `for-each`

Apply procedure for side effects.

```scheme
(for-each proc list ...)
```

Example:
```scheme
(for-each display '("a" "b" "c"))
; Prints: abc
```

### `filter`

Select elements satisfying a predicate.

```scheme
(filter pred list)
```

Example:
```scheme
(filter even? '(1 2 3 4 5 6))
; => (2 4 6)

(filter (lambda (x) (> x 3)) '(1 5 2 7 3 8))
; => (5 7 8)
```

### `fold` (fold-left)

Left-associative fold.

```scheme
(fold proc init list)
```

Example:
```scheme
(fold + 0 '(1 2 3 4 5))  ; => 15
(fold cons '() '(1 2 3))  ; => (3 2 1)
(fold (lambda (x acc) (+ acc 1)) 0 '(a b c d))  ; => 4 (length)
```

### `fold-right`

Right-associative fold.

```scheme
(fold-right proc init list)
```

Example:
```scheme
(fold-right cons '() '(1 2 3))  ; => (1 2 3)
(fold-right + 0 '(1 2 3 4 5))   ; => 15
```

## List Operations

### `make-list`

Create a list of specified length.

```scheme
(make-list k)        ; List of k () elements
(make-list k fill)   ; List of k fill elements
```

Example:
```scheme
(make-list 5)      ; => (() () () () ())
(make-list 3 'x)   ; => (x x x)
```

### `list-copy`

Create a shallow copy of a list.

```scheme
(list-copy list)
```

### `list-set!`

Mutate an element of a list.

```scheme
(list-set! list k obj)
```

Example:
```scheme
(define lst (list 'a 'b 'c))
(list-set! lst 1 'x)
lst  ; => (a x c)
```

## Vector Operations

### `vector-copy`

Copy a vector or subvector.

```scheme
(vector-copy vec)
(vector-copy vec start)
(vector-copy vec start end)
```

Example:
```scheme
(define v (vector 1 2 3 4 5))
(vector-copy v)      ; => #(1 2 3 4 5)
(vector-copy v 2)    ; => #(3 4 5)
(vector-copy v 1 4)  ; => #(2 3 4)
```

### `vector-fill!`

Fill a vector with a value.

```scheme
(vector-fill! vec fill)
(vector-fill! vec fill start)
(vector-fill! vec fill start end)
```

### `vector-append`

Concatenate vectors.

```scheme
(vector-append vec ...)
```

Example:
```scheme
(vector-append (vector 1 2) (vector 3 4))
; => #(1 2 3 4)
```

## String Operations

### `string-copy`

Copy a string or substring.

```scheme
(string-copy str)
(string-copy str start)
(string-copy str start end)
```

### `substring`

Extract a substring.

```scheme
(substring str start end)
```

Example:
```scheme
(substring "hello world" 0 5)  ; => "hello"
(substring "hello world" 6 11) ; => "world"
```

### String Comparison

```scheme
(string=? str1 str2)   ; Equality
(string<? str1 str2)   ; Lexicographic ordering
```

## Numeric Operations

### `square`

Square a number.

```scheme
(square x)  ; => x*x
```

### `exact` and `inexact`

Convert between exact and inexact numbers.

```scheme
(exact 3.7)    ; => 3
(inexact 5)    ; => 5.0
```

### `finite?`, `infinite?`, `nan?`

Test for special floating-point values.

```scheme
(finite? 42)          ; => #t
(infinite? (/ 1 0))   ; Implementation-dependent
(nan? (sqrt -1))      ; Implementation-dependent
```

### `gcd` and `lcm`

Greatest common divisor and least common multiple.

```scheme
(gcd 12 18)      ; => 6
(gcd 12 18 8)    ; => 2
(lcm 12 18)      ; => 36
(lcm 3 4 6)      ; => 12
```

## Equivalence Predicates

### `boolean=?`

Test if all arguments are the same boolean.

```scheme
(boolean=? #t #t #t)  ; => #t
(boolean=? #f #f)     ; => #t
(boolean=? #t #f)     ; => #f
```

### `symbol=?`

Test if all arguments are the same symbol.

```scheme
(symbol=? 'foo 'foo 'foo)  ; => #t
(symbol=? 'foo 'bar)       ; => #f
```

## What's Not Implemented

The following R7RS-small features are **not** implemented:

1. **Library System** (`define-library`, `import`, `export`)
2. **`include` and `include-ci`** - File inclusion
3. **`cond-expand`** - Feature-based conditional
4. **Parameters** (`make-parameter`, `parameterize`)
5. **Full Exception System** (`guard` with proper exception handling)
6. **String Ports** (`open-input-string`, `open-output-string`)
7. **`syntax-rules`** - Hygienic macros
8. **`define-record-type`** - Syntactic record definition
9. **`delay` / `force`** - Lazy evaluation
10. **Binary I/O** - `read-u8`, `write-u8`, etc.

## Test Results

The R7RS test suite (`test/r7rs_test.scm`) covers:

- **Special forms**: 13 tests - all pass
- **Multiple values**: 2 tests - all pass
- **List operations**: 5 tests - all pass
- **Vector operations**: 11 tests - all pass
- **String operations**: 6 tests - all pass
- **Numeric operations**: 9 tests - all pass
- **Equivalence predicates**: 5 tests - all pass
- **Higher-order functions**: 6 tests - all pass

**Total**: 56 tests, 56 pass (100% pass rate)

## Usage Examples

### Using case-lambda for Optional Arguments

```scheme
(define greet
  (case-lambda
    (() (greet "World"))
    ((name) (string-append "Hello, " name "!"))))

(greet)         ; => "Hello, World!"
(greet "Alice") ; => "Hello, Alice!"
```

### Using do for Complex Iteration

```scheme
; Fibonacci sequence
(define (fib-list n)
  (do ((i 0 (+ i 1))
       (a 0 b)
       (b 1 (+ a b))
       (result '() (cons a result)))
      ((= i n) (reverse result))))

(fib-list 10)  ; => (0 1 1 2 3 5 8 13 21 34)
```

### Using Higher-Order Functions

```scheme
; Pipeline of operations
(define (process-data data)
  (fold +
        0
        (map square
             (filter positive? data))))

(process-data '(-3 1 -2 4 -5 2))  ; => 21 (1² + 4² + 2²)
```

### Using Multiple Values

```scheme
(define (stats lst)
  (let ((n (length lst))
        (sum (fold + 0 lst)))
    (values sum (/ sum n) n)))

(call-with-values
  (lambda () (stats '(10 20 30 40 50)))
  (lambda (sum avg count)
    (list 'sum: sum 'avg: avg 'n: count)))
; => (sum: 150 avg: 30 n: 5)
```

## Differences from R7RS-small Standard

1. **No library system**: Use `load` for file inclusion
2. **No hygienic macros**: Use `defmacro` instead
3. **No parameters**: Dynamic variables not supported
4. **Simplified exception handling**: `guard` doesn't catch actual exceptions
5. **No binary I/O**: Text I/O only
6. **No exact/inexact distinction**: All numbers are IEEE 754 doubles

## Version Information

```
LispCompiler v1.2.0
R7RS Features: Partial Implementation (R7RS-small subset)
Date: 2026-02-03
```

## References

1. **R7RS-small**: Shinn, Cowan, et al. "Revised⁷ Report on the Algorithmic Language Scheme." 2013.

2. **SRFI-1**: List Library - Many R7RS list procedures originate here

3. **SRFI-9**: Defining Record Types - Basis for R7RS records
