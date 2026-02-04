# Lisp Compiler/Interpreter

A complete Lisp/Scheme implementation with both interpretation and compilation to
x64 MASM assembly.

## Features

- **Interactive REPL** - Read-Eval-Print Loop for interactive development
- **File Execution** - Run Lisp/Scheme source files
- **MASM Compilation** - Compile to x64 Windows assembly (MASM syntax)
- **Lexical Scoping** - Proper closure support with lexical environments
- **Tail Call Optimization** - Efficient recursive functions
- **Standard Library** - Core Lisp primitives and functions

## Building

### Prerequisites

- CMake 3.16 or later
- C compiler (MSVC, GCC, or Clang)
- Optional: LALRGen (for parser generation)

### Build Steps

```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Visual Studio

```batch
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Then open the generated `.sln` file.

## Usage

### Interactive REPL

```
> lisp
Lisp REPL v1.0.0
Type ',help' for help, ',quit' to exit.

> (+ 1 2 3)
6
> (define (square x) (* x x))
square
> (square 5)
25
> ,quit
Goodbye!
```

### Execute a File

```
> lisp test/factorial.scm
5! = 120
10! = 3628800
12! (tail-recursive) = 479001600
```

### Compile to MASM

```
> lisp -c test/factorial.scm
Compiling test/factorial.scm -> test/factorial.asm
Compilation successful.

To assemble and link (Windows):
  ml64 /c factorial.asm
  link /subsystem:console /entry:main factorial.obj lisp_rt.lib
```

## Language Reference

### Special Forms

| Form | Syntax | Description |
|------|--------|-------------|
| `quote` | `(quote x)` or `'x` | Return x unevaluated |
| `if` | `(if test then else)` | Conditional expression |
| `define` | `(define var val)` | Define a variable |
| `define` | `(define (f x) body)` | Define a function |
| `set!` | `(set! var val)` | Mutate a variable |
| `lambda` | `(lambda (x) body)` | Create a closure |
| `begin` | `(begin e1 e2 ...)` | Sequence expressions |
| `let` | `(let ((v e) ...) body)` | Local bindings |
| `let*` | `(let* ((v e) ...) body)` | Sequential bindings |
| `letrec` | `(letrec ((v e) ...) body)` | Recursive bindings |
| `cond` | `(cond (test e) ...)` | Multi-way conditional |
| `and` | `(and e1 e2 ...)` | Short-circuit and |
| `or` | `(or e1 e2 ...)` | Short-circuit or |
| `defmacro` | `(defmacro name (args) body)` | Define a macro |
| `quasiquote` | `` `x `` | Quasiquotation |

### Built-in Functions

#### List Operations
- `(car pair)` - First element
- `(cdr pair)` - Rest of list
- `(cons a b)` - Create pair
- `(list a b ...)` - Create list
- `(length lst)` - List length
- `(append l1 l2 ...)` - Concatenate lists
- `(reverse lst)` - Reverse list

#### Type Predicates
- `(null? x)` - Is nil?
- `(pair? x)` - Is pair?
- `(number? x)` - Is number?
- `(symbol? x)` - Is symbol?
- `(string? x)` - Is string?
- `(procedure? x)` - Is function?
- `(boolean? x)` - Is boolean?

#### Arithmetic
- `(+ a b ...)` - Addition
- `(- a b ...)` - Subtraction
- `(* a b ...)` - Multiplication
- `(/ a b)` - Division
- `(mod a b)` - Modulo
- `(abs n)` - Absolute value

#### Comparison
- `(= a b)` - Numeric equality
- `(< a b)` - Less than
- `(> a b)` - Greater than
- `(<= a b)` - Less or equal
- `(>= a b)` - Greater or equal
- `(eq? a b)` - Pointer equality
- `(equal? a b)` - Structural equality

#### Boolean
- `(not x)` - Logical negation

#### I/O
- `(display x)` - Print without newline
- `(newline)` - Print newline
- `(print x)` - Print with newline

#### String Operations
- `(string-length s)` - String length
- `(string-append s1 s2 ...)` - Concatenate strings
- `(string-ref s i)` - Character at index
- `(number->string n)` - Number to string
- `(string->number s)` - String to number
- `(symbol->string s)` - Symbol to string
- `(string->symbol s)` - String to symbol

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Source Code (.scm)                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│  Lexer (lexer.c)                                            │
│  - Tokenizes source into symbols, numbers, strings, etc.    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│  Parser (parser.c)                                          │
│  - Builds S-expression tree (AST)                           │
│  - Handles quote syntax sugar                               │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌───────────────────────────┐   ┌───────────────────────────┐
│  Interpreter (eval.c)     │   │  Compiler (codegen.c)     │
│  - Tree-walking eval      │   │  - MASM x64 generation    │
│  - Environment/closures   │   │  - Tail call optimization │
│  - Primitives (prim.c)    │   │  - Runtime library calls  │
└───────────────────────────┘   └───────────────────────────┘
              │                               │
              ▼                               ▼
┌───────────────────────────┐   ┌───────────────────────────┐
│       Result Value        │   │    Assembly (.asm)        │
└───────────────────────────┘   └───────────────────────────┘
```

## File Structure

```
LispCompiler/
├── src/
│   ├── main.c          # Driver (REPL, file execution, compilation)
│   ├── lisp.h/c        # Object representation
│   ├── lexer.h/c       # Tokenizer
│   ├── parser.h/c      # Recursive descent parser
│   ├── env.h/c         # Environments and scoping
│   ├── eval.h/c        # Interpreter
│   ├── primitives.h/c  # Built-in functions
│   ├── codegen.h/c     # MASM code generator
│   └── lisp_grammar.y  # LALRGen grammar (optional)
├── test/
│   ├── hello.scm       # Hello World
│   ├── factorial.scm   # Factorial tests
│   └── list_ops.scm    # List operation tests
├── docs/
│   └── README.md       # This file
└── CMakeLists.txt      # Build configuration
```

## Examples

### Fibonacci

```scheme
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1))
         (fib (- n 2)))))

(display (fib 10))  ; 55
```

### Higher-Order Functions

```scheme
(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst))
            (map f (cdr lst)))))

(map (lambda (x) (* x x)) '(1 2 3 4 5))
; => (1 4 9 16 25)
```

### Closures

```scheme
(define (make-counter)
  (let ((count 0))
    (lambda ()
      (set! count (+ count 1))
      count)))

(define c1 (make-counter))
(c1)  ; 1
(c1)  ; 2
(c1)  ; 3
```

## License

This project is part of the lispos project and follows its licensing terms.
