# Debugger Implementation Documentation

This document describes the implementation of the debugger for the Scheme interpreter and SchemeEdit IDE.

## Overview

The debugger consists of three main components:

1. **Core Debugger** (LispCompiler) - Handles breakpoints, stepping, and state inspection
2. **Debug Protocol** - JSON-based communication between IDE and interpreter
3. **IDE Integration** (SchemeEdit) - Visual debugging interface

## Phase 1: Core Debugger (LispCompiler)

### Files Modified/Created

| File | Purpose |
|------|---------|
| [debug.h](../src/debug.h) | Debug state structures and API declarations |
| [debug.c](../src/debug.c) | Debug implementation |
| [eval.c](../src/eval.c) | Added debug hooks |
| [env.c](../src/env.c) | Added `env_get_all_bindings()` |
| [main.c](../src/main.c) | Added `--debug` flags |

### Debug State Structure

```c
typedef struct DebugState {
    DebugMode mode;           // NONE, STEP_INTO, STEP_OVER, STEP_OUT, CONTINUE, PAUSED
    int step_depth;           // For step-over/step-out tracking
    int is_running;

    // Breakpoints
    Breakpoint breakpoints[MAX_BREAKPOINTS];
    int num_breakpoints;

    // Call stack
    CallFrame call_stack[MAX_CALL_STACK];
    int stack_depth;

    // Current state
    LispObject *current_expr;
    Environment *current_env;
    SourceLocation current_location;
} DebugState;
```

### Key Functions

| Function | Description |
|----------|-------------|
| `debug_init()` | Initialize debugger |
| `debug_enable()` / `debug_disable()` | Enable/disable debugging |
| `debug_add_breakpoint()` | Add breakpoint at file:line |
| `debug_check_break()` | Check if should stop (called by eval) |
| `debug_push_frame()` / `debug_pop_frame()` | Manage call stack |
| `debug_repl()` | Interactive debug command loop |

### Eval Integration

Debug hooks are added to `eval()`:

```c
LispObject *eval(LispObject *expr, Environment *env) {
    // Debug hook: check if we should break
    if (debug_is_enabled()) {
        debug_check_break(expr, env);
    }
    // ... rest of evaluation
}
```

Call stack tracking in `apply()`:

```c
if (is_lambda(func)) {
    if (debug_is_enabled()) {
        debug_push_frame(func_name, args, call_env, NULL);
    }
    LispObject *result = eval_sequence(func->lambda.body, call_env);
    if (debug_is_enabled()) {
        debug_pop_frame();
    }
    return result;
}
```

## Phase 2: Debug Protocol

### Command-Line Usage

```bash
# Run with text-based debugger
lisp.exe --debug program.scm

# Run with JSON protocol (for IDE)
lisp.exe --debug-json program.scm
```

### Text-Based Commands

| Command | Description |
|---------|-------------|
| `run`, `r` | Continue execution |
| `step`, `s` | Step into |
| `next`, `n` | Step over |
| `finish`, `f` | Step out |
| `break <line>` | Set breakpoint |
| `delete <id>` | Delete breakpoint |
| `backtrace`, `bt` | Show call stack |
| `print <expr>` | Evaluate expression |
| `locals` | Show local variables |
| `help` | Show commands |
| `quit` | Exit debugger |

### JSON Protocol

Request format:
```json
{
    "type": "request",
    "command": "setBreakpoint",
    "arguments": {
        "file": "program.scm",
        "line": 10
    }
}
```

Event format (when stopped):
```json
{
    "type": "event",
    "event": "stopped",
    "body": {
        "reason": "breakpoint",
        "file": "program.scm",
        "line": 10
    }
}
```

## Phase 3: IDE Integration (SchemeEdit)

### Files Modified/Created

