/*
 * debug.c - Scheme Debugger Implementation
 *
 * Implements the debugging functionality including breakpoints,
 * stepping, call stack tracking, and variable inspection.
 */

#include "debug.h"
#include "parser.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global debug state */
DebugState *g_debug_state = NULL;

/* Private state for debug REPL */
static int debug_repl_running = 0;

/*
 * Initialize the debugger
 */
void debug_init(void) {
    if (g_debug_state) {
        debug_shutdown();
    }

    g_debug_state = (DebugState *)malloc(sizeof(DebugState));
    if (!g_debug_state) {
        fprintf(stderr, "Failed to allocate debug state\n");
        return;
    }

    memset(g_debug_state, 0, sizeof(DebugState));
    g_debug_state->mode = DEBUG_MODE_NONE;
    g_debug_state->next_breakpoint_id = 1;
    g_debug_state->next_watch_id = 1;
    g_debug_state->is_running = 0;
    g_debug_state->json_mode = 0;
    g_debug_state->exception_break_mode = EXCEPTION_BREAK_NONE;
}

/*
 * Shutdown the debugger
 */
void debug_shutdown(void) {
    if (g_debug_state) {
        free(g_debug_state);
        g_debug_state = NULL;
    }
}

/*
 * Enable debugging mode
 */
void debug_enable(void) {
    if (!g_debug_state) {
        debug_init();
    }
    g_debug_state->mode = DEBUG_MODE_PAUSED;
}

/*
 * Disable debugging mode
 */
void debug_disable(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_NONE;
    }
}

/*
 * Check if debugging is enabled
 */
int debug_is_enabled(void) {
    return g_debug_state && g_debug_state->mode != DEBUG_MODE_NONE;
}

/*
 * Set JSON mode for IDE integration
 */
void debug_set_json_mode(int enabled) {
    if (g_debug_state) {
        g_debug_state->json_mode = enabled;
    }
}

/* ============================================
 * Breakpoint Management
 * ============================================ */

/*
 * Add a breakpoint at the specified location
 */
int debug_add_breakpoint(const char *filename, int line) {
    return debug_add_conditional_breakpoint(filename, line, NULL);
}

/*
 * Add a conditional breakpoint
 */
int debug_add_conditional_breakpoint(const char *filename, int line, const char *condition) {
    if (!g_debug_state) return -1;
    if (g_debug_state->num_breakpoints >= MAX_BREAKPOINTS) {
        fprintf(stderr, "Maximum breakpoints reached\n");
        return -1;
    }

    Breakpoint *bp = &g_debug_state->breakpoints[g_debug_state->num_breakpoints];
    bp->id = g_debug_state->next_breakpoint_id++;
    bp->filename = strdup(filename);
    bp->line = line;
    bp->enabled = 1;
    bp->hit_count = 0;

    if (condition) {
        strncpy(bp->condition, condition, MAX_CONDITION_LENGTH - 1);
        bp->condition[MAX_CONDITION_LENGTH - 1] = '\0';
    } else {
        bp->condition[0] = '\0';
    }

    g_debug_state->num_breakpoints++;

    if (!g_debug_state->json_mode) {
        printf("Breakpoint %d set at %s:%d\n", bp->id, filename, line);
    }

    return bp->id;
}

/*
 * Remove a breakpoint by ID
 */
int debug_remove_breakpoint(int id) {
    if (!g_debug_state) return 0;

    for (int i = 0; i < g_debug_state->num_breakpoints; i++) {
        if (g_debug_state->breakpoints[i].id == id) {
            /* Free filename string */
            free((void *)g_debug_state->breakpoints[i].filename);

            /* Shift remaining breakpoints */
            for (int j = i; j < g_debug_state->num_breakpoints - 1; j++) {
                g_debug_state->breakpoints[j] = g_debug_state->breakpoints[j + 1];
            }
            g_debug_state->num_breakpoints--;

            if (!g_debug_state->json_mode) {
                printf("Breakpoint %d deleted\n", id);
            }
            return 1;
        }
    }
    return 0;
}

/*
 * Enable a breakpoint
 */
int debug_enable_breakpoint(int id) {
    Breakpoint *bp = debug_get_breakpoint(id);
    if (bp) {
        bp->enabled = 1;
        return 1;
    }
    return 0;
}

/*
 * Disable a breakpoint
 */
int debug_disable_breakpoint(int id) {
    Breakpoint *bp = debug_get_breakpoint(id);
    if (bp) {
        bp->enabled = 0;
        return 1;
    }
    return 0;
}

/*
 * Get a breakpoint by ID
 */
Breakpoint *debug_get_breakpoint(int id) {
    if (!g_debug_state) return NULL;

    for (int i = 0; i < g_debug_state->num_breakpoints; i++) {
        if (g_debug_state->breakpoints[i].id == id) {
            return &g_debug_state->breakpoints[i];
        }
    }
    return NULL;
}

