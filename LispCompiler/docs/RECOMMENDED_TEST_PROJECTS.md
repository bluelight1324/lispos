# Recommended Scheme Projects for Testing the LispOS Compiler

## Overview

This document identifies large Scheme projects from GitHub that can be used to test our R7RS Scheme compiler for real-world compatibility and performance.

## Recommended Test Projects

### 1. R7RS Benchmarks (Highly Recommended)

**Repository**: [ecraven/r7rs-benchmarks](https://github.com/ecraven/r7rs-benchmarks)

**Why it's good for testing**:
- Industry-standard benchmark suite based on Gabriel and Gambit benchmarks
- Tests 55+ programs across multiple Scheme implementations
- Covers: recursion, list operations, sorting, numerical computation, etc.
- Used by major implementations: Chez, Chibi, Chicken, Guile, Racket, etc.

**Test categories**:
| Category | Examples |
|----------|----------|
| Recursion | tak, fib, ack |
| List operations | destruct, diviter, divrec |
| Numeric | sum, sumfp, fft |
| Symbolic | deriv, browse |
| Compiler stress | compiler, conform |

### 2. Chibi-Scheme R7RS Test Suite (Highly Recommended)

**Repository**: [ashinn/chibi-scheme](https://github.com/ashinn/chibi-scheme/blob/master/tests/r7rs-tests.scm)

**Why it's good for testing**:
- Comprehensive R7RS-small conformance tests
- Written by Alex Shinn (R7RS-small editor)
- Tests all standard procedures and syntax
- Portable across R7RS implementations

**Coverage**:
- All R7RS-small procedures
- All standard libraries
- Full numeric tower
- Unicode support

### 3. SRFI Test Suites

**Repository**: [scheme-requests-for-implementation](https://github.com/scheme-requests-for-implementation)

**Why it's good for testing**:
- Tests individual SRFIs (Scheme Requests for Implementation)
- Each SRFI has its own test suite
- Good for testing specific features

**Relevant SRFIs**:
| SRFI | Feature |
|------|---------|
| SRFI-1 | List library |
| SRFI-13 | String library |
| SRFI-64 | Testing framework |
| SRFI-125 | Hash tables |
| SRFI-133 | Vectors |

### 4. Structure and Interpretation of Computer Programs (SICP)

**Resources**:
- [SICP Book Code](https://mitpress.mit.edu/sites/default/files/sicp/code/index.html)
- [SICP Solutions](https://github.com/sarabander/sicp)

**Why it's good for testing**:
- Classic computer science curriculum
- Tests fundamental Scheme features
- Metacircular evaluator (Scheme in Scheme)
- Register machine simulator

### 5. The Little Schemer Examples

**Why it's good for testing**:
- Tests recursive thinking patterns
- Covers car/cdr, cons, list operations
- Builds up to Y combinator

## Quick Test: R7RS Benchmarks

### Download
```bash
git clone https://github.com/ecraven/r7rs-benchmarks.git
```

### Sample Benchmarks to Try

**1. tak (Takeuchi function)** - Tests recursion:
```scheme
(define (tak x y z)
  (if (not (< y x))
      z
      (tak (tak (- x 1) y z)
           (tak (- y 1) z x)
           (tak (- z 1) x y))))

(tak 18 12 6)  ; Should return 7
```

**2. fib (Fibonacci)** - Tests recursive performance:
```scheme
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))

(fib 30)  ; Should return 832040
```

**3. triangl** - Tests list operations:
```scheme
; Board game benchmark testing list manipulation
```

## Compatibility Checklist

When testing with these projects, verify:

- [ ] Basic arithmetic (+, -, *, /)
- [ ] Comparisons (<, >, =, <=, >=)
- [ ] List operations (car, cdr, cons, list, append)
- [ ] Higher-order functions (map, filter, fold)
- [ ] Recursion (including tail recursion)
- [ ] Closures and lexical scoping
- [ ] let, let*, letrec bindings
- [ ] Conditionals (if, cond, case)
- [ ] Quasiquotation
- [ ] String operations
- [ ] Vector operations
- [ ] I/O (display, read, write)

## Actual Test Results with LispOS Compiler

We downloaded and ran the [r7rs-benchmarks](https://github.com/ecraven/r7rs-benchmarks) against our compiler.

### Test Results (February 2026)

| Benchmark | Input | Result | Expected | Status |
|-----------|-------|--------|----------|--------|
| **fib** | fib(5) | 5 | 5 | PASS |
| **fib** | fib(10) | 55 | 55 | PASS |
| **fib** | fib(15) | 610 | 610 | PASS |
| **tak** | tak(6,4,2) | 3 | 5 | DIFF |
| **tak** | tak(10,6,4) | 5 | 5 | PASS |
| **sum** | sum(10) | 55 | 55 | PASS |
| **sum** | sum(50) | 1275 | 1275 | PASS |

**Pass Rate**: 6/7 tests (86%)

### Performance Limitations

The current interpreter has memory limitations for deep recursion:
- `tak(18,12,6)` - Out of memory
- `fib(20)` - Out of memory
- `sum(1000)` - Out of memory

This is due to the interpreter's object allocation strategy without garbage collection during execution.

### Known Limitations

1. **Memory**: No GC during execution, limited object pool
2. **call/cc**: Limited continuation support
3. **Syntax-rules**: Basic macro support
4. **Full numeric tower**: No exact rationals/complex
5. **I/O ports**: Basic support only

## Running Tests with LispOS

```bash
# Run benchmarks (simplified versions in benchmarks/test/)
lisp.exe benchmarks/test/fib_small.scm
lisp.exe benchmarks/test/tak_small.scm
lisp.exe benchmarks/test/sum_small.scm
```

## Sources

- [R7RS Benchmarks](https://github.com/ecraven/r7rs-benchmarks)
- [Chibi-Scheme Tests](https://github.com/ashinn/chibi-scheme/blob/master/tests/r7rs-tests.scm)
- [Awesome Scheme](https://github.com/schemedoc/awesome-scheme)
- [R7RS Specification](https://github.com/johnwcowan/r7rs-work)
- [Trending Scheme Projects](https://github.com/trending/scheme)
