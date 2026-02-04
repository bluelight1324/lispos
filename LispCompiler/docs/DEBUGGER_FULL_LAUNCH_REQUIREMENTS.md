# Debugger Full Launch Requirements

This document describes what additional features are needed to complete a full-featured debugger for production use.

## Current Implementation Status

### What's Implemented (Phases 1-3)

| Feature | Status | Notes |
|---------|--------|-------|
| Breakpoints (line-based) | ✅ Done | Works at expression level |
| Step Into | ✅ Done | `step` command |
| Step Over | ✅ Done | `next` command |
| Step Out | ✅ Done | `finish` command |
| Continue | ✅ Done | `run`/`continue` commands |
| Call Stack Display | ✅ Done | `backtrace` command |
| Local Variables | ✅ Done | `locals` command |
| Expression Evaluation | ✅ Done | `print` command |
| JSON Protocol | ✅ Done | Basic IDE communication |
| Text-based REPL | ✅ Done | Interactive debugger |
| IDE Integration | ✅ Done | SchemeEdit + VS Code configs |

### What's Missing (Phase 4 + Beyond)

## 1. Conditional Breakpoints (Priority: HIGH)

**Current State**: Condition storage exists but evaluation not implemented.

**What's Needed**:
```c
// In debug.c
int debug_eval_condition(const char *condition, Environment *env) {
    // Parse condition string
    LispObject *cond_expr = parse_string(condition);
    if (!cond_expr) return 1; // Break on parse error

    // Evaluate in current environment
    LispObject *result = eval(cond_expr, env);

    // Return true if result is not #f
    return !is_false(result);
}
```

**User Impact**: Can't set breakpoints like "break when x > 100" - must break every time and manually check.

## 2. Watch Expressions (Priority: HIGH)

**Current State**: Not implemented.

**What's Needed**:
```c
typedef struct WatchExpression {
    int id;
    char *expression;
    LispObject *last_value;
    int break_on_change;  // Data breakpoint mode
} WatchExpression;

// API
int debug_add_watch(const char *expr);
void debug_remove_watch(int id);
void debug_update_watches(void);  // Called after each step
LispObject *debug_get_watch_value(int id);
```

**Commands**:
```
watch <expr>        Add watch expression
unwatch <id>        Remove watch
info watch          List all watches
```

**User Impact**: Must manually `print` variables repeatedly; can't track values over time.

## 3. Data Breakpoints (Priority: MEDIUM)

**Current State**: Not implemented.

**What's Needed**:
- Track memory locations or variable bindings
- Break when value changes

```c
typedef struct DataBreakpoint {
    int id;
    const char *variable_name;
    LispObject *last_value;
    int enabled;
} DataBreakpoint;

int debug_add_data_breakpoint(const char *var_name);
void debug_check_data_breakpoints(Environment *env);  // Called after mutations
```

**Commands**:
```
watch <var> break   Break when variable changes
```

**User Impact**: Can't automatically stop when a variable changes value.

## 4. Exception Breakpoints (Priority: HIGH)

**Current State**: Not implemented.

**What's Needed**:
```c
typedef enum {
    EXCEPTION_BREAK_NONE,
    EXCEPTION_BREAK_UNCAUGHT,
    EXCEPTION_BREAK_ALL
} ExceptionBreakMode;

void debug_set_exception_break(ExceptionBreakMode mode);
void debug_on_exception(const char *message, LispObject *expr);  // Called from error handler
```

**Commands**:
```
catch all           Break on all exceptions
catch uncaught      Break only on uncaught exceptions
catch none          Don't break on exceptions
```

**User Impact**: Errors just print a message; can't examine state when error occurs.

## 5. Line-Level Source Tracking (Priority: HIGH)

**Current State**: Expressions tracked, not individual lines.

**What's Needed**:
- Parser must record line/column for every token
- Each node in AST gets source location
- Multi-line expressions track start and end

```c
// In parser
typedef struct Token {
    TokenType type;
    const char *text;
    int line;      // Line number
    int column;    // Column number
} Token;

// In lisp.h
typedef struct LispObject {
    LispType type;
    SourceLocation loc_start;  // Where expression starts
    SourceLocation loc_end;    // Where expression ends
    // ... rest of object
} LispObject;
```

**User Impact**: Breakpoints at line 5 might not hit if expression starts at line 4.

## 6. Full DAP Compliance (Priority: MEDIUM)

**Current State**: Basic JSON protocol implemented.

**What's Needed** for [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/):

| DAP Feature | Current | Needed |
|-------------|---------|--------|
| Initialize | ❌ | Capability negotiation |
| Launch/Attach | Partial | Full configuration |
| SetBreakpoints | ✅ | Verified/unverified status |
| SetFunctionBreakpoints | ❌ | Break on function entry |
| SetExceptionBreakpoints | ❌ | Exception handling |
| ConfigurationDone | ❌ | Startup sequence |
| Continue | ✅ | - |
| Next (Step Over) | ✅ | - |
| StepIn | ✅ | - |
| StepOut | ✅ | - |
| Pause | Partial | Async pause |
| StackTrace | ✅ | - |
| Scopes | Partial | Local/Global/Closure scopes |
| Variables | ✅ | Structured/lazy loading |
| Source | ❌ | Retrieve source code |
| Threads | ❌ | Single-threaded OK |
| Evaluate | ✅ | Watch/Hover/REPL contexts |
| Completions | ❌ | Auto-complete in debug console |
| Modules | ❌ | Library/module info |
| LoadedSources | ❌ | List loaded files |
| ReadMemory | ❌ | Raw memory inspection |
| Disassemble | ❌ | Show generated assembly |