/*
 * List all breakpoints
 */
void debug_list_breakpoints(void) {
    if (!g_debug_state || g_debug_state->num_breakpoints == 0) {
        printf("No breakpoints set.\n");
        return;
    }

    printf("Breakpoints:\n");
    for (int i = 0; i < g_debug_state->num_breakpoints; i++) {
        Breakpoint *bp = &g_debug_state->breakpoints[i];
        printf("  %d: %s:%d %s (hit %d times)",
               bp->id, bp->filename, bp->line,
               bp->enabled ? "[enabled]" : "[disabled]",
               bp->hit_count);
        if (bp->condition[0]) {
            printf(" if %s", bp->condition);
        }
        printf("\n");
    }
}

/*
 * Clear all breakpoints
 */
void debug_clear_all_breakpoints(void) {
    if (!g_debug_state) return;

    for (int i = 0; i < g_debug_state->num_breakpoints; i++) {
        free((void *)g_debug_state->breakpoints[i].filename);
    }
    g_debug_state->num_breakpoints = 0;
    printf("All breakpoints cleared.\n");
}

/* ============================================
 * Watch Expression Management
 * ============================================ */

/*
 * Add a watch expression
 */
int debug_add_watch(const char *expression) {
    if (!g_debug_state) return -1;
    if (g_debug_state->num_watches >= MAX_WATCH_EXPRESSIONS) {
        fprintf(stderr, "Maximum watch expressions reached\n");
        return -1;
    }

    WatchExpression *watch = &g_debug_state->watches[g_debug_state->num_watches];
    watch->id = g_debug_state->next_watch_id++;
    strncpy(watch->expression, expression, MAX_WATCH_NAME_LENGTH - 1);
    watch->expression[MAX_WATCH_NAME_LENGTH - 1] = '\0';
    watch->last_value = NULL;
    watch->break_on_change = 0;
    watch->enabled = 1;

    g_debug_state->num_watches++;

    if (!g_debug_state->json_mode) {
        printf("Watch %d added: %s\n", watch->id, expression);
    }

    return watch->id;
}

/*
 * Add a data breakpoint (watch with break on change)
 */
int debug_add_data_breakpoint(const char *expression) {
    int id = debug_add_watch(expression);
    if (id > 0) {
        WatchExpression *watch = debug_get_watch(id);
        if (watch) {
            watch->break_on_change = 1;
            if (!g_debug_state->json_mode) {
                printf("Data breakpoint %d set: break when '%s' changes\n", id, expression);
            }
        }
    }
    return id;
}

/*
 * Remove a watch expression
 */
int debug_remove_watch(int id) {
    if (!g_debug_state) return 0;

    for (int i = 0; i < g_debug_state->num_watches; i++) {
        if (g_debug_state->watches[i].id == id) {
            /* Shift remaining watches */
            for (int j = i; j < g_debug_state->num_watches - 1; j++) {
                g_debug_state->watches[j] = g_debug_state->watches[j + 1];
            }
            g_debug_state->num_watches--;

            if (!g_debug_state->json_mode) {
                printf("Watch %d removed\n", id);
            }
            return 1;
        }
    }
    return 0;
}

/*
 * Enable a watch expression
 */
int debug_enable_watch(int id) {
    WatchExpression *watch = debug_get_watch(id);
    if (watch) {
        watch->enabled = 1;
        return 1;
    }
    return 0;
}

/*
 * Disable a watch expression
 */
int debug_disable_watch(int id) {
    WatchExpression *watch = debug_get_watch(id);
    if (watch) {
        watch->enabled = 0;
        return 1;
    }
    return 0;
}

/*
 * Get a watch expression by ID
 */
WatchExpression *debug_get_watch(int id) {
    if (!g_debug_state) return NULL;

    for (int i = 0; i < g_debug_state->num_watches; i++) {
        if (g_debug_state->watches[i].id == id) {
            return &g_debug_state->watches[i];
        }
    }
    return NULL;
}

/*
 * List all watch expressions
 */
void debug_list_watches(void) {
    if (!g_debug_state || g_debug_state->num_watches == 0) {
        printf("No watch expressions.\n");
        return;
    }

    printf("Watch expressions:\n");
    for (int i = 0; i < g_debug_state->num_watches; i++) {
        WatchExpression *watch = &g_debug_state->watches[i];
        printf("  %d: %s %s%s",
               watch->id, watch->expression,
               watch->enabled ? "[enabled]" : "[disabled]",
               watch->break_on_change ? " [data bp]" : "");

        /* Evaluate and show current value */
        if (watch->enabled && g_debug_state->current_env) {
            LispObject *value = debug_eval_in_frame(watch->expression, -1);
            printf(" = ");
            lisp_print(value);
        }
        printf("\n");
    }
}

