# SchemeEdit User Guide

## Overview

SchemeEdit is a full-featured development environment for Scheme programming, built with Qt and integrated with the LispOS R7RS Scheme compiler.

## Getting Started

### Launching SchemeEdit

Run `SchemeEdit.exe` from the `build/bin` directory.

### Quick Start

1. Go to **Examples** menu and select any example to load it
2. Press **F5** to run the code
3. View results in the Output pane at the bottom

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **F5** | Run entire file |
| **Ctrl+Enter** | Run selected text or current line |
| **Ctrl+B** | Compile to MASM x64 assembly |
| **Ctrl+Shift+B** | Compile and run |
| **Ctrl+L** | Clear output pane |
| **Ctrl+N** | New file |
| **Ctrl+O** | Open file |
| **Ctrl+S** | Save file |
| **Ctrl+Shift+S** | Save as |
| **Ctrl+Z** | Undo |
| **Ctrl+Y** | Redo |
| **Ctrl+X** | Cut |
| **Ctrl+C** | Copy |
| **Ctrl+V** | Paste |
| **Ctrl+A** | Select all |
| **F1** | Quick Start Guide |

## Features

### Syntax Highlighting

SchemeEdit provides syntax highlighting for:
- Keywords (define, lambda, if, cond, etc.)
- Strings
- Numbers
- Comments
- Parentheses

### Running Code

**Run Entire File (F5)**
- Runs all code in the editor
- Results appear in the Output pane

**Run Selection (Ctrl+Enter)**
- Runs only the selected text
- If nothing is selected, runs the current line
- Useful for testing individual expressions

### Compiling

**Compile to Assembly (Ctrl+B)**
- Compiles the Scheme file to MASM x64 assembly
- Output file has `.asm` extension
- Shows compilation status in Output pane

**Compile and Run (Ctrl+Shift+B)**
- Currently runs in interpreted mode
- Future: Will compile, assemble, link, and run

### Output Pane

The Output pane shows:
- Execution results
- Error messages
- Compilation status
- Timestamps for each operation

Status indicators:
- ✓ Success (green)
- ✗ Error (red)
- ▶ Running (blue)

## Examples Menu

The Examples menu contains 8 tutorial programs:

### 1. Hello World
Basic output using `display` and `newline`.

### 2. Variables
Defining and using variables with `define`.

### 3. Functions
Creating reusable functions.

### 4. Conditionals
Using `if`, `cond`, `and`, `or`.

### 5. Recursion
Self-referencing functions (factorial, fibonacci).

### 6. Lists
Working with Scheme's fundamental data structure.

### 7. Higher-Order Functions
Functions as values (map, filter, reduce).

### 8. Complete Program
Prime number generator using multiple techniques.

## Scheme Quick Reference

### Basic Syntax

```scheme
; This is a comment

; Numbers
42
3.14
-5

; Strings
"Hello, World!"

; Booleans
#t  ; true
#f  ; false

; Lists
'(1 2 3)
(list 1 2 3)
```

### Defining Variables

```scheme
(define x 42)
(define pi 3.14159)
(define greeting "Hello")
```

### Defining Functions

```scheme
; Simple function
(define (square x)
  (* x x))

; Multiple arguments
(define (add a b)
  (+ a b))

; With conditionals
(define (abs x)
  (if (< x 0)
      (- 0 x)
      x))
```

### Conditionals

```scheme
; if-then-else
(if (> x 0)
    "positive"
    "not positive")

; Multiple conditions
(cond ((> x 0) "positive")
      ((< x 0) "negative")
      (else "zero"))

; Boolean logic
(and (> x 0) (< x 10))
(or (= x 0) (= x 1))
```

### Lists

```scheme
; Create list
'(1 2 3 4 5)

; First element
(car '(1 2 3))  ; => 1

; Rest of list
(cdr '(1 2 3))  ; => (2 3)

; Add to front
(cons 0 '(1 2 3))  ; => (0 1 2 3)

; Check empty
(null? '())  ; => #t
```

### Higher-Order Functions

```scheme
; Anonymous function
(lambda (x) (* x x))

; Apply to list
(map (lambda (x) (* x 2)) '(1 2 3))
; => (2 4 6)

; Filter list
(filter (lambda (x) (> x 2)) '(1 2 3 4 5))
; => (3 4 5)
```

## Troubleshooting

### "Interpreter not found"

Ensure the LispOS compiler is built:
```
cd LispCompiler
cmake -B build -S .
cmake --build build --config Release
```

### Code doesn't run

- Check for matching parentheses
- Ensure the file is saved before compiling
- Look for error messages in the Output pane

### Compilation errors

- Check syntax errors in your code
- Verify the output path is writable
- Check that all required functions are defined

## File Locations

- **Examples**: `SchemeEdit/examples/*.scm`
- **Editor**: `SchemeEdit/build/bin/SchemeEdit.exe`
- **Compiler**: `LispCompiler/build/Release/lisp.exe`