**User Impact**: VS Code integration works but lacks polish; some debug features disabled.

## 7. Source Mapping (Priority: MEDIUM)

**Current State**: Not implemented.

**What's Needed**:
- Map compiled code back to source positions
- Support for macros (where did this code come from?)
- Multi-file support with include tracking

```c
typedef struct SourceMap {
    const char *original_file;
    int original_line;
    const char *expanded_file;  // After macro expansion
    int expanded_line;
} SourceMap;
```

**User Impact**: After macro expansion, stepping shows expanded code, not original source.

## 8. Hot Reload / Edit and Continue (Priority: LOW)

**Current State**: Not implemented.

**What's Needed**:
- Re-parse changed definitions without restarting
- Update function bindings in running environment
- Handle changed variable values

```c
int debug_reload_file(const char *filename);
int debug_redefine(const char *definition);
```

**Commands**:
```
reload              Reload current file
reload <file>       Reload specific file
eval (define ...)   Redefine on the fly
```

**User Impact**: Must restart debug session after any code change.

## 9. Memory/Object Inspection (Priority: LOW)

**Current State**: Can print values but not examine structure.

**What's Needed**:
```c
void debug_inspect_object(LispObject *obj);  // Show internal structure
void debug_memory_stats(void);                // GC stats, allocations
void debug_find_references(LispObject *obj);  // Who points to this?
```

**Commands**:
```
inspect <expr>      Show object internals
memory              Show memory statistics
refs <expr>         Find references to object
```

**User Impact**: Can't understand why objects aren't being garbage collected.

## 10. REPL Integration (Priority: MEDIUM)

**Current State**: Debug REPL and regular REPL are separate.

**What's Needed**:
- Interactive REPL that can be paused and inspected
- Seamless switch between running and debugging
- History and command completion

**User Impact**: Must choose between REPL mode and debug mode upfront.

## Implementation Priority

### Critical for Production (Must Have)

1. **Conditional Breakpoints** - Essential for debugging loops
2. **Exception Breakpoints** - Essential for error debugging
3. **Watch Expressions** - Essential for tracking state
4. **Line-Level Source Tracking** - Essential for accurate breakpoints

### Important (Should Have)

5. **Full DAP Compliance** - Required for first-class VS Code support
6. **Data Breakpoints** - Very useful for state debugging
7. **Source Mapping** - Important for macro-heavy code

### Nice to Have (Could Have)

8. **REPL Integration** - Convenience feature
9. **Hot Reload** - Development convenience
10. **Memory Inspection** - Advanced debugging

## Estimated Implementation Effort

| Feature | Complexity | Files Changed | Est. Lines |
|---------|------------|---------------|------------|
| Conditional Breakpoints | Low | debug.c | ~50 |
| Watch Expressions | Medium | debug.c, debug.h | ~150 |
| Exception Breakpoints | Medium | debug.c, error.c | ~100 |
| Data Breakpoints | Medium | debug.c, env.c | ~200 |
| Line-Level Tracking | High | parser.c, lisp.h | ~300 |
| Full DAP | High | debug.c (new: dap.c) | ~800 |
| Source Mapping | Medium | parser.c, debug.c | ~200 |
| Hot Reload | High | eval.c, env.c | ~400 |
| Memory Inspection | Low | debug.c, memory.c | ~100 |
| REPL Integration | Medium | main.c, debug.c | ~150 |

## Recommended Implementation Order

### Phase 4A: Essential Features
1. Conditional breakpoints
2. Exception breakpoints
3. Watch expressions

### Phase 4B: Accuracy
4. Line-level source tracking
5. Source mapping

### Phase 4C: IDE Polish
6. Full DAP compliance
7. Data breakpoints

### Phase 4D: Advanced
8. REPL integration
9. Hot reload
10. Memory inspection

## Testing Requirements

Each feature needs:
1. Unit tests for core functionality
2. Integration tests with debugger
3. VS Code extension testing
4. SchemeEdit IDE testing
5. Documentation updates

## Conclusion

The current debugger provides **functional debugging** suitable for:
- Learning and small projects
- Basic troubleshooting
- Step-through debugging

For **production-grade debugging**, the following are essential:
- Conditional breakpoints
- Exception breakpoints
- Watch expressions
- Accurate line-level tracking

Full implementation of all features would bring the debugger to parity with professional tools like:
- GDB (GNU Debugger)
- LLDB
- Visual Studio Debugger
- IntelliJ IDEA Debugger

The phased approach allows incremental improvement while maintaining a working debugger throughout development.