/*
 * Update all watch expressions and check for changes
 * Returns 1 if a data breakpoint was triggered
 */
void debug_update_watches(void) {
    if (!g_debug_state || !g_debug_state->current_env) return;

    for (int i = 0; i < g_debug_state->num_watches; i++) {
        WatchExpression *watch = &g_debug_state->watches[i];
        if (!watch->enabled) continue;

        /* Evaluate current value */
        LispObject *new_value = debug_eval_in_frame(watch->expression, -1);

        /* Check for data breakpoint trigger */
        if (watch->break_on_change && watch->last_value) {
            /* Compare values - simple equality check */
            int changed = 0;

            if (new_value->type != watch->last_value->type) {
                changed = 1;
            } else if (is_number(new_value)) {
                changed = (new_value->number != watch->last_value->number);
            } else if (is_string(new_value)) {
                changed = (strcmp(new_value->string.data, watch->last_value->string.data) != 0);
            } else if (is_symbol(new_value)) {
                changed = (strcmp(new_value->symbol.name, watch->last_value->symbol.name) != 0);
            } else if (is_boolean(new_value)) {
                changed = (is_true(new_value) != is_true(watch->last_value));
            } else {
                /* For complex types, just check pointer equality */
                changed = (new_value != watch->last_value);
            }

            if (changed) {
                g_debug_state->stop_reason = STOP_REASON_DATA_CHANGE;
                g_debug_state->mode = DEBUG_MODE_PAUSED;
                g_debug_state->is_running = 0;

                if (!g_debug_state->json_mode) {
                    printf("\nData breakpoint hit: '%s' changed\n", watch->expression);
                    printf("  Old value: ");
                    lisp_print(watch->last_value);
                    printf("\n  New value: ");
                    lisp_print(new_value);
                    printf("\n");
                }
            }
        }

        watch->last_value = new_value;
    }
}

/*
 * Clear all watch expressions
 */
void debug_clear_all_watches(void) {
    if (!g_debug_state) return;
    g_debug_state->num_watches = 0;
    printf("All watch expressions cleared.\n");
}

/* ============================================
 * Exception Breakpoint Management
 * ============================================ */

/*
 * Set exception break mode
 */
void debug_set_exception_break(ExceptionBreakMode mode) {
    if (g_debug_state) {
        g_debug_state->exception_break_mode = mode;
        if (!g_debug_state->json_mode) {
            switch (mode) {
                case EXCEPTION_BREAK_NONE:
                    printf("Exception breakpoints disabled.\n");
                    break;
                case EXCEPTION_BREAK_UNCAUGHT:
                    printf("Breaking on uncaught exceptions.\n");
                    break;
                case EXCEPTION_BREAK_ALL:
                    printf("Breaking on all exceptions.\n");
                    break;
            }
        }
    }
}

/*
 * Get current exception break mode
 */
ExceptionBreakMode debug_get_exception_break(void) {
    return g_debug_state ? g_debug_state->exception_break_mode : EXCEPTION_BREAK_NONE;
}

/*
 * Called when an exception/error occurs
 * This should be called from the error handling code
 */
void debug_on_exception(const char *message, LispObject *expr) {
    if (!g_debug_state) return;
    if (g_debug_state->exception_break_mode == EXCEPTION_BREAK_NONE) return;

    /* For now, treat all exceptions as breaking (EXCEPTION_BREAK_ALL behavior) */
    /* EXCEPTION_BREAK_UNCAUGHT would require tracking try/catch contexts */

    g_debug_state->last_exception_message = message;
    g_debug_state->stop_reason = STOP_REASON_EXCEPTION;
    g_debug_state->mode = DEBUG_MODE_PAUSED;
    g_debug_state->is_running = 0;

    if (expr) {
        g_debug_state->current_expr = expr;
    }

    if (!g_debug_state->json_mode) {
        printf("\nException breakpoint hit!\n");
        printf("Error: %s\n", message ? message : "(unknown error)");
        if (expr) {
            printf("At: ");
            lisp_print(expr);
            printf("\n");
        }
        debug_repl();
    } else {
        /* Send JSON event */
        char body[512];
        snprintf(body, sizeof(body),
                 "{\"reason\":\"exception\",\"message\":\"%s\"}",
                 message ? message : "");
        debug_send_json_event("stopped", body);
    }
}

/* ============================================
 * Execution Control
 * ============================================ */

/*
 * Continue execution
 */
void debug_run(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_CONTINUE;
        g_debug_state->is_running = 1;
    }
}

/*
 * Step into next expression
 */
void debug_step_into(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_STEP_INTO;
        g_debug_state->is_running = 1;
    }
}

/*
 * Step over (stay at same call level)
 */
void debug_step_over(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_STEP_OVER;
        g_debug_state->step_depth = g_debug_state->stack_depth;
        g_debug_state->is_running = 1;
    }
}

