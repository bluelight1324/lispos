# Scheme Debugger Design Document

This document describes how to implement a debugger for the LispCompiler interpreter and integrate it with SchemeEdit.

## Overview

The debugger will operate at two levels:
1. **Interpreter Level** - Core debugging functionality in the evaluator
2. **IDE Level** - User interface in SchemeEdit

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    SchemeEdit IDE                        │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │   Editor    │  │  Call Stack  │  │   Variables   │  │
│  │  (with BP)  │  │    Panel     │  │    Watch      │  │
│  └─────────────┘  └──────────────┘  └───────────────┘  │
├─────────────────────────────────────────────────────────┤
│              Debug Protocol (JSON/Pipe)                  │
├─────────────────────────────────────────────────────────┤
│                  LispCompiler (lisp.exe)                │
│  ┌──────────────────────────────────────────────────┐  │
│  │                Debug Controller                   │  │
│  │  - Breakpoint Manager                            │  │
│  │  - Execution Controller                          │  │
│  │  - State Inspector                               │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │              Evaluator (eval.c)                   │  │
│  │  - Debug hooks at eval() entry/exit              │  │
│  │  - Source location tracking                      │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Part 1: Interpreter-Level Debugging

### 1.1 Source Location Tracking

Add source location information to parsed expressions:

```c
/* In lisp.h */
typedef struct SourceLocation {
    const char *filename;
    int line;
    int column;
} SourceLocation;

/* Extend LispObject */
typedef struct LispObject {
    LispType type;
    SourceLocation *source_loc;  /* NEW: source location */
    /* ... rest of object */
} LispObject;
```

### 1.2 Debug State Structure

```c
/* In debug.h */
typedef enum {
    DEBUG_MODE_NONE,        /* Normal execution */
    DEBUG_MODE_STEP_INTO,   /* Stop at next expression */
    DEBUG_MODE_STEP_OVER,   /* Stop at next expression same level */
    DEBUG_MODE_STEP_OUT,    /* Stop when returning from current function */
    DEBUG_MODE_CONTINUE     /* Run until breakpoint */
} DebugMode;

typedef struct Breakpoint {
    const char *filename;
    int line;
    int id;
    int enabled;
    char *condition;        /* Optional: conditional breakpoint */
} Breakpoint;

typedef struct CallFrame {
    const char *function_name;
    LispObject *arguments;
    Environment *env;
    SourceLocation *location;
    struct CallFrame *parent;
} CallFrame;

typedef struct DebugState {
    DebugMode mode;
    int step_depth;         /* For step-over/step-out */

    /* Breakpoints */
    Breakpoint *breakpoints;
    int num_breakpoints;
    int max_breakpoints;

    /* Call stack */
    CallFrame *current_frame;
    int stack_depth;

    /* Callbacks */
    void (*on_break)(DebugState *state, LispObject *expr);
    void (*on_step)(DebugState *state, LispObject *expr);
} DebugState;

/* Global debug state */
extern DebugState *g_debug_state;
```

### 1.3 Debug Hooks in Evaluator

Modify `eval()` to call debug hooks:

```c
/* In eval.c */
LispObject *eval(LispObject *expr, Environment *env) {
    /* Debug hook: before evaluation */
    if (g_debug_state && g_debug_state->mode != DEBUG_MODE_NONE) {
        debug_check_break(g_debug_state, expr, env);
    }

    /* ... existing eval code ... */

    LispObject *result = /* evaluation result */;

    /* Debug hook: after evaluation (for step-out) */
    if (g_debug_state && g_debug_state->mode == DEBUG_MODE_STEP_OUT) {
        debug_check_return(g_debug_state, expr, result);
    }

    return result;
}
```

### 1.4 Breakpoint Checking

