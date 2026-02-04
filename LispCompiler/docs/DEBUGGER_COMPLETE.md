# Debugger Complete Implementation

This document describes the complete debugger implementation for LispCompiler, including all features from Task 22.1.

## Overview

The Scheme debugger now provides full-featured debugging capabilities:

- **Conditional Breakpoints** - Break only when a condition is true
- **Watch Expressions** - Monitor variable values
- **Data Breakpoints** - Break when a value changes
- **Exception Breakpoints** - Break on errors/exceptions
- **Full VS Code Integration** - DAP-compliant debug adapter

## New Features

### 1. Conditional Breakpoints

Set breakpoints that only trigger when a condition evaluates to true.

#### Usage

```
(debug) break 10
Breakpoint 1 set at example.scm:10

(debug) cond 1 (> x 5)
Breakpoint 1 condition set: (> x 5)
```

Now the breakpoint at line 10 will only trigger when `x > 5`.

#### Commands

| Command | Description |
|---------|-------------|
| `cond <id> <expr>` | Set condition on breakpoint |
| `cond <id>` | Clear condition (break always) |

### 2. Watch Expressions

Monitor the value of expressions as you step through code.

#### Usage

```
(debug) watch x
Watch 1 added: x

(debug) watch (+ x y)
Watch 2 added: (+ x y)

(debug) info watch
Watch expressions:
  1: x [enabled] = 10
  2: (+ x y) [enabled] = 30
```

#### Commands

| Command | Description |
|---------|-------------|
| `watch <expr>` | Add watch expression |
| `watch <expr> break` | Add data breakpoint |
| `unwatch <id>` | Remove watch |
| `info watch` | List all watches |

### 3. Data Breakpoints

Break when a watched value changes.

#### Usage

```
(debug) watch counter break
Data breakpoint 1 set: break when 'counter' changes

... execution continues ...

Data breakpoint hit: 'counter' changed
  Old value: 5
  New value: 6
```

#### How It Works

1. Add a watch with `break` suffix
2. Debugger monitors the value after each expression
3. When value changes, execution pauses
4. Both old and new values are displayed

### 4. Exception Breakpoints

Break when errors occur during execution.

#### Usage

```
(debug) catch all
Breaking on all exceptions.

... execution continues until an error ...

Exception breakpoint hit!
Error: Division by zero
At: (/ 10 0)
```

#### Commands

| Command | Description |
|---------|-------------|
| `catch all` | Break on all exceptions |
| `catch uncaught` | Break on uncaught exceptions only |
| `catch none` | Disable exception breakpoints |

### 5. VS Code Integration

The VS Code debug adapter supports all new features:

#### Conditional Breakpoints in VS Code

1. Right-click on a breakpoint in the gutter
2. Select "Edit Breakpoint..."
3. Enter condition: `(> x 5)`
4. Breakpoint now has a condition icon

#### Exception Breakpoints in VS Code

1. Open Debug view (Ctrl+Shift+D)
2. In "Breakpoints" section, check:
   - "All Exceptions" - break on every error
   - "Uncaught Exceptions" - break on unhandled errors

#### Watch Panel

1. Open Debug view
2. In "Watch" section, click "+"
3. Enter expression: `x` or `(+ a b)`
4. Value updates as you step

## Implementation Details

### Files Modified

| File | Changes |
|------|---------|
| `src/debug.h` | Added WatchExpression, ExceptionBreakMode types |
| `src/debug.c` | Implemented all new features |
| `vscode-scheme/src/debugAdapter.ts` | Added DAP support for new features |

### New API Functions

```c
/* Conditional breakpoint evaluation */
static int debug_eval_condition(const char *condition, Environment *env);

/* Watch expressions */
int debug_add_watch(const char *expression);
int debug_add_data_breakpoint(const char *expression);
int debug_remove_watch(int id);
void debug_list_watches(void);
void debug_update_watches(void);
void debug_clear_all_watches(void);

/* Exception breakpoints */
void debug_set_exception_break(ExceptionBreakMode mode);
ExceptionBreakMode debug_get_exception_break(void);
void debug_on_exception(const char *message, LispObject *expr);
```

### Data Structures

```c
/* Watch expression */
typedef struct WatchExpression {
    int id;
    char expression[MAX_WATCH_NAME_LENGTH];
    LispObject *last_value;
    int break_on_change;  /* 1 = data breakpoint */
    int enabled;
} WatchExpression;

/* Exception break mode */
typedef enum {
    EXCEPTION_BREAK_NONE,
    EXCEPTION_BREAK_UNCAUGHT,
    EXCEPTION_BREAK_ALL
} ExceptionBreakMode;
```