| File | Purpose |
|------|---------|
| [debugcontroller.h](../../SchemeEdit/src/debugcontroller.h) | Debug controller class |
| [debugcontroller.cpp](../../SchemeEdit/src/debugcontroller.cpp) | Controller implementation |
| [editor.h](../../SchemeEdit/src/editor.h) | Added breakpoint support |
| [editor.cpp](../../SchemeEdit/src/editor.cpp) | Breakpoint markers and debug line |
| [mainwindow.h](../../SchemeEdit/src/mainwindow.h) | Added debug UI components |
| [mainwindow.cpp](../../SchemeEdit/src/mainwindow.cpp) | Debug actions and panels |

### DebugController Class

The `DebugController` class manages communication with the debugger:

```cpp
class DebugController : public QObject {
    Q_OBJECT
public:
    void startDebugging(const QString &file, const QString &compilerPath);
    void stopDebugging();
    void continueExecution();
    void stepInto();
    void stepOver();
    void stepOut();
    void setBreakpoint(const QString &file, int line);

signals:
    void paused(const QString &file, int line, const QString &reason);
    void stackTraceReceived(const QVector<StackFrame> &frames);
    void variablesReceived(const QVector<Variable> &variables);
};
```

### Editor Breakpoint Support

The editor now supports:

- **Breakpoint markers** - Red circles in the gutter
- **Debug line indicator** - Yellow arrow showing current execution
- **Click to toggle** - Click in gutter to add/remove breakpoints

```cpp
void Editor::toggleBreakpoint(int line);
bool Editor::hasBreakpoint(int line) const;
void Editor::setDebugLine(int line);
void Editor::clearDebugLine();
```

### Debug UI Components

| Component | Description |
|-----------|-------------|
| Debug Menu | Start/Stop, Step controls, Toggle Breakpoint |
| Debug Toolbar | Quick access buttons for debug actions |
| Call Stack Panel | Shows current call stack |
| Variables Panel | Shows local variables |

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| F9 | Start Debugging / Toggle Breakpoint |
| Shift+F5 | Stop Debugging |
| F5 | Continue |
| F10 | Step Over |
| F11 | Step Into |
| Shift+F11 | Step Out |

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    SchemeEdit IDE                        │
├───────────────────┬─────────────────┬───────────────────┤
│      Editor       │   Call Stack    │    Variables      │
│  (breakpoints)    │     Panel       │      Panel        │
├───────────────────┴─────────────────┴───────────────────┤
│                  DebugController                         │
│            (QProcess communication)                      │
├─────────────────────────────────────────────────────────┤
│                  JSON Protocol                           │
├─────────────────────────────────────────────────────────┤
│               lisp.exe --debug-json                      │
├─────────────────────────────────────────────────────────┤
│                    Debug Module                          │
│  ┌──────────────┐  ┌────────────┐  ┌─────────────────┐ │
│  │  Breakpoint  │  │   Call     │  │    Variable     │ │
│  │   Manager    │  │   Stack    │  │   Inspector     │ │
│  └──────────────┘  └────────────┘  └─────────────────┘ │
├─────────────────────────────────────────────────────────┤
│                   Evaluator (eval.c)                     │
│                 (with debug hooks)                       │
└─────────────────────────────────────────────────────────┘
```

## Building

### LispCompiler

```bash
cd LispCompiler
cmake -B build -S .
cmake --build build --config Release
```

### SchemeEdit

```bash
cd SchemeEdit
cmake -B build -S .
cmake --build build --config Release
```

## Usage Example

1. Open a Scheme file in SchemeEdit
2. Click in the gutter to set breakpoints
3. Press F9 or Debug > Start Debugging
4. When execution stops:
   - View call stack in the Call Stack panel
   - Inspect variables in the Variables panel
   - Use F10/F11 to step through code
   - Press F5 to continue to next breakpoint
5. Press Shift+F5 to stop debugging

## Limitations

- Source location tracking is approximate (line-level)
- Conditional breakpoints require full implementation
- Watch expressions not yet implemented
- No data breakpoints (break on value change)

## Future Enhancements

- Full source location tracking with column information
- Watch expressions panel
- Conditional breakpoints
- Exception breakpoints
- Data breakpoints
- Remote debugging support
