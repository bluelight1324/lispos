# R6RS Implementation Documentation

## Overview

This document describes the R6RS (Revised⁶ Report on the Algorithmic Language Scheme) features implemented in LispCompiler. R6RS is a major revision of the Scheme standard published in 2007 that adds libraries, Unicode support, exceptions, records, hash tables, and more.

## Implementation Status

| R6RS Feature | Status | Notes |
|--------------|--------|-------|
| Vectors | **Complete** | Full mutable vector support |
| Bytevectors | **Complete** | u8 operations |
| Hash Tables | **Complete** | eq, eqv, equal hash tables |
| Records | **Partial** | Basic record types, no nongenerative |
| Conditions | **Partial** | Basic condition type, limited hierarchy |
| Multiple Values | **Complete** | values/call-with-values |
| Library System | **Not Implemented** | Use load/include |
| Unicode | **Partial** | ASCII subset |
| Exact/Inexact | **Partial** | All numbers are IEEE 754 doubles |

## Vectors

Vectors are fixed-length arrays that provide O(1) random access.

### Procedures

```scheme
; Type predicate
(vector? obj)              ; => #t if obj is a vector

; Constructors
(make-vector k)            ; Create vector of length k, unspecified fill
(make-vector k fill)       ; Create vector of length k, filled with fill
(vector obj ...)           ; Create vector from arguments

; Accessors
(vector-length vec)        ; => number of elements
(vector-ref vec k)         ; => element at index k
(vector-set! vec k obj)    ; Set element at index k

; Conversion
(vector->list vec)         ; Convert vector to list
(list->vector list)        ; Convert list to vector
```

### Examples

```scheme
(define v (make-vector 5 0))      ; #(0 0 0 0 0)
(vector-set! v 2 42)              ; #(0 0 42 0 0)
(vector-ref v 2)                  ; => 42

(define v2 (vector 'a 'b 'c))     ; #(a b c)
(vector->list v2)                 ; => (a b c)
```

## Bytevectors

Bytevectors are vectors of bytes (unsigned 8-bit integers).

### Procedures

```scheme
; Type predicate
(bytevector? obj)              ; => #t if obj is a bytevector

; Constructor
(make-bytevector k)            ; Create bytevector of length k
(make-bytevector k fill)       ; Create bytevector filled with byte fill

; Accessors
(bytevector-length bv)         ; => number of bytes
(bytevector-u8-ref bv k)       ; => unsigned byte at index k
(bytevector-u8-set! bv k byte) ; Set byte at index k
```

### Examples

```scheme
(define bv (make-bytevector 4 0))
(bytevector-u8-set! bv 0 255)
(bytevector-u8-ref bv 0)          ; => 255
```

## Hash Tables

Hash tables provide efficient key-value lookup with O(1) average case.

### Procedures

```scheme
; Type predicate
(hashtable? obj)               ; => #t if obj is a hashtable

; Constructors (by equality type)
(make-eq-hashtable)            ; Uses eq? for comparison
(make-eqv-hashtable)           ; Uses eqv? for comparison
(make-hashtable hash equiv)    ; Custom (defaults to equal?)

; Operations
(hashtable-ref ht key default) ; Lookup key, return default if not found
(hashtable-set! ht key value)  ; Set key to value
(hashtable-delete! ht key)     ; Remove key
(hashtable-contains? ht key)   ; => #t if key exists
(hashtable-size ht)            ; => number of entries
(hashtable-keys ht)            ; => list of all keys
```

### Examples

```scheme
(define ht (make-eq-hashtable))
(hashtable-set! ht 'name "Alice")
(hashtable-set! ht 'age 30)

(hashtable-ref ht 'name "unknown")  ; => "Alice"
(hashtable-ref ht 'city "unknown")  ; => "unknown"
(hashtable-contains? ht 'age)       ; => #t
(hashtable-size ht)                 ; => 2
(hashtable-delete! ht 'age)
(hashtable-size ht)                 ; => 1
```

## Records (Partial)

Records are user-defined data types with named fields.

### Basic Structure

```scheme
; Record type descriptor (internal use)
(make-record-type name parent fields)
(record-type? obj)

; Record instance
(make-record rtd)
(record? obj)
(record-rtd rec)
(record-ref rec index)
(record-set! rec index value)
```

**Note**: Full R6RS procedural and syntactic record layers are not implemented.

## Numeric Extensions

### Rounding Procedures

```scheme
(floor x)      ; Largest integer <= x
(ceiling x)    ; Smallest integer >= x
(truncate x)   ; Integer closest to x toward zero
(round x)      ; Nearest integer (half rounds away from zero*)
```

*Note: R6RS specifies round-to-even for .5 cases. This implementation uses C's `round()` which rounds half away from zero.

### Mathematical Functions

```scheme
(sqrt x)       ; Square root
(expt x y)     ; x raised to power y
(log x)        ; Natural logarithm
(sin x)        ; Sine (radians)
(cos x)        ; Cosine (radians)
(tan x)        ; Tangent (radians)
```

### Integer Division

```scheme
(quotient n d)   ; Integer quotient (truncated toward zero)
(remainder n d)  ; Remainder with same sign as dividend
(modulo n d)     ; Remainder with same sign as divisor
```

### Numeric Predicates