```c
/* In debug.c */
void debug_check_break(DebugState *state, LispObject *expr, Environment *env) {
    int should_break = 0;

    /* Check mode */
    switch (state->mode) {
        case DEBUG_MODE_STEP_INTO:
            should_break = 1;
            break;

        case DEBUG_MODE_STEP_OVER:
            should_break = (state->stack_depth <= state->step_depth);
            break;

        case DEBUG_MODE_STEP_OUT:
            should_break = (state->stack_depth < state->step_depth);
            break;

        case DEBUG_MODE_CONTINUE:
            /* Check breakpoints */
            if (expr->source_loc) {
                should_break = debug_has_breakpoint(state,
                    expr->source_loc->filename,
                    expr->source_loc->line);
            }
            break;
    }

    if (should_break && state->on_break) {
        state->on_break(state, expr);
    }
}

int debug_has_breakpoint(DebugState *state, const char *file, int line) {
    for (int i = 0; i < state->num_breakpoints; i++) {
        Breakpoint *bp = &state->breakpoints[i];
        if (bp->enabled && bp->line == line) {
            if (strcmp(bp->filename, file) == 0) {
                /* Check condition if present */
                if (bp->condition) {
                    /* Evaluate condition in current env */
                    return debug_eval_condition(bp->condition);
                }
                return 1;
            }
        }
    }
    return 0;
}
```

### 1.5 Call Stack Management

```c
/* In debug.c */
void debug_push_frame(DebugState *state, const char *func_name,
                      LispObject *args, Environment *env,
                      SourceLocation *loc) {
    CallFrame *frame = malloc(sizeof(CallFrame));
    frame->function_name = func_name;
    frame->arguments = args;
    frame->env = env;
    frame->location = loc;
    frame->parent = state->current_frame;

    state->current_frame = frame;
    state->stack_depth++;
}

void debug_pop_frame(DebugState *state) {
    if (state->current_frame) {
        CallFrame *old = state->current_frame;
        state->current_frame = old->parent;
        state->stack_depth--;
        free(old);
    }
}

/* Get call stack as list for display */
LispObject *debug_get_call_stack(DebugState *state) {
    LispObject *stack = make_nil();
    CallFrame *frame = state->current_frame;

    while (frame) {
        LispObject *frame_info = make_cons(
            make_string(frame->function_name),
            make_cons(make_number(frame->location->line),
                     frame->arguments));
        stack = make_cons(frame_info, stack);
        frame = frame->parent;
    }

    return stack;
}
```

### 1.6 Variable Inspection

```c
/* In debug.c */
LispObject *debug_get_locals(DebugState *state) {
    if (!state->current_frame) return make_nil();

    Environment *env = state->current_frame->env;
    return env_get_all_bindings(env);  /* Returns alist of (name . value) */
}

LispObject *debug_eval_expression(DebugState *state, const char *expr_str) {
    if (!state->current_frame) return make_nil();

    /* Parse and evaluate in current frame's environment */
    LispObject *expr = parse_string(expr_str);
    return eval(expr, state->current_frame->env);
}
```

## Part 2: Debug Protocol

### 2.1 Command-Line Debug Mode

Add `--debug` flag to lisp.exe:

```c
/* In main.c */
int main(int argc, char *argv[]) {
    int debug_mode = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
        }
    }

    if (debug_mode) {
        init_debugger();
        run_debug_repl();
    } else {
        /* Normal execution */
    }
}
```

### 2.2 Debug Commands (Text Protocol)

```
Command             Description
---------------------------------------------------------
break <file> <line> Set breakpoint
delete <id>         Delete breakpoint
enable <id>         Enable breakpoint
disable <id>        Disable breakpoint
list                List all breakpoints

run                 Start/continue execution
step                Step into next expression
next                Step over (stay at same level)
finish              Step out (return from function)
stop                Pause execution

backtrace / bt      Show call stack
frame <n>           Select stack frame
up / down           Move up/down stack

print <expr>        Evaluate expression
locals              Show local variables
info                Show current location

quit                Exit debugger
```

### 2.3 JSON Protocol (for IDE Integration)

