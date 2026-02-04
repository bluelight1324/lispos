# Lisp Version Specification

## Overview

This implementation is a **Scheme-like dialect** of Lisp, most closely resembling
**R5RS Scheme** (Revised⁵ Report on the Algorithmic Language Scheme) with some
simplifications and extensions.

## Dialect Classification

| Aspect | Classification |
|--------|----------------|
| **Family** | Lisp |
| **Dialect** | Scheme-like |
| **Closest Standard** | R5RS (1998) + R6RS features |
| **Lexical Scoping** | Yes (like Scheme) |
| **Proper Tail Calls** | Yes |
| **First-class Continuations** | No (simplified) |
| **Hygienic Macros** | No (uses defmacro) |

## Version Identifier

```
LispCompiler v1.0.0
Dialect: Scheme-R5RS-subset
```

## Why Scheme (Not Common Lisp)?

This implementation follows the Scheme tradition rather than Common Lisp because:

1. **Lexical Scoping**: Variables are resolved based on where functions are defined,
   not where they are called (Scheme style)

2. **Single Namespace**: Functions and variables share the same namespace
   (Lisp-1 like Scheme, not Lisp-2 like Common Lisp)

3. **Minimal Core**: Small set of special forms with maximum expressiveness

4. **Tail Call Optimization**: Proper tail calls are guaranteed

5. **Boolean Values**: Uses `#t` and `#f` (Scheme) not `T` and `NIL` (Common Lisp)

## Comparison with Standard Schemes

### R5RS Compliance

| Feature | R5RS | This Implementation | Notes |
|---------|------|---------------------|-------|
| `lambda` | ✓ | ✓ | Full support |
| `define` | ✓ | ✓ | Variables and functions |
| `if` | ✓ | ✓ | Two and three argument forms |
| `cond` | ✓ | ✓ | Including `else` and `=>` |
| `let` | ✓ | ✓ | Parallel binding |
| `let*` | ✓ | ✓ | Sequential binding |
| `letrec` | ✓ | ✓ | Recursive binding |
| Named `let` | ✓ | ✓ | Loop construct |
| `quote` | ✓ | ✓ | And `'` syntax |
| `quasiquote` | ✓ | ✓ | `` ` ``, `,`, `,@` |
| `set!` | ✓ | ✓ | Mutation |
| `begin` | ✓ | ✓ | Sequencing |
| `and` / `or` | ✓ | ✓ | Short-circuit evaluation |
| `call/cc` | ✓ | ✗ | Not implemented |
| `delay`/`force` | ✓ | ✗ | Not implemented |
| `define-syntax` | ✓ | ✗ | Uses `defmacro` instead |
| Proper tail calls | ✓ | ✓ | Guaranteed |

### Data Types

| Type | R5RS | This Implementation |
|------|------|---------------------|
| Numbers (exact integers) | ✓ | ✓ (via double) |
| Numbers (inexact reals) | ✓ | ✓ (double precision) |
| Numbers (rationals) | ✓ | ✗ |
| Numbers (complex) | ✓ | ✗ |
| Booleans | ✓ | ✓ |
| Characters | ✓ | ✓ |
| Strings | ✓ | ✓ (immutable) |
| Symbols | ✓ | ✓ (interned) |
| Pairs/Lists | ✓ | ✓ |
| Vectors | ✓ | ✓ (R6RS) |
| Ports | ✓ | ✗ |
| Bytevectors | ✗ | ✓ (R6RS) |
| Hash tables | ✗ | ✓ (R6RS) |
| Procedures | ✓ | ✓ |

### Standard Procedures

#### Implemented (R5RS Compatible)

```scheme
; Type predicates
boolean? pair? symbol? number? string? procedure? null?

; Equality
eq? equal? =

; List operations
car cdr cons list length append reverse

; Arithmetic
+ - * / abs

; Comparison
< > <= >=

; Boolean
not

; I/O
display newline

; String operations
string-length string-append string-ref
number->string string->number
symbol->string string->symbol

; R6RS: Vectors
make-vector vector vector-length vector-ref vector-set!
vector->list list->vector vector?

; R6RS: Bytevectors
make-bytevector bytevector-length bytevector-u8-ref
bytevector-u8-set! bytevector?

; R6RS: Hash tables
make-eq-hashtable make-eqv-hashtable make-hashtable
hashtable-ref hashtable-set! hashtable-delete!
hashtable-contains? hashtable-size hashtable-keys hashtable?

; R6RS: Numeric
floor ceiling truncate round sqrt expt log sin cos tan
quotient remainder modulo integer? real? zero? positive?
negative? odd? even? min max

; R6RS: List extensions
list? list-ref list-tail memq memv member assq assv assoc

; R6RS: Characters
char? char=? char<? char->integer integer->char

; R7RS: Multiple values
values call-with-values

; R7RS: List operations
make-list list-copy list-set!

; R7RS: Vector operations
vector-copy vector-fill! vector-append

; R7RS: String operations
string-copy substring string=? string<?

; R7RS: Numeric operations
square exact inexact finite? infinite? nan? gcd lcm

; R7RS: Equivalence
boolean=? symbol=?

; R7RS: Higher-order functions
map for-each filter fold fold-right
```

#### Not Implemented

```scheme
; Continuations
call-with-current-continuation (call/cc)
dynamic-wind