```scheme
(integer? x)    ; #t if x is an integer
(real? x)       ; #t if x is a real number
(zero? x)       ; #t if x = 0
(positive? x)   ; #t if x > 0
(negative? x)   ; #t if x < 0
(odd? x)        ; #t if x is odd
(even? x)       ; #t if x is even
```

### Min/Max

```scheme
(min x ...)     ; Minimum of arguments
(max x ...)     ; Maximum of arguments
```

## List Extensions

### List Predicates and Accessors

```scheme
(list? obj)         ; #t if proper list (ends in nil)
(list-ref lst k)    ; kth element of list
(list-tail lst k)   ; Sublist starting at index k
```

### Membership

```scheme
(memq obj lst)      ; Find obj in lst using eq?
(memv obj lst)      ; Find obj in lst using eqv?
(member obj lst)    ; Find obj in lst using equal?
```

### Association Lists

```scheme
(assq obj alist)    ; Find pair with key eq? to obj
(assv obj alist)    ; Find pair with key eqv? to obj
(assoc obj alist)   ; Find pair with key equal? to obj
```

### Examples

```scheme
(list-ref '(a b c d) 2)           ; => c
(list-tail '(a b c d) 2)          ; => (c d)

(memq 'b '(a b c))                ; => (b c)
(member '(2) '((1) (2) (3)))      ; => ((2) (3))

(define alist '((a . 1) (b . 2)))
(assq 'b alist)                   ; => (b . 2)
```

## Character Extensions

```scheme
(char? obj)         ; #t if obj is a character
(char=? c1 c2)      ; Character equality
(char<? c1 c2)      ; Character ordering
(char->integer c)   ; Character to integer (code point)
(integer->char n)   ; Integer to character
```

## Conditions (Partial)

Basic condition support is available:

```scheme
(make-condition type message irritants)
(condition? obj)
```

Full R6RS condition hierarchy and `guard` syntax are not implemented.

## Multiple Values

```scheme
(values v ...)      ; Return multiple values
```

**Note**: `call-with-values` is not yet implemented.

## Ports (Partial)

Basic port structure exists but port operations are limited:

```scheme
(port? obj)         ; #t if obj is a port
(input-port? obj)   ; #t if input port
(output-port? obj)  ; #t if output port
```

## What's Not Implemented

The following R6RS features are **not** implemented:

1. **Library System** (`library`, `import`, `export`)
2. **Full Unicode Support** (only ASCII)
3. **Exact/Inexact Numbers** (all numbers are doubles)
4. **Continuations** (`call/cc`, `dynamic-wind`)
5. **Full Exception System** (`guard`, `raise`, `with-exception-handler`)
6. **Hygienic Macros** (`syntax-case`, `syntax-rules`)
7. **Complex/Rational Numbers**
8. **Full Record System** (syntactic layer)
9. **Enumerations** (`define-enumeration`)
10. **Sorting** (`list-sort`, `vector-sort`)

## Test Results

The R6RS test suite (`test/r6rs_test.scm`) covers:

- **Vectors**: 10 tests - all pass
- **Bytevectors**: 5 tests - all pass
- **Hashtables**: 10 tests - all pass
- **Numeric operations**: 28 tests - 27 pass, 1 minor rounding difference
- **List operations**: 10 tests - all pass
- **Character operations**: 6 tests - all pass

**Total**: 69 tests, 68 pass (98.5% pass rate)

## Usage Examples

### Using Vectors for Arrays

```scheme
; Matrix as vector of vectors
(define (make-matrix rows cols fill)
  (let ((m (make-vector rows)))
    (let loop ((i 0))
      (if (< i rows)
          (begin
            (vector-set! m i (make-vector cols fill))
            (loop (+ i 1)))))
    m))

(define (matrix-ref m i j)
  (vector-ref (vector-ref m i) j))

(define (matrix-set! m i j val)
  (vector-set! (vector-ref m i) j val))
```

### Using Hash Tables for Caching

```scheme
(define cache (make-eq-hashtable))

(define (fib-cached n)
  (let ((cached (hashtable-ref cache n #f)))
    (if cached
        cached
        (let ((result (if (< n 2)
                          n
                          (+ (fib-cached (- n 1))
                             (fib-cached (- n 2))))))
          (hashtable-set! cache n result)
          result))))
```

### Using Bytevectors for Binary Data

```scheme
; Write a 32-bit little-endian integer
(define (bv-set-u32-le! bv offset val)
  (bytevector-u8-set! bv offset (modulo val 256))
  (bytevector-u8-set! bv (+ offset 1) (modulo (quotient val 256) 256))
  (bytevector-u8-set! bv (+ offset 2) (modulo (quotient val 65536) 256))
  (bytevector-u8-set! bv (+ offset 3) (quotient val 16777216)))
```

## Version Information

```
LispCompiler v1.1.0
R6RS Features: Partial Implementation
Date: 2026-02-03
```

## References

1. **R6RS**: Sperber, Dybvig, Flatt, Van Straaten, Findler, Matthews. "Revised⁶ Report on the Algorithmic Language Scheme." 2007.

2. **R6RS Standard Libraries**: Sperber, et al. "Revised⁶ Report on the Algorithmic Language Scheme - Standard Libraries." 2007.