/*
 * Step out (return from current function)
 */
void debug_step_out(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_STEP_OUT;
        g_debug_state->step_depth = g_debug_state->stack_depth;
        g_debug_state->is_running = 1;
    }
}

/*
 * Pause execution
 */
void debug_pause(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_PAUSED;
        g_debug_state->is_running = 0;
    }
}

/*
 * Stop execution
 */
void debug_stop(void) {
    if (g_debug_state) {
        g_debug_state->mode = DEBUG_MODE_NONE;
        g_debug_state->is_running = 0;
        debug_repl_running = 0;
    }
}

/* ============================================
 * Breakpoint Checking (called by evaluator)
 * ============================================ */

/*
 * Evaluate a conditional breakpoint expression
 * Returns 1 if condition is true (should break), 0 otherwise
 */
static int debug_eval_condition(const char *condition, Environment *env) {
    if (!condition || !condition[0]) return 1;  /* No condition = always break */
    if (!env) return 1;

    /* Parse the condition */
    Lexer lexer;
    lexer_init(&lexer, condition);

    Parser parser;
    parser_init(&parser, &lexer);

    LispObject *cond_expr = parse_expression(&parser);
    if (parser_had_error(&parser)) {
        /* Parse error - report but break anyway */
        if (!g_debug_state->json_mode) {
            printf("Warning: breakpoint condition parse error: %s\n", condition);
        }
        return 1;
    }

    /* Temporarily disable debug mode to avoid recursion */
    DebugMode saved_mode = g_debug_state->mode;
    g_debug_state->mode = DEBUG_MODE_NONE;

    LispObject *result = eval(cond_expr, env);

    g_debug_state->mode = saved_mode;

    /* Check if result is false (#f or nil) */
    return !is_false(result) && !is_nil(result);
}

/*
 * Check if a breakpoint exists at the given location
 */
static int has_breakpoint_at(const char *file, int line) {
    if (!g_debug_state || !file) return 0;

    for (int i = 0; i < g_debug_state->num_breakpoints; i++) {
        Breakpoint *bp = &g_debug_state->breakpoints[i];
        if (bp->enabled && bp->line == line) {
            /* Check if filenames match (just compare basenames) */
            const char *bp_base = strrchr(bp->filename, '/');
            const char *file_base = strrchr(file, '/');
            bp_base = bp_base ? bp_base + 1 : bp->filename;
            file_base = file_base ? file_base + 1 : file;

            if (strcmp(bp_base, file_base) == 0) {
                /* Check condition if present */
                if (bp->condition[0]) {
                    if (!debug_eval_condition(bp->condition, g_debug_state->current_env)) {
                        continue;  /* Condition not met, skip this breakpoint */
                    }
                }
                bp->hit_count++;
                return bp->id;
            }
        }
    }
    return 0;
}

/*
 * Check if should break at current location
 * Called by the evaluator before evaluating each expression
 * Returns 1 if execution should pause, 0 otherwise
 */
int debug_check_break(LispObject *expr, Environment *env) {
    if (!g_debug_state || g_debug_state->mode == DEBUG_MODE_NONE) {
        return 0;
    }

    g_debug_state->current_expr = expr;
    g_debug_state->current_env = env;

    /* Update watch expressions and check for data breakpoints */
    debug_update_watches();

    /* Check if data breakpoint was triggered */
    if (g_debug_state->mode == DEBUG_MODE_PAUSED &&
        g_debug_state->stop_reason == STOP_REASON_DATA_CHANGE) {
        /* debug_update_watches already set up the stop state */
        if (!g_debug_state->json_mode) {
            debug_repl();
        }
        return 1;
    }

    int should_break = 0;
    StopReason reason = STOP_REASON_NONE;

    switch (g_debug_state->mode) {
        case DEBUG_MODE_STEP_INTO:
            should_break = 1;
            reason = STOP_REASON_STEP;
            break;

        case DEBUG_MODE_STEP_OVER:
            if (g_debug_state->stack_depth <= g_debug_state->step_depth) {
                should_break = 1;
                reason = STOP_REASON_STEP_OVER;
            }
            break;

        case DEBUG_MODE_STEP_OUT:
            if (g_debug_state->stack_depth < g_debug_state->step_depth) {
                should_break = 1;
                reason = STOP_REASON_STEP_OUT;
            }
            break;

        case DEBUG_MODE_CONTINUE: {
            int bp_id = has_breakpoint_at(
                g_debug_state->current_location.filename,
                g_debug_state->current_location.line);
            if (bp_id > 0) {
                should_break = 1;
                reason = STOP_REASON_BREAKPOINT;
            }
            break;
        }

        case DEBUG_MODE_PAUSED:
            should_break = 1;
            reason = STOP_REASON_ENTRY;
            break;

        default:
            break;
    }

    if (should_break) {
        g_debug_state->stop_reason = reason;
        g_debug_state->is_running = 0;
        g_debug_state->mode = DEBUG_MODE_PAUSED;

        /* Call the callback if registered */
        if (g_debug_state->on_stop) {
            g_debug_state->on_stop(g_debug_state->callback_context,
                                   reason, expr, env);
        }

        /* If not in JSON mode, enter debug REPL */
        if (!g_debug_state->json_mode) {
            debug_repl();
        }
    }

    return should_break;
}