```json
// Request: Set breakpoint
{
    "type": "request",
    "command": "setBreakpoint",
    "arguments": {
        "file": "example.scm",
        "line": 10,
        "condition": "(> x 5)"
    }
}

// Response
{
    "type": "response",
    "command": "setBreakpoint",
    "success": true,
    "body": {
        "breakpointId": 1
    }
}

// Event: Stopped at breakpoint
{
    "type": "event",
    "event": "stopped",
    "body": {
        "reason": "breakpoint",
        "file": "example.scm",
        "line": 10,
        "breakpointId": 1
    }
}

// Request: Get call stack
{
    "type": "request",
    "command": "stackTrace"
}

// Response
{
    "type": "response",
    "command": "stackTrace",
    "body": {
        "stackFrames": [
            {
                "id": 0,
                "name": "factorial",
                "file": "example.scm",
                "line": 10
            },
            {
                "id": 1,
                "name": "main",
                "file": "example.scm",
                "line": 25
            }
        ]
    }
}
```

## Part 3: SchemeEdit IDE Integration

### 3.1 Debug Toolbar

```cpp
// In mainwindow.h
class MainWindow : public QMainWindow {
private:
    // Debug actions
    QAction *m_debugStartAction;
    QAction *m_debugStopAction;
    QAction *m_debugStepIntoAction;
    QAction *m_debugStepOverAction;
    QAction *m_debugStepOutAction;
    QAction *m_debugContinueAction;

    // Debug state
    QProcess *m_debugProcess;
    bool m_isDebugging;

    // Debug panels
    QDockWidget *m_callStackDock;
    QDockWidget *m_variablesDock;
    QListWidget *m_callStackList;
    QTreeWidget *m_variablesTree;
};
```

### 3.2 Breakpoint Markers in Editor

```cpp
// In editor.h
class Editor : public QPlainTextEdit {
private:
    QSet<int> m_breakpoints;  // Line numbers with breakpoints

public:
    void toggleBreakpoint(int line);
    bool hasBreakpoint(int line) const;
    QList<int> getBreakpoints() const;

protected:
    void lineNumberAreaMouseEvent(QMouseEvent *event);
};

// In editor.cpp
void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    // ... existing code ...

    // Draw breakpoint markers
    while (block.isValid() && top <= event->rect().bottom()) {
        if (m_breakpoints.contains(blockNumber)) {
            // Draw red circle for breakpoint
            int centerY = top + fontMetrics().height() / 2;
            painter.setBrush(QColor(255, 0, 0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPoint(8, centerY), 5, 5);
        }

        // Draw yellow arrow for current execution line
        if (blockNumber == m_currentDebugLine) {
            painter.setBrush(QColor(255, 255, 0));
            // Draw arrow shape
            QPolygon arrow;
            arrow << QPoint(4, top + 4)
                  << QPoint(12, top + fontMetrics().height() / 2)
                  << QPoint(4, top + fontMetrics().height() - 4);
            painter.drawPolygon(arrow);
        }

        // ... rest of line number drawing ...
    }
}

void Editor::toggleBreakpoint(int line) {
    if (m_breakpoints.contains(line)) {
        m_breakpoints.remove(line);
    } else {
        m_breakpoints.insert(line);
    }
    m_lineNumberArea->update();
    emit breakpointsChanged();
}
```

### 3.3 Debug Process Communication

```cpp
// In debugcontroller.h
class DebugController : public QObject {
    Q_OBJECT

public:
    explicit DebugController(QObject *parent = nullptr);

    void startDebugging(const QString &file);
    void stopDebugging();

    void stepInto();
    void stepOver();
    void stepOut();
    void continueExecution();

    void setBreakpoint(const QString &file, int line);
    void removeBreakpoint(int id);

    void evaluateExpression(const QString &expr);

signals:
    void stopped(const QString &file, int line, const QString &reason);
    void callStackUpdated(const QList<StackFrame> &frames);
    void variablesUpdated(const QList<Variable> &variables);
    void outputReceived(const QString &output);
    void debuggingFinished();

private slots:
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode);

private:
    QProcess *m_process;
    void sendCommand(const QJsonObject &cmd);
    void handleResponse(const QJsonObject &response);
    void handleEvent(const QJsonObject &event);
};
```