## Complete Command Reference

### Execution Control

| Command | Shortcut | Description |
|---------|----------|-------------|
| `run` | `r` | Continue execution |
| `step` | `s` | Step into |
| `next` | `n` | Step over |
| `finish` | `f` | Step out |
| `stop` | - | Stop debugging |

### Breakpoints

| Command | Shortcut | Description |
|---------|----------|-------------|
| `break <line>` | `b` | Set breakpoint |
| `break <file>:<line>` | - | Set at file:line |
| `cond <id> <expr>` | - | Set condition |
| `delete <id>` | `d` | Remove breakpoint |
| `enable <id>` | - | Enable breakpoint |
| `disable <id>` | - | Disable breakpoint |
| `info break` | - | List breakpoints |
| `clear` | - | Remove all |

### Watch Expressions

| Command | Description |
|---------|-------------|
| `watch <expr>` | Add watch |
| `watch <expr> break` | Add data breakpoint |
| `unwatch <id>` | Remove watch |
| `info watch` | List watches |

### Exception Handling

| Command | Description |
|---------|-------------|
| `catch all` | Break on all errors |
| `catch uncaught` | Break on uncaught |
| `catch none` | Disable |

### Stack & Variables

| Command | Shortcut | Description |
|---------|----------|-------------|
| `backtrace` | `bt` | Show call stack |
| `frame <n>` | - | Select frame |
| `up` | - | Go up one frame |
| `down` | - | Go down one frame |
| `print <expr>` | `p` | Evaluate expression |
| `locals` | - | Show local variables |
| `where` | - | Show current location |

### Other

| Command | Shortcut | Description |
|---------|----------|-------------|
| `help` | `h` | Show help |
| `quit` | `q` | Exit debugger |

## Example Debug Session

```
$ lisp.exe --debug factorial.scm

Scheme Debugger v1.1.0
Debugging: factorial.scm
Type 'help' for debugger commands.

Program stopped (entry)

Stopped at factorial.scm:1
=> (define (factorial n) ...)
(debug) break 3
Breakpoint 1 set at factorial.scm:3

(debug) cond 1 (= n 3)
Breakpoint 1 condition set: (= n 3)

(debug) watch n
Watch 1 added: n

(debug) catch all
Breaking on all exceptions.

(debug) run
... execution continues ...

Hit breakpoint 1 at factorial.scm:3 (condition: (= n 3))
=> (* n (factorial (- n 1)))

(debug) info watch
Watch expressions:
  1: n [enabled] = 3

(debug) print (* n (factorial (- n 1)))
6

(debug) continue
Result: 120
Program finished.
```

## Testing

### Test Conditional Breakpoints

```scheme
; test_conditional.scm
(define (loop n)
  (if (> n 0)
      (begin
        (display n)
        (newline)
        (loop (- n 1)))))

(loop 10)
```

```
(debug) break 4
(debug) cond 1 (= n 5)
(debug) run
; Stops only when n = 5
```

### Test Data Breakpoints

```scheme
; test_data_bp.scm
(define counter 0)
(define (increment)
  (set! counter (+ counter 1)))

(increment)
(increment)
(increment)
```

```
(debug) watch counter break
(debug) run
; Stops each time counter changes
```

### Test Exception Breakpoints

```scheme
; test_exception.scm
(define (divide a b)
  (/ a b))

(divide 10 0)  ; Error!
```

```
(debug) catch all
(debug) run
; Stops on division by zero
```

## Known Limitations

1. **Condition Evaluation**: Conditions are evaluated in the current environment, which may not have all expected bindings
2. **Data Breakpoints**: Only simple value types are compared; complex structures use pointer equality
3. **Exception Breakpoints**: Currently treats all exceptions as catchable (uncaught mode works same as all)

## Future Enhancements

1. Hit count conditions (`break 10 if hit > 5`)
2. Log points (print without breaking)
3. Improved data breakpoint comparison for lists/vectors
4. Full uncaught exception tracking with try/catch

## Conclusion

The debugger now provides professional-grade debugging capabilities:

- **Conditional Breakpoints** - Debug specific scenarios
- **Watch Expressions** - Track values over time
- **Data Breakpoints** - Catch unexpected mutations
- **Exception Breakpoints** - Debug error conditions
- **VS Code Integration** - Full IDE support

These features bring the Scheme debugger to parity with debuggers for mainstream languages.