/* ============================================
 * Call Stack Management
 * ============================================ */

/*
 * Push a new call frame onto the stack
 */
void debug_push_frame(const char *func_name, LispObject *args,
                      Environment *env, SourceLocation *loc) {
    if (!g_debug_state) return;
    if (g_debug_state->stack_depth >= MAX_CALL_STACK) {
        fprintf(stderr, "Debug: Call stack overflow\n");
        return;
    }

    CallFrame *frame = &g_debug_state->call_stack[g_debug_state->stack_depth];
    frame->function_name = func_name ? strdup(func_name) : "<lambda>";
    frame->arguments = args;
    frame->env = env;
    frame->frame_id = g_debug_state->stack_depth;

    if (loc) {
        frame->location = *loc;
    } else {
        frame->location = g_debug_state->current_location;
    }

    g_debug_state->stack_depth++;
}

/*
 * Pop a call frame from the stack
 */
void debug_pop_frame(void) {
    if (!g_debug_state || g_debug_state->stack_depth == 0) return;

    g_debug_state->stack_depth--;
    CallFrame *frame = &g_debug_state->call_stack[g_debug_state->stack_depth];

    if (frame->function_name && strcmp(frame->function_name, "<lambda>") != 0) {
        free((void *)frame->function_name);
    }
}

/*
 * Get current stack depth
 */
int debug_get_stack_depth(void) {
    return g_debug_state ? g_debug_state->stack_depth : 0;
}

/*
 * Get a specific stack frame
 */
CallFrame *debug_get_frame(int index) {
    if (!g_debug_state || index < 0 || index >= g_debug_state->stack_depth) {
        return NULL;
    }
    return &g_debug_state->call_stack[index];
}

/*
 * Print backtrace
 */
void debug_print_backtrace(void) {
    if (!g_debug_state) {
        printf("Debugger not initialized.\n");
        return;
    }

    if (g_debug_state->stack_depth == 0) {
        printf("No call stack (at top level).\n");
        return;
    }

    printf("Call stack:\n");
    for (int i = g_debug_state->stack_depth - 1; i >= 0; i--) {
        CallFrame *frame = &g_debug_state->call_stack[i];
        printf("  #%d  %s", i, frame->function_name);

        if (frame->location.filename) {
            printf(" at %s:%d",
                   frame->location.filename,
                   frame->location.line);
        }

        printf("\n");

        /* Show arguments */
        if (frame->arguments && !is_nil(frame->arguments)) {
            printf("       args: ");
            lisp_print(frame->arguments);
            printf("\n");
        }
    }
}

/* ============================================
 * Source Location
 * ============================================ */

/*
 * Set current source location (called by parser/evaluator)
 */
void debug_set_current_location(const char *file, int line, int column) {
    if (!g_debug_state) return;

    g_debug_state->current_location.filename = file;
    g_debug_state->current_location.line = line;
    g_debug_state->current_location.column = column;
    g_debug_state->current_file = file;
}

/*
 * Get current source location
 */
SourceLocation debug_get_current_location(void) {
    if (g_debug_state) {
        return g_debug_state->current_location;
    }
    SourceLocation empty = {NULL, 0, 0};
    return empty;
}

/* ============================================
 * Variable Inspection
 * ============================================ */

/*
 * Get local variables in a frame as an alist
 */
LispObject *debug_get_locals(int frame_index) {
    CallFrame *frame = debug_get_frame(frame_index);
    if (!frame || !frame->env) {
        return make_nil();
    }
    return env_get_all_bindings(frame->env);
}

/*
 * Evaluate an expression in a specific frame's environment
 */
LispObject *debug_eval_in_frame(const char *expr_str, int frame_index) {
    CallFrame *frame = debug_get_frame(frame_index);
    Environment *env = frame ? frame->env : g_debug_state->current_env;

    if (!env) {
        printf("No environment available.\n");
        return make_nil();
    }

    /* Parse the expression */
    Lexer lexer;
    lexer_init(&lexer, expr_str);

    Parser parser;
    parser_init(&parser, &lexer);

    LispObject *expr = parse_expression(&parser);
    if (parser_had_error(&parser)) {
        printf("Parse error: %s\n", parser_error_message(&parser));
        return make_nil();
    }

    /* Temporarily disable debug mode to avoid recursion */
    DebugMode saved_mode = g_debug_state->mode;
    g_debug_state->mode = DEBUG_MODE_NONE;

    LispObject *result = eval(expr, env);

    g_debug_state->mode = saved_mode;

    return result;
}