; Multiple values
values call-with-values

; I/O Ports
read write
open-input-file open-output-file
close-input-port close-output-port

; Vectors
vector vector-ref vector-set! vector-length
make-vector vector->list list->vector

; Characters (partial)
char<? char>? char=? char-ci=? etc.

; Advanced numeric
quotient remainder modulo
numerator denominator
floor ceiling truncate round
sqrt expt log sin cos tan etc.
```

## Extensions Beyond R5RS

### `defmacro` (Traditional Lisp Macros)

Unlike R5RS which uses hygienic `define-syntax`, this implementation provides
traditional Lisp-style macros:

```scheme
(defmacro when (test . body)
  `(if ,test (begin ,@body)))

(defmacro unless (test . body)
  `(if (not ,test) (begin ,@body)))
```

### `mod` (Modulo Operator)

```scheme
(mod 17 5)  ; => 2
```

### `print` (Print with Newline)

```scheme
(print "hello")  ; Prints "hello\n"
```

## Semantic Differences from R5RS

### 1. Numbers

All numbers are represented as IEEE 754 double-precision floats internally.
This means:
- Integers up to 2^53 are exact
- No arbitrary precision integers
- No exact/inexact distinction

```scheme
(/ 1 3)   ; => 0.333333... (not exact rational)
```

### 2. Strings

Strings are immutable. There is no `string-set!`.

### 3. `#f` is the Only False Value

Like R5RS, only `#f` is false. Unlike some Lisps, `'()` (nil) is true:

```scheme
(if '() 'yes 'no)  ; => yes (nil is true!)
(if #f 'yes 'no)   ; => no
```

### 4. No Multiple Return Values

Functions return a single value. No `values` or `call-with-values`.

## Grammar (EBNF)

```ebnf
program     = { datum } ;
datum       = atom | list | quoted ;
atom        = SYMBOL | NUMBER | STRING | BOOLEAN | CHARACTER ;
list        = "(" { datum } ")"
            | "(" { datum } "." datum ")" ;
quoted      = "'" datum
            | "`" datum
            | "," datum
            | ",@" datum ;

SYMBOL      = symbol-char { symbol-char } ;
NUMBER      = [ "-" ] digit { digit } [ "." digit { digit } ] ;
STRING      = '"' { string-char } '"' ;
BOOLEAN     = "#t" | "#f" ;
CHARACTER   = "#\" char-name | "#\" any-char ;
```

## Memory Model

- **Garbage Collection**: Manual/simple (no generational GC)
- **Symbol Interning**: All symbols are interned (pointer equality works)
- **Closures**: Captured environments are shared (not copied)

## Comparison with Other Lisps

| Feature | This Impl | Scheme R5RS | Common Lisp | Clojure |
|---------|-----------|-------------|-------------|---------|
| Namespace | Lisp-1 | Lisp-1 | Lisp-2 | Lisp-1 |
| Nil/False | `#f` only | `#f` only | `NIL` | `nil`/`false` |
| Macros | `defmacro` | `syntax-rules` | `defmacro` | `defmacro` |
| Tail calls | Yes | Yes | No (impl-dependent) | No (JVM) |
| Continuations | No | Yes | No | No |
| Immutability | Partial | Partial | No | Default |
| Typing | Dynamic | Dynamic | Dynamic | Dynamic |

## Implementation Notes

### Interpreter

- Tree-walking evaluator
- Tail call optimization via trampoline pattern
- Lexical environments as linked structures

### Compiler (MASM Output)

- Generates x64 Windows assembly
- Uses runtime library for object allocation
- Proper tail calls compiled to jumps
- Closure conversion for nested functions

## Version History

| Version | Date | Notes |
|---------|------|-------|
| 1.0.0 | 2026-02-03 | Initial release (R5RS subset) |
| 1.1.0 | 2026-02-03 | R6RS features: vectors, bytevectors, hash tables, numeric extensions |
| 1.2.0 | 2026-02-03 | R7RS features: when/unless, case-lambda, do, multiple values, higher-order functions |

## References

1. **R5RS**: Kelsey, Clinger, Rees (eds.). "Revised⁵ Report on the Algorithmic
   Language Scheme." 1998.

2. **SICP**: Abelson, Sussman. "Structure and Interpretation of Computer
   Programs." MIT Press, 1996.

3. **Lisp in Small Pieces**: Queinnec. "Lisp in Small Pieces." Cambridge
   University Press, 2003.

## Summary

This is a **Scheme-like Lisp** implementing a practical subset of R5RS with R6RS and R7RS extensions:

- ✓ Lexical scoping
- ✓ First-class functions and closures
- ✓ Proper tail call optimization
- ✓ Traditional Lisp macros (defmacro)
- ✓ Quasiquotation
- ✓ R6RS vectors and bytevectors
- ✓ R6RS hash tables
- ✓ R6RS numeric extensions (floor, ceiling, sqrt, etc.)
- ✓ R7RS when/unless, case-lambda, do
- ✓ R7RS multiple values (values, call-with-values)
- ✓ R7RS higher-order functions (map, filter, fold)
- ✗ Continuations
- ✗ Hygienic macros
- ✗ Full numeric tower
- ✗ Library system

It's best described as: **"R5RS Scheme subset with defmacro, R6RS data structures, and R7RS control forms"**
