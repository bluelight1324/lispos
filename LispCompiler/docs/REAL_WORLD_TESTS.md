# Real World Tests Documentation

## Overview

This document records comprehensive real-world testing of the LispOS Scheme compiler/interpreter, including factorial computation and various algorithmic tests.

## Test Environment

- **Platform**: Windows 10/11 x64
- **Compiler**: MSVC (Visual Studio 2022 Build Tools)
- **Build Type**: Release
- **Date**: 2026-02-04

---

## Test 1: Factorial Computation

### Test File: `factorial_test.scm`

```scheme
(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))
```

### Results

| Input | Result | Verified |
|-------|--------|----------|
| factorial(5) | 120 | ✅ |
| factorial(10) | 3,628,800 | ✅ |
| factorial(12) | 479,001,600 | ✅ |
| factorial(20) | 2,432,902,008,176,640,000 | ✅ |
| factorial(50) | 3.04141e+64 | ✅ (floating point) |
| factorial(100) | 9.33262e+157 | ✅ (floating point) |

### Notes
- Numbers up to factorial(20) fit in 64-bit integers
- Larger factorials use double-precision floating point
- factorial(100) ≈ 9.332621544×10^157 (correct scientific notation)

---

## Test 2: List Processing

### Operations Tested
- **map**: Apply function to each element
- **filter**: Select elements matching predicate
- **reduce**: Fold list into single value

### Results

```
Original list: (1 2 3 4 5 6 7 8 9 10)
Squared:       (1 4 9 16 25 36 49 64 81 100) ✅
Even numbers:  (2 4 6 8 10) ✅
Sum:           55 ✅
Product:       3628800 (= 10!) ✅
```

---

## Test 3: Higher-Order Functions

### Operations Tested
- **compose**: Function composition (f ∘ g)
- **curry**: Partial application

### Results

```
add-one-then-double(5) = 12 ✅  (double(5+1) = 12)
curried add-10(7) = 17 ✅
```

---

## Test 4: Prime Numbers (Sieve)

### Algorithm
Simple trial division primality test

### Results

```
Primes up to 50: (2 3 5 7 11 13 17 19 23 29 31 37 41 43 47) ✅
```

15 primes found - all correct.

---

## Test 5: Recursion Patterns

### Ackermann Function
A highly recursive function that grows extremely fast.

| Input | Result | Verified |
|-------|--------|----------|
| Ackermann(2,3) | 9 | ✅ |
| Ackermann(3,3) | 61 | ✅ |

### GCD (Euclid's Algorithm)

| Input | Result | Verified |
|-------|--------|----------|
| GCD(48, 18) | 6 | ✅ |
| GCD(1071, 462) | 21 | ✅ |

---

## Test 6: Fibonacci (Iterative)

Using tail-recursive named `let` for efficiency.

| Input | Result | Verified |
|-------|--------|----------|
| fib(30) | 832,040 | ✅ |
| fib(40) | 102,334,155 | ✅ |

---

## Test 7: Data Structures

### Points as Pairs

```scheme
(define (make-point x y) (cons x y))
(define (distance p1 p2) ...)
```

### Results

```
Distance from origin to (3,4) = 5 ✅  (3-4-5 triangle)
Distance from (3,4) to (6,8) = 5 ✅   (same 3-4-5 pattern)
```

---

## Test 8: Large List Operations

### Tail-Recursive Sum

| Operation | Result | Formula Verification |
|-----------|--------|---------------------|
| Sum 1..100 | 5,050 | n(n+1)/2 = 100×101/2 = 5,050 ✅ |
| Sum 1..1000 | 500,500 | n(n+1)/2 = 1000×1001/2 = 500,500 ✅ |

---

## Summary

| Test Category | Status |
|--------------|--------|
| Factorial (including 100!) | ✅ PASSED |
| List Processing (map/filter/reduce) | ✅ PASSED |
| Higher-Order Functions | ✅ PASSED |
| Prime Number Generation | ✅ PASSED |
| Recursive Algorithms (Ackermann, GCD) | ✅ PASSED |
| Fibonacci (iterative) | ✅ PASSED |
| Data Structures | ✅ PASSED |
| Large List Operations | ✅ PASSED |

**All 8 test categories passed successfully.**

---

## Performance Notes

1. **Large Number Support**: Uses IEEE 754 double-precision for numbers exceeding 64-bit integer range
2. **Recursion**: Both deep recursion (Ackermann) and tail recursion work correctly
3. **GC Integration**: All tests run with garbage collection enabled, no memory errors
4. **List Processing**: Efficient handling of lists up to 1000+ elements

---

## Test Files

All test files located in `LispCompiler/tests/`:
- `factorial_test.scm` - Factorial computation tests
- `real_world_tests.scm` - Comprehensive algorithmic tests

---

## Conclusion

The LispOS Scheme compiler/interpreter successfully handles real-world computational tasks including:
- Mathematical computations with large numbers
- Functional programming patterns
- Complex recursive algorithms
- Data structure manipulation
- Large dataset processing

The implementation is suitable for general-purpose Scheme programming.