/*
 * Look up a variable by name in a frame
 */
LispObject *debug_lookup_variable(const char *name, int frame_index) {
    CallFrame *frame = debug_get_frame(frame_index);
    Environment *env = frame ? frame->env : g_debug_state->current_env;

    if (!env) return NULL;

    LispObject *sym = make_symbol(name);
    return env_lookup(env, sym);
}

/* ============================================
 * Callback Registration
 * ============================================ */

/*
 * Set debug callback
 */
void debug_set_callback(DebugCallback callback, void *context) {
    if (g_debug_state) {
        g_debug_state->on_stop = callback;
        g_debug_state->callback_context = context;
    }
}

/* ============================================
 * Debug REPL
 * ============================================ */

/*
 * Print help message
 */
static void debug_print_help(void) {
    printf("\nDebugger Commands:\n");
    printf("  run, r           Continue execution\n");
    printf("  step, s          Step into next expression\n");
    printf("  next, n          Step over (same level)\n");
    printf("  finish, f        Step out (return from function)\n");
    printf("  stop             Stop debugging\n");
    printf("\n");
    printf("  break <line>     Set breakpoint at line\n");
    printf("  break <file>:<line>  Set breakpoint at file:line\n");
    printf("  cond <id> <expr> Set breakpoint condition\n");
    printf("  delete <id>      Delete breakpoint\n");
    printf("  enable <id>      Enable breakpoint\n");
    printf("  disable <id>     Disable breakpoint\n");
    printf("  info break       List all breakpoints\n");
    printf("  clear            Clear all breakpoints\n");
    printf("\n");
    printf("  watch <expr>     Add watch expression\n");
    printf("  watch <expr> break  Add data breakpoint (break on change)\n");
    printf("  unwatch <id>     Remove watch expression\n");
    printf("  info watch       List all watch expressions\n");
    printf("\n");
    printf("  catch all        Break on all exceptions\n");
    printf("  catch uncaught   Break on uncaught exceptions\n");
    printf("  catch none       Disable exception breakpoints\n");
    printf("\n");
    printf("  backtrace, bt    Print call stack\n");
    printf("  frame <n>        Select stack frame\n");
    printf("  up               Move up one frame\n");
    printf("  down             Move down one frame\n");
    printf("\n");
    printf("  print <expr>     Evaluate and print expression\n");
    printf("  locals           Show local variables\n");
    printf("  where            Show current location\n");
    printf("\n");
    printf("  help, h          Show this help\n");
    printf("  quit, q          Quit debugger\n");
    printf("\n");
}

/*
 * Show current location with context
 */
static void debug_show_location(void) {
    if (!g_debug_state) return;

    SourceLocation *loc = &g_debug_state->current_location;

    if (loc->filename) {
        printf("\nStopped at %s:%d\n", loc->filename, loc->line);
    } else {
        printf("\nStopped (location unknown)\n");
    }

    /* Show current expression */
    if (g_debug_state->current_expr) {
        printf("=> ");
        lisp_print(g_debug_state->current_expr);
        printf("\n");
    }
}

/*
 * Interactive debug REPL
 */
