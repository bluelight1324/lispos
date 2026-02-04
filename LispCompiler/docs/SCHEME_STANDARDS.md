# Scheme Standards After R5RS

## Overview

The Scheme programming language has evolved through several standardization efforts.
This document covers the standards that came after R5RS (1998).

## Timeline of Scheme Standards

```
1975    Original Scheme (Sussman & Steele)
1978    Revised Report (R1RS)
1985    R2RS
1986    R3RS
1991    R4RS, IEEE Standard
1998    R5RS ← Current implementation baseline
2007    R6RS (controversial, larger language)
2013    R7RS-small (return to minimalism)
ongoing R7RS-large (modular extensions)
```

## R6RS (2007)

### Overview

R6RS was ratified in August 2007 and represented a significant departure from the
minimalist philosophy of R5RS.

**Official Document**: [R6RS Standard](https://standards.scheme.org/official/r6rs.pdf)

### Key Changes from R5RS

| Feature | R5RS | R6RS |
|---------|------|------|
| Size | ~50 pages | ~180 pages |
| Library system | None | Full module system |
| Unicode | None | Full Unicode support |
| Exceptions | None | Condition system |
| Records | None | Record types |
| Hash tables | None | Built-in |
| Bytevectors | None | Built-in |
| Exact/Inexact | Partial | Strict requirements |

### New Features in R6RS

1. **Library System**
```scheme
(library (my-library)
  (export my-function)
  (import (rnrs))
  (define (my-function x) (* x x)))
```

2. **Unicode Strings**
```scheme
(string-length "日本語")  ; => 3
```

3. **Condition System (Exceptions)**
```scheme
(guard (ex
        [(error? ex) (display "Error!")])
  (raise (make-error)))
```

4. **Record Types**
```scheme
(define-record-type point
  (fields x y))
(make-point 3 4)
```

5. **Hash Tables**
```scheme
(define ht (make-eq-hashtable))
(hashtable-set! ht 'key 'value)
```

### Controversy

R6RS was controversial because:
- **Size**: Much larger than R5RS
- **Complexity**: Abandoned minimalist philosophy
- **Compatibility**: Breaking changes from R5RS
- **Adoption**: Many implementations didn't fully adopt it

> "R6RS abandoned the simplicity of R5RS and shipped a more full-featured language."

## R7RS (2013)

### Overview

Due to the controversy around R6RS, the Scheme Steering Committee decided in 2009
to split the standard into two parts:

1. **R7RS-small**: Minimal, R5RS-compatible core
2. **R7RS-large**: Practical features for mainstream development

**Official Document**: [R7RS Standard](https://standards.scheme.org/official/r7rs.pdf)

### R7RS-small (Ratified 2013)

R7RS-small is designed to be:
- Compatible with R5RS
- Suitable for education and embedded systems
- Minimal but practical

#### Changes from R5RS

| Feature | R5RS | R7RS-small |
|---------|------|------------|
| Libraries | None | `define-library` |
| Parameters | None | `parameterize` |
| Exceptions | None | `guard`, `raise` |
| Records | None | `define-record-type` |
| Bytevectors | None | Basic support |
| `include` | None | Supported |
| `cond-expand` | None | Feature detection |

#### New Syntax in R7RS-small

1. **Library Definition**
```scheme
(define-library (example lib)
  (export square)
  (import (scheme base))
  (begin
    (define (square x) (* x x))))
```

2. **Exception Handling**
```scheme
(guard (condition
        [(error-object? condition)
         (display "Error!")])
  (error "Something went wrong"))
```

3. **Parameters (Dynamic Variables)**
```scheme
(define current-output (make-parameter (current-output-port)))
(parameterize ((current-output some-port))
  (display "Hello"))
```

4. **Feature Detection**
```scheme
(cond-expand
  [(library (srfi 1)) (import (srfi 1))]
  [else (include "my-srfi-1.scm")])
```

5. **Record Types**
```scheme
(define-record-type <point>
  (make-point x y)
  point?
  (x point-x)
  (y point-y))
```

### R7RS-large (Ongoing)

R7RS-large is being developed incrementally through **SRFIs** (Scheme Requests
for Implementation). It aims to provide a practical standard library.

**Status**: Ongoing development via [SRFI process](https://srfi.schemers.org/)

#### Planned/Completed Dockets

| Docket | Name | Status | Contents |
|--------|------|--------|----------|
| Red | Data Structures | Complete | Lists, vectors, strings, hash tables |
| Tangerine | Numerics | Complete | Numeric operations, random numbers |
| Orange | I/O | In Progress | Ports, file system |
| Yellow | Syntax | In Progress | Pattern matching, macros |
| Green | Misc | In Progress | Various utilities |
| Blue | Portable | Planned | OS interface |
| Indigo | Advanced | Planned | Concurrency, FFI |

## Comparison Table

| Feature | R5RS | R6RS | R7RS-small | R7RS-large |
|---------|------|------|------------|------------|
| Year | 1998 | 2007 | 2013 | Ongoing |
| Philosophy | Minimal | Full-featured | Minimal | Practical |
| Spec size | ~50 pp | ~180 pp | ~80 pp | Modular |
| Libraries | No | Yes | Yes | Yes |
| Unicode | No | Yes | Yes | Yes |
| Exceptions | No | Yes | Yes | Yes |
| Records | No | Yes | Yes | Yes |
| Hash tables | No | Yes | SRFI | Yes |
| Continuations | Yes | Yes | Yes | Yes |
| Hygienic macros | Yes | Yes | Yes | Yes |
| Tail calls | Yes | Yes | Yes | Yes |

## Current Implementation Landscape

### Major Implementations and Standards

| Implementation | R5RS | R6RS | R7RS |
|----------------|------|------|------|
| **Chez Scheme** | ✓ | ✓ | Partial |
| **Racket** | ✓ | ✓ | ✗ (own language) |
| **Guile** | ✓ | Partial | Partial |
| **Chicken** | ✓ | ✗ | ✓ |
| **Chibi-Scheme** | ✓ | ✗ | ✓ (reference) |
| **Gambit** | ✓ | Partial | Partial |
| **MIT Scheme** | ✓ | ✗ | ✗ |
| **Kawa** | ✓ | ✓ | Partial |
| **Larceny** | ✓ | ✓ | ✓ |

## Recommendations

### For New Projects

1. **Education/Learning**: R5RS or R7RS-small
2. **Production (portable)**: R7RS-small + selected SRFIs
3. **Production (full-featured)**: R6RS or implementation-specific

### For This Implementation

Our implementation is based on **R5RS** with extensions. To upgrade:

| To Support | Effort | Priority |
|------------|--------|----------|
| R7RS-small libraries | Medium | High |
| R7RS-small exceptions | Low | High |
| R7RS-small records | Medium | Medium |
| R6RS full | High | Low |
| R7RS-large | Very High | Future |

## Summary

| Standard | Description | Recommendation |
|----------|-------------|----------------|
| **R5RS** | Classic minimal Scheme | Good baseline |
| **R6RS** | Full-featured but controversial | Production use |
| **R7RS-small** | Modern minimal Scheme | **Recommended** |
| **R7RS-large** | Practical extensions | Watch/adopt incrementally |

**Current Best Choice**: R7RS-small provides a good balance between R5RS simplicity
and modern features. It's the recommended target for new Scheme implementations.

## References

1. [Scheme Standards Official Site](https://standards.scheme.org/)
2. [R7RS Specification (PDF)](https://standards.scheme.org/official/r7rs.pdf)
3. [R6RS Specification (PDF)](https://standards.scheme.org/official/r6rs.pdf)
4. [Scheme (Wikipedia)](https://en.wikipedia.org/wiki/Scheme_(programming_language))
5. [R6RS vs R7RS Discussion](https://elmord.org/blog/?entry=20171001-r6rs-r7rs)
6. [SRFI Process](https://srfi.schemers.org/)