### 3.4 Debug Panel Widgets

```cpp
// Call Stack Panel
class CallStackWidget : public QListWidget {
public:
    void updateStack(const QList<StackFrame> &frames) {
        clear();
        for (const StackFrame &frame : frames) {
            QString text = QString("%1 at %2:%3")
                .arg(frame.name)
                .arg(frame.file)
                .arg(frame.line);
            addItem(text);
        }
    }
};

// Variables Panel
class VariablesWidget : public QTreeWidget {
public:
    void updateVariables(const QList<Variable> &vars) {
        clear();
        for (const Variable &var : vars) {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, var.name);
            item->setText(1, var.value);
            item->setText(2, var.type);
            addTopLevelItem(item);
        }
    }
};
```

## Part 4: Implementation Plan

### Phase 1: Core Debugger (Interpreter)
1. Add source location tracking to parser
2. Implement DebugState and breakpoint management
3. Add debug hooks to eval()
4. Implement call stack tracking
5. Create text-based debug REPL

**Estimated complexity**: Medium-High

### Phase 2: Debug Protocol
1. Define JSON protocol specification
2. Implement JSON command handler
3. Add `--debug` and `--debug-json` flags
4. Test with manual JSON commands

**Estimated complexity**: Medium

### Phase 3: IDE Integration
1. Add breakpoint markers to editor
2. Create debug toolbar and actions
3. Implement DebugController class
4. Add call stack panel
5. Add variables panel
6. Connect all components

**Estimated complexity**: High

### Phase 4: Advanced Features
1. Conditional breakpoints
2. Watch expressions
3. Expression evaluation in context
4. Data breakpoints (break on value change)
5. Exception/error breakpoints

**Estimated complexity**: Medium

## File Structure

```
LispCompiler/
├── src/
│   ├── debug.h          # Debug state and API
│   ├── debug.c          # Debug implementation
│   ├── debug_protocol.h # JSON protocol definitions
│   ├── debug_protocol.c # Protocol handling
│   └── eval.c           # Modified with debug hooks
│
SchemeEdit/
├── src/
│   ├── debugcontroller.h
│   ├── debugcontroller.cpp
│   ├── callstackwidget.h
│   ├── callstackwidget.cpp
│   ├── variableswidget.h
│   ├── variableswidget.cpp
│   └── editor.cpp        # Modified for breakpoints
```

## Example Debug Session

```
$ lisp.exe --debug factorial.scm

Scheme Debugger v1.0
Type 'help' for commands.

(debug) break factorial.scm 5
Breakpoint 1 set at factorial.scm:5

(debug) run
Starting program...
Hit breakpoint 1 at factorial.scm:5

   3:   (define (factorial n)
   4:     (if (<= n 1)
=> 5:         1
   6:         (* n (factorial (- n 1)))))

(debug) print n
5

(debug) backtrace
#0  factorial (n=5) at factorial.scm:5
#1  <toplevel> at factorial.scm:10

(debug) step
Hit breakpoint at factorial.scm:6

   4:     (if (<= n 1)
   5:         1
=> 6:         (* n (factorial (- n 1)))))

(debug) continue
Program output: 120
Program finished.

(debug) quit
```

## Summary

This debugger design provides:

1. **Full source-level debugging** with breakpoints, stepping, and variable inspection
2. **Flexible protocol** supporting both text and JSON modes
3. **IDE integration** with visual breakpoints and debug panels
4. **Incremental implementation** through phased approach

The design follows patterns from established debuggers (GDB, LLDB) and debug protocols (DAP - Debug Adapter Protocol) while being tailored to the Scheme interpreter's architecture.