void debug_repl(void) {
    char line[1024];
    int current_frame = -1;  /* -1 means current (top) */

    if (!g_debug_state) {
        printf("Debugger not initialized.\n");
        return;
    }

    debug_repl_running = 1;

    /* Show initial location */
    const char *reason_str = "";
    switch (g_debug_state->stop_reason) {
        case STOP_REASON_BREAKPOINT: reason_str = " (breakpoint)"; break;
        case STOP_REASON_STEP: reason_str = " (step)"; break;
        case STOP_REASON_STEP_OVER: reason_str = " (step over)"; break;
        case STOP_REASON_STEP_OUT: reason_str = " (step out)"; break;
        case STOP_REASON_ENTRY: reason_str = " (entry)"; break;
        case STOP_REASON_EXCEPTION: reason_str = " (exception)"; break;
        case STOP_REASON_DATA_CHANGE: reason_str = " (data breakpoint)"; break;
        default: break;
    }
    printf("\nProgram stopped%s\n", reason_str);
    debug_show_location();

    while (debug_repl_running && g_debug_state->mode == DEBUG_MODE_PAUSED) {
        printf("(debug) ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        /* Skip empty lines */
        if (len == 0) continue;

        /* Parse command */
        char cmd[64] = {0};
        char arg[256] = {0};
        sscanf(line, "%63s %255[^\n]", cmd, arg);

        /* Help */
        if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
            debug_print_help();
        }
        /* Quit */
        else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            debug_stop();
            break;
        }
        /* Run/Continue */
        else if (strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0 ||
                 strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0) {
            debug_run();
            break;
        }
        /* Step into */
        else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
            debug_step_into();
            break;
        }
        /* Step over */
        else if (strcmp(cmd, "next") == 0 || strcmp(cmd, "n") == 0) {
            debug_step_over();
            break;
        }
        /* Step out */
        else if (strcmp(cmd, "finish") == 0 || strcmp(cmd, "f") == 0) {
            debug_step_out();
            break;
        }
        /* Stop */
        else if (strcmp(cmd, "stop") == 0) {
            debug_stop();
            printf("Debugging stopped.\n");
            break;
        }
        /* Breakpoint */
        else if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: break <line> or break <file>:<line>\n");
            } else {
                char *colon = strchr(arg, ':');
                if (colon) {
                    *colon = '\0';
                    int line_num = atoi(colon + 1);
                    debug_add_breakpoint(arg, line_num);
                } else {
                    int line_num = atoi(arg);
                    const char *file = g_debug_state->current_file ?
                                       g_debug_state->current_file : "unknown";
                    debug_add_breakpoint(file, line_num);
                }
            }
        }
        /* Delete breakpoint */
        else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "d") == 0) {
            int id = atoi(arg);
            if (!debug_remove_breakpoint(id)) {
                printf("Breakpoint %d not found.\n", id);
            }
        }
        /* Enable breakpoint */
        else if (strcmp(cmd, "enable") == 0) {
            int id = atoi(arg);
            if (debug_enable_breakpoint(id)) {
                printf("Breakpoint %d enabled.\n", id);
            } else {
                printf("Breakpoint %d not found.\n", id);
            }
        }
        /* Disable breakpoint */
        else if (strcmp(cmd, "disable") == 0) {
            int id = atoi(arg);
            if (debug_disable_breakpoint(id)) {
                printf("Breakpoint %d disabled.\n", id);
            } else {
                printf("Breakpoint %d not found.\n", id);
            }
        }
        /* Conditional breakpoint */
        else if (strcmp(cmd, "cond") == 0) {
            int id = 0;
            char cond_expr[256] = {0};
            if (sscanf(arg, "%d %255[^\n]", &id, cond_expr) >= 1) {
                Breakpoint *bp = debug_get_breakpoint(id);
                if (bp) {
                    strncpy(bp->condition, cond_expr, MAX_CONDITION_LENGTH - 1);
                    bp->condition[MAX_CONDITION_LENGTH - 1] = '\0';
                    if (cond_expr[0]) {
                        printf("Breakpoint %d condition set: %s\n", id, cond_expr);
                    } else {
                        printf("Breakpoint %d condition cleared.\n", id);
                    }
                } else {
                    printf("Breakpoint %d not found.\n", id);
                }
            } else {
                printf("Usage: cond <breakpoint-id> [condition]\n");
            }
        }
        /* Info breakpoints/watches */
        else if (strcmp(cmd, "info") == 0) {
            if (strcmp(arg, "break") == 0 || strcmp(arg, "breakpoints") == 0) {
                debug_list_breakpoints();
            } else if (strcmp(arg, "watch") == 0 || strcmp(arg, "watches") == 0) {
                debug_list_watches();
            } else {
                printf("Usage: info break | info watch\n");
            }
        }
        /* Clear breakpoints */
        else if (strcmp(cmd, "clear") == 0) {
            debug_clear_all_breakpoints();
        }
        /* Watch expression */
        else if (strcmp(cmd, "watch") == 0 || strcmp(cmd, "w") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: watch <expression> [break]\n");
            } else {
                /* Check if "break" suffix for data breakpoint */
                char *break_suffix = strstr(arg, " break");
                if (break_suffix) {
                    *break_suffix = '\0';  /* Remove "break" from expression */
                    debug_add_data_breakpoint(arg);
                } else {
                    debug_add_watch(arg);
                }
            }
        }
        /* Remove watch */
        else if (strcmp(cmd, "unwatch") == 0) {
            int id = atoi(arg);
            if (!debug_remove_watch(id)) {
                printf("Watch %d not found.\n", id);
            }
        }
        /* Exception breakpoints */
        else if (strcmp(cmd, "catch") == 0) {
            if (strcmp(arg, "all") == 0) {
                debug_set_exception_break(EXCEPTION_BREAK_ALL);
            } else if (strcmp(arg, "uncaught") == 0) {
                debug_set_exception_break(EXCEPTION_BREAK_UNCAUGHT);
            } else if (strcmp(arg, "none") == 0) {
                debug_set_exception_break(EXCEPTION_BREAK_NONE);
            } else {
                printf("Usage: catch all | catch uncaught | catch none\n");
            }
        }
        /* Backtrace */
        else if (strcmp(cmd, "backtrace") == 0 || strcmp(cmd, "bt") == 0) {
            debug_print_backtrace();
        }
        /* Frame selection */
        else if (strcmp(cmd, "frame") == 0) {
            current_frame = atoi(arg);
            printf("Switched to frame %d.\n", current_frame);
        }
        else if (strcmp(cmd, "up") == 0) {
            if (current_frame < g_debug_state->stack_depth - 1) {
                current_frame++;
                printf("Frame %d.\n", current_frame);
            } else {
                printf("Already at top frame.\n");
            }
        }
        else if (strcmp(cmd, "down") == 0) {
            if (current_frame > 0) {
                current_frame--;
                printf("Frame %d.\n", current_frame);
            } else {
                printf("Already at bottom frame.\n");
            }
        }
        /* Print expression */
        else if (strcmp(cmd, "print") == 0 || strcmp(cmd, "p") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: print <expression>\n");
            } else {
                LispObject *result = debug_eval_in_frame(arg, current_frame);
                lisp_print(result);
                printf("\n");
            }
        }
        /* Locals */
        else if (strcmp(cmd, "locals") == 0) {
            LispObject *locals = debug_get_locals(current_frame);
            if (is_nil(locals)) {
                printf("No local variables.\n");
            } else {
                printf("Local variables:\n");
                while (is_cons(locals)) {
                    LispObject *binding = car(locals);
                    if (is_cons(binding)) {
                        printf("  ");
                        lisp_print(car(binding));
                        printf(" = ");
                        lisp_print(cdr(binding));
                        printf("\n");
                    }
                    locals = cdr(locals);
                }
            }
        }
        /* Where */
        else if (strcmp(cmd, "where") == 0) {
            debug_show_location();
        }
        /* Unknown command */
        else {
            printf("Unknown command: %s (type 'help' for commands)\n", cmd);
        }
    }
}

/* ============================================
 * JSON Protocol (for IDE integration)
 * ============================================ */

/*
 * Handle a JSON command from the IDE
 */
void debug_handle_json_command(const char *json_cmd) {
    /* Simple JSON parsing - in production use a proper JSON library */
    /* For now, this is a stub that shows the expected format */

    if (strstr(json_cmd, "\"command\":\"setBreakpoint\"")) {
        /* Extract file and line from JSON */
        /* This would need proper JSON parsing */
        printf("{\"type\":\"response\",\"success\":true}\n");
        fflush(stdout);
    }
    else if (strstr(json_cmd, "\"command\":\"continue\"")) {
        debug_run();
        printf("{\"type\":\"response\",\"success\":true}\n");
        fflush(stdout);
    }
    else if (strstr(json_cmd, "\"command\":\"stepIn\"")) {
        debug_step_into();
        printf("{\"type\":\"response\",\"success\":true}\n");
        fflush(stdout);
    }
    else if (strstr(json_cmd, "\"command\":\"stepOver\"")) {
        debug_step_over();
        printf("{\"type\":\"response\",\"success\":true}\n");
        fflush(stdout);
    }
    else if (strstr(json_cmd, "\"command\":\"stepOut\"")) {
        debug_step_out();
        printf("{\"type\":\"response\",\"success\":true}\n");
        fflush(stdout);
    }
    else if (strstr(json_cmd, "\"command\":\"stackTrace\"")) {
        printf("{\"type\":\"response\",\"command\":\"stackTrace\",\"body\":{\"stackFrames\":[");
        for (int i = g_debug_state->stack_depth - 1; i >= 0; i--) {
            CallFrame *frame = &g_debug_state->call_stack[i];
            if (i < g_debug_state->stack_depth - 1) printf(",");
            printf("{\"id\":%d,\"name\":\"%s\",\"line\":%d}",
                   i, frame->function_name, frame->location.line);
        }
        printf("]}}\n");
        fflush(stdout);
    }
    else {
        printf("{\"type\":\"response\",\"success\":false,\"message\":\"Unknown command\"}\n");
        fflush(stdout);
    }
}

/*
 * Send a JSON event to the IDE
 */
void debug_send_json_event(const char *event_type, const char *body) {
    printf("{\"type\":\"event\",\"event\":\"%s\",\"body\":%s}\n",
           event_type, body ? body : "{}");
    fflush(stdout);
}

/* ============================================
 * Utility Functions
 * ============================================ */

/*
 * Print source location
 */
void debug_print_location(SourceLocation *loc) {
    if (loc && loc->filename) {
        printf("%s:%d:%d", loc->filename, loc->line, loc->column);
    } else {
        printf("<unknown>");
    }
}

/*
 * Print expression (truncated if too long)
 */
void debug_print_expression(LispObject *expr) {
    char buffer[256];
    lisp_print_to_buffer(expr, buffer, sizeof(buffer));
    printf("%s", buffer);
}
